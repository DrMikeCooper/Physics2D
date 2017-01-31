#pragma once
#include "PhysicsObject.h"

class RigidBody : public PhysicsObject
{
public:
	RigidBody();

	virtual void Update(float dt);

	// force and pos are in world coordinates
	virtual void ApplyForce(glm::vec2 force, glm::vec2 pos);
	void ApplyContactForce(float penetration, glm::vec2 normal);

	virtual float getEnergy(float& k, float& g, float &r);

	void ResolveCollision(RigidBody* other, glm::vec2 contact, glm::vec2* direction = NULL);

	glm::vec2 ToWorld(glm::vec2 pos);

	static glm::vec2 gravity;

	glm::vec2 position;
	glm::vec2 velocity;
	float angle;
	float rotation;
	float mass;
	float moment;

	float restitution;

	bool awake;
	bool fixed;
	bool hasContact = false;

	glm::vec2 localX, localY;
};