#pragma once

#include "RigidBody.h"

class Circle : public RigidBody
{
public:
	Circle() { oType = CIRCLE; }
	Circle(glm::vec2 p, glm::vec2 v, float r = 1, float a = 0, float density = 1) 
	{
		position = p;
		velocity = v;
		angle = a;
		rotation = 0;
		mass = 3.14159f * r * r * density;
		radius = r;
		oType = CIRCLE; 
		moment = 0.5f* mass * radius*radius;
		awake = true;
		fixed = false;
	}

	virtual void CollideWithPlane(Plane* plane);
	virtual void CollideWithCircle(Circle* circle);
	virtual void CollideWithBox(Box* box);
	virtual void Draw();
	virtual bool IsInside(glm::vec2 pt);

	float radius = 1;
};