#pragma once

class Plane;
class Circle;
class Box;

class PhysicsObject
{
public:
	enum PhysicsObjectType
	{
		PLANE,
		CIRCLE,
		BOX,
		SPRING,
	};

	PhysicsObject() : color(1, 0, 0, 1) {}

	virtual void Update(float dt) = 0;
	virtual void Draw() = 0;

	virtual void CheckCollisions(PhysicsObject * other);

	virtual void CollideWithPlane(Plane* plane) = 0;
	virtual void CollideWithCircle(Circle* circle) = 0;
	virtual void CollideWithBox(Box* circle) = 0;

	virtual float getEnergy(float& k, float& g, float &r) { k = 0; r = 0; g = 0; return 0; }

	virtual bool IsInside(glm::vec2 pt) { return false; }

	PhysicsObjectType oType;
	glm::vec4 color;

	int lifeSpan = 0;
};