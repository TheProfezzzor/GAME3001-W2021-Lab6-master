#pragma once
#ifndef __SHIP__
#define __SHIP__

#include "DisplayObject.h"
#include "TextureManager.h"
#include <glm/vec4.hpp> // For line color.

class Ship final : public DisplayObject
{
public:
	Ship();
	~Ship();

	// Inherited via GameObject
	void draw() override;
	void update() override;
	void clean() override;

	void turnRight();
	void turnLeft();
	void moveForward();
	void moveBack();
	
	void move();

	// getters
	glm::vec2 getTargetPosition() const;
	glm::vec2 getCurrentDirection() const;
	float getMaxSpeed() const;

	float getLOSDistance() const;
	bool hasLOS() const;
	float getCurrentHeading() const;

	// setters
	void setTargetPosition(glm::vec2 newPosition);
	void setCurrentDirection(glm::vec2 newDirection);
	void setMaxSpeed(float newSpeed);

	void setLOSDistance(float distance);
	void setHasLOS(bool state);
	void setCurrentHeading(float heading);

private:
	void m_checkBounds();
	void m_reset();

	// steering behaviours
	float m_maxSpeed;
	float m_turnRate;

	void m_changeDirection();
	float m_currentHeading;
	glm::vec2 m_currentDirection;
	glm::vec2 m_targetPosition;

	//LOS (line-of-sight)
	float m_LOSDistance;
	bool m_hasLOS;
	glm::vec4 m_LOSColour;
};

#endif /* defined (__SHIP__) */