#pragma once
#include "RigidBody.h"

class Spring : public PhysicsObject
{
public:
	Spring() { oType = SPRING;  }
	Spring(RigidBody* b1, RigidBody* b2, float l, float f, glm::vec2 c1 = glm::vec2(0,0), glm::vec2 c2 = glm::vec2(0, 0)) : 
		body1(b1), body2(b2), restLength(l), restoringForce(f), contact1(c1), contact2(c2) { oType = SPRING; }
	RigidBody* body1;
	RigidBody* body2;

	virtual void Update(float dt);
	virtual void Draw();

	virtual void CollideWithPlane(Plane* plane) {};
	virtual void CollideWithCircle(Circle* circle) {};	
	virtual void CollideWithBox(Box* circle) {};

	glm::vec2 contact1;
	glm::vec2 contact2;
	float restLength;
	float restoringForce;
};