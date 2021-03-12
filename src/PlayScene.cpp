#include "PlayScene.h"
#include "Game.h"
#include "EventManager.h"

// required for IMGUI
#include "imgui.h"
#include "imgui_sdl.h"
#include "Renderer.h"
#include "Util.h"

PlayScene::PlayScene()
{
	PlayScene::start();
}

PlayScene::~PlayScene()
= default;

void PlayScene::draw()
{
	drawDisplayList();
	
	if(EventManager::Instance().isIMGUIActive())
	{
		GUI_Function();	
	}

	SDL_SetRenderDrawColor(Renderer::Instance()->getRenderer(), 255, 255, 255, 255);
}

void PlayScene::update()
{
	updateDisplayList();

	if(m_shipIsMoving)
	{
		m_moveShip();
	}
}

void PlayScene::clean()
{
	removeAllChildren();
}

void PlayScene::handleEvents()
{
	EventManager::Instance().update();

	if (EventManager::Instance().isKeyDown(SDL_SCANCODE_ESCAPE))
	{
		TheGame::Instance()->quit();
	}

	if (EventManager::Instance().isKeyDown(SDL_SCANCODE_1))
	{
		TheGame::Instance()->changeSceneState(START_SCENE);
	}

	if (EventManager::Instance().isKeyDown(SDL_SCANCODE_2))
	{
		TheGame::Instance()->changeSceneState(END_SCENE);
	}

	if(EventManager::Instance().isKeyDown(SDL_SCANCODE_F))
	{
		m_findShortestPath();

	}

	
	if(EventManager::Instance().isKeyDown(SDL_SCANCODE_M))
	{
		m_shipIsMoving = true;
	}

	if (EventManager::Instance().isKeyDown(SDL_SCANCODE_G))
	{
		m_setGridEnabled(!m_getGridEnabled());
	}
	
}

void PlayScene::start()
{
	// Set GUI Title
	m_guiTitle = "Play Scene";

	m_buildGrid();
	auto offset = glm::vec2(Config::TILE_SIZE * 0.5f, Config::TILE_SIZE * 0.5f);
	currentHeuristic = EUCLIDEAN;
	
	// add the ship to the scene as a start point
	m_pShip = new Ship();
	m_pShip->getTransform()->position = m_getTile(1, 3)->getTransform()->position + offset;
	m_pShip->setGridPosition(1, 3);
	m_getTile(1, 3)->setTileStatus(START);
	addChild(m_pShip);
	
	// added the target to the scene a goal
	m_pTarget = new Target();
	m_pTarget->getTransform()->position = m_getTile(15, 11)->getTransform()->position + offset;
	m_pTarget->setGridPosition(15, 11);
	m_getTile(15, 11)->setTileStatus(GOAL);
	addChild(m_pTarget);

	m_computeTileCosts();
	
}

void PlayScene::GUI_Function() 
{
	//TODO: We need to deal with this
	auto offset = glm::vec2(Config::TILE_SIZE * 0.5f, Config::TILE_SIZE * 0.5f);
	
	// Always open with a NewFrame
	ImGui::NewFrame();

	// See examples by uncommenting the following - also look at imgui_demo.cpp in the IMGUI filter
	//ImGui::ShowDemoWindow();
	
	ImGui::Begin("GAME3001 - Lab 5", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar);

	static bool isGridEnabled = false;
	if(ImGui::Checkbox("Grid Enabled", &isGridEnabled))
	{
		// toggle grid on/off
		m_setGridEnabled(isGridEnabled);
	}

	ImGui::Separator();

	auto radio = static_cast<int>(currentHeuristic);
	ImGui::LabelText("", "Heuristic Type");
	ImGui::RadioButton("Manhattan", &radio, static_cast<int>(MANHATTAN));
	ImGui::SameLine();
	ImGui::RadioButton("Euclidean", &radio, static_cast<int>(EUCLIDEAN));
	if(currentHeuristic != Heuristic(radio))
	{
		currentHeuristic = Heuristic(radio);
		m_computeTileCosts();
	}

	ImGui::Separator();

	static int startPosition[] = { m_pShip->getGridPosition().x, m_pShip->getGridPosition().y };
	if (ImGui::SliderInt2("Start Position", startPosition, 0, Config::COL_NUM - 1))
	{
		// Row adjustment
		if (startPosition[1] > Config::ROW_NUM - 1)
		{
			startPosition[1] = Config::ROW_NUM - 1;
		}

		SDL_RenderClear(Renderer::Instance()->getRenderer());
		m_getTile(m_pShip->getGridPosition())->setTileStatus(UNVISITED);
		m_pShip->getTransform()->position = m_getTile(startPosition[0], startPosition[1])->getTransform()->position + offset;
		m_pShip->setGridPosition(startPosition[0], startPosition[1]);
		m_getTile(m_pShip->getGridPosition())->setTileStatus(START);
		SDL_SetRenderDrawColor(Renderer::Instance()->getRenderer(), 255, 255, 255, 255);
		SDL_RenderPresent(Renderer::Instance()->getRenderer());
	}
	
	static int targetPosition[] = { m_pTarget->getGridPosition().x, m_pTarget->getGridPosition().y };
	if(ImGui::SliderInt2("Target Position", targetPosition, 0, Config::COL_NUM - 1))
	{
		// Row adjustment
		if(targetPosition[1] > Config::ROW_NUM - 1)
		{
			targetPosition[1] = Config::ROW_NUM - 1;
		}
		
		SDL_RenderClear(Renderer::Instance()->getRenderer());
		m_getTile(m_pTarget->getGridPosition())->setTileStatus(UNVISITED);
		m_pTarget->getTransform()->position = m_getTile(targetPosition[0], targetPosition[1])->getTransform()->position + offset;
		m_pTarget->setGridPosition(targetPosition[0], targetPosition[1]);
		m_getTile(m_pTarget->getGridPosition())->setTileStatus(GOAL);
		m_computeTileCosts();
		SDL_SetRenderDrawColor(Renderer::Instance()->getRenderer(), 255, 255, 255, 255);
		SDL_RenderPresent(Renderer::Instance()->getRenderer());
	}
	
	ImGui::Separator();
	
	if(ImGui::Button("Find Shortest Path"))
	{
		m_findShortestPath();
	}

	if (ImGui::Button("Start"))
	{
		// make ship move

		
		m_shipIsMoving = true;
	}

	ImGui::SameLine();
	
	if (ImGui::Button("Reset"))
	{
		// reset everything back to initial values
		
	}

	ImGui::Separator();

	
	ImGui::End();

	// Don't Remove this
	ImGui::Render();
	ImGuiSDL::Render(ImGui::GetDrawData());
	ImGui::StyleColorsDark();
}

void PlayScene::m_buildGrid()
{
	auto tileSize = Config::TILE_SIZE;

	// add tiles to the grid
	for (int row = 0; row < Config::ROW_NUM; ++row)
	{
		for (int col = 0; col < Config::COL_NUM; ++col)
		{
			Tile* tile = new Tile(); // create empty tile
			tile->getTransform()->position = glm::vec2(col * tileSize, row * tileSize);
			tile->setGridPosition(col, row);
			addChild(tile);
			tile->addLabels();
			tile->setEnabled(false);
			m_pGrid.push_back(tile);
		}
	}

	// create references for each tile to its neighbours
	for (int row = 0; row < Config::ROW_NUM; ++row)
	{
		for (int col = 0; col < Config::COL_NUM; ++col)
		{
			Tile* tile = m_getTile(col, row);

			// Topmost row
			if(row == 0)
			{
				tile->setNeighbourTile(TOP_TILE, nullptr);
			}
			else
			{
				tile->setNeighbourTile(TOP_TILE, m_getTile(col, row - 1));
			}

			// rightmost column
			if(col == Config::COL_NUM - 1)
			{
				tile->setNeighbourTile(RIGHT_TILE, nullptr);
			}
			else
			{
				tile->setNeighbourTile(RIGHT_TILE, m_getTile(col + 1, row));
			}

			// bottommost row
			if(row == Config::ROW_NUM - 1)
			{
				tile->setNeighbourTile(BOTTOM_TILE, nullptr);
			}
			else
			{
				tile->setNeighbourTile(BOTTOM_TILE, m_getTile(col, row + 1));
			}

			// leftmost  column
			if(col == 0)
			{
				tile->setNeighbourTile(LEFT_TILE, nullptr);
			}
			else
			{
				tile->setNeighbourTile(LEFT_TILE, m_getTile(col - 1, row));
			}
		}
	}

	std::cout << m_pGrid.size() << std::endl;
}

void PlayScene::m_computeTileCosts()
{
	float distance, dx, dy;
	
	for (auto tile : m_pGrid)
	{
		switch(currentHeuristic)
		{
		case MANHATTAN:
			// Manhattan Distance
			dx = abs(tile->getGridPosition().x - m_pTarget->getGridPosition().x);
			dy = abs(tile->getGridPosition().y - m_pTarget->getGridPosition().y);
			distance = dx + dy;
			break;
		case EUCLIDEAN:
			// Euclidean Distance
			distance = Util::distance(m_pTarget->getGridPosition(), tile->getGridPosition());
			break;
		}
		
		tile->setTileCost(distance);
	}
}

void PlayScene::m_findShortestPath()
{
	if(m_pPathList.empty())
	{
		// Step 1 - Add Start position to the open list
		auto startTile = m_getTile(m_pShip->getGridPosition());
		startTile->setTileStatus(OPEN);
		m_pOpenList.push_back(startTile);

		bool goalFound = false;

		// Step 2 - Loop until the OpenList is empty or the Goal is found
		while (!m_pOpenList.empty() && !goalFound)
		{
			auto min = INFINITY;
			Tile* minTile;
			int minTileIndex = 0;
			int count = 0;

			std::vector<Tile*> neighbourList;
			for (int index = 0; index < NUM_OF_NEIGHBOUR_TILES; ++index)
			{
				neighbourList.push_back(m_pOpenList[0]->getNeighbourTile(NeighbourTile(index)));
			}

			for (auto neighbour : neighbourList)
			{
				if (neighbour->getTileStatus() != GOAL)
				{
					if (neighbour->getTileCost() < min)
					{
						min = neighbour->getTileCost();
						minTile = neighbour;
						minTileIndex = count;
					}
					count++;
				}
				else
				{
					minTile = neighbour;
					m_pPathList.push_back(minTile);
					goalFound = true;
					break;
				}
			}

			// remove the reference of the current tile in the open list
			m_pPathList.push_back(m_pOpenList[0]);
			m_pOpenList.pop_back(); // empties the open list

			// add the minTile to the openList
			m_pOpenList.push_back(minTile);
			minTile->setTileStatus(OPEN);
			neighbourList.erase(neighbourList.begin() + minTileIndex);

			// push all remaining neighbours onto the closed list
			for (auto neighbour : neighbourList)
			{
				if (neighbour->getTileStatus() == UNVISITED)
				{
					neighbour->setTileStatus(CLOSED);
					m_pClosedList.push_back(neighbour);
				}
			}
		}

		m_displayPathList();
	}
	
	
}

void PlayScene::m_displayPathList()
{
	for (auto node : m_pPathList)
	{
		std::cout << "(" << node->getGridPosition().x << ", " << node->getGridPosition().y << ")" << std::endl;
	}
	std::cout << "Path Length: " << m_pPathList.size() << std::endl;
}

void PlayScene::m_setGridEnabled(bool state) 
{
	for (auto tile : m_pGrid)
	{
		tile->setEnabled(state);
		tile->setLabelsEnabled(state);
	}

	if(state == false)
	{
		SDL_RenderClear(Renderer::Instance()->getRenderer());
	}

	m_isGridEnabled = state;
}

bool PlayScene::m_getGridEnabled() const
{
	return m_isGridEnabled;
}

Tile* PlayScene::m_getTile(const int col, const int row)
{
	return m_pGrid[(row * Config::COL_NUM) + col];
}

Tile* PlayScene::m_getTile(const glm::vec2 grid_position)
{
	const auto col = grid_position.x;
	const auto row = grid_position.y;
	return m_pGrid[(row * Config::COL_NUM) + col];
}

void PlayScene::m_moveShip()
{
	auto offset = glm::vec2(Config::TILE_SIZE * 0.5f, Config::TILE_SIZE * 0.5f);
	if (moveCounter < m_pPathList.size())
	{

		m_pShip->getTransform()->position = m_getTile(m_pPathList[moveCounter]->getGridPosition())->getTransform()->position + offset;
		m_pShip->setGridPosition(m_pPathList[moveCounter]->getGridPosition().x, m_pPathList[moveCounter]->getGridPosition().y);
		if (Game::Instance()->getFrames() % 20 == 0)
		{
			moveCounter++;
		}
	}
	else
	{
		m_shipIsMoving = false;
	}
}
