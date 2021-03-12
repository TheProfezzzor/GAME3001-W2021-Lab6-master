#pragma once
#ifndef __TILE__
#define __TILE__

#include "Label.h"
#include "NavigationObject.h"
#include "NeighbourTile.h"
#include "TileStatus.h"

class Tile : public NavigationObject
{
public:
	// Constructor
	Tile();
	
	// Destructor
	~Tile();
	
	// Life-Cycle Functions
	void draw() override;
	void update() override;
	void clean() override;

	Tile* getNeighbourTile(NeighbourTile position);
	void setNeighbourTile(NeighbourTile position, Tile* tile);

	float getTileCost() const;
	void setTileCost(float cost);

	TileStatus getTileStatus() const;
	void setTileStatus(TileStatus status);

	void addLabels();
	void setLabelsEnabled(bool state);
	
private:
	float m_cost;
	TileStatus m_status;

	Label* m_costLabel;
	Label* m_statusLabel;
	
	Tile* m_neighbours[NUM_OF_NEIGHBOUR_TILES];
};

#endif /* defined (__TILE__) */