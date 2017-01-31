#pragma once
#include "PhysicsObject.h"

class Plane : public PhysicsObject
{
public:
	Plane() { oType = PLANE; }
	Plane(glm::vec2 o, glm::vec2 n) : origin(o), normal(n) { oType = PLANE; oneSided = true; }

	virtual void Update(float dt);
	virtual void Draw();

	virtual void CollideWithPlane(Plane* plane) {} // plane-plane collisions do nothing
	virtual void CollideWithCircle(Circle* circle);
	virtual void CollideWithBox(Box* circle);

	// equation of the plane is (origin-x) cross (normal) = 0;
	// or (x-origin.x)*normal.y + (y-origin.y)*normal.x = 0

	glm::vec2 normal;
	glm::vec2 origin;

	bool oneSided;
};