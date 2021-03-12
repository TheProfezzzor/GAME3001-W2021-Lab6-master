#include "NavigationObject.h"

NavigationObject::NavigationObject()
= default;

NavigationObject::~NavigationObject()
= default;

glm::vec2 NavigationObject::getGridPosition() const
{
	return m_gridPosition;
}

void NavigationObject::setGridPosition(const float col, const float row)
{
	m_gridPosition = glm::vec2(col, row);
}
