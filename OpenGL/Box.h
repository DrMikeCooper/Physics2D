#pragma once
#include "RigidBody.h"

class Box : public RigidBody
{
public:
	Box() { oType = BOX; }
	Box(glm::vec2 p, glm::vec2 v, float a = 0, float w = 1, float h = 1, float density = 1, bool fx = false) : width(w), height(h)
	{
		position = p; 
		velocity = v;
		angle = a;
		rotation = 0;
		mass = density * width * height;
		oType = BOX; 
		moment = 1.0f/12.0f * mass * (width*width +height*height);
		awake = true;
		fixed = fx;
		restitution = 0.95f;

		//store the local axes
		float cs = cosf(angle);
		float sn = sinf(angle);
		localX = glm::vec2(cs, sn);
		localY = glm::vec2(-sn, cs);
	}

	virtual void CollideWithPlane(Plane* plane);
	virtual void CollideWithCircle(Circle* circle);
	virtual void CollideWithBox(Box* box);
	virtual void Draw();
	virtual bool IsInside(glm::vec2 pt);

	bool Box::CheckOverlap(Box* box, float& overlap, glm::vec2& contact, glm::vec2& normal);
	void CheckBoxCorners(Box* box, glm::vec2& contact, int& numContacts, glm::vec2& edgeNormal);
	bool CheckBoxCorners2(Box* box, glm::vec2& contact, int& numContacts, float& pen, glm::vec2& edgeNormal);
	float width, height;
};