#include <gl_core_4_4.h>
#include <glm\glm\glm.hpp>
#include <aie/Gizmos.h>
#include "Plane.h"
#include "Circle.h"
#include "Box.h"

void Plane::Update(float dt)
{
}

void Plane::Draw()
{
	// plane has equation x*normal.x + y*normal.y = origin
	// find the intersections with circle of 100 units: x^2+y^2 = 100^2
	glm::vec2 start(origin.x - 100 * normal.y, origin.y + 100 * normal.x);
	glm::vec2 end(origin.x + 100 * normal.y, origin.y - 100 * normal.x);
	Gizmos::add2DLine(start, end, color);
}

void Plane::CollideWithCircle(Circle* circle)
{
	circle->CollideWithPlane(this);
}

void Plane::CollideWithBox(Box* box)
{
	box->CollideWithPlane(this);
}