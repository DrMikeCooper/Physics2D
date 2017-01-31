#include <gl_core_4_4.h>
#include <glm\glm\glm.hpp>
#include <glm\glm\ext.hpp>
#include <aie/Gizmos.h>

#include "RigidBody.h"
#include "Plane.h"
#include "Circle.h"
#include "PhysicsApplication.h"

glm::vec2 RigidBody::gravity(0, -1);

RigidBody::RigidBody()
{
	restitution = 0.95f;
}

void RigidBody::Update(float dt)
{
	if (!hasContact)
		awake = true;

	if (awake && !fixed)
	{
		angle += rotation * dt;
		position += velocity * dt;

		//apply air resistance
		velocity *= 0.99f;
		rotation *= 0.99f;

		if (glm::length(velocity) < 0.8f && fabsf(rotation) < 0.005f)
			awake = false;
		
		// apply gravity to the centre of mass as a straight acceleration
		velocity += gravity * dt;
	}

	//store the local axes
	float cs = cosf(angle);
	float sn = sinf(angle);
	localX = glm::vec2(cs, sn);
	localY = glm::vec2(-sn, cs);

	// set this flag to false. We have to touch something else to stay asleep
	hasContact = false;
}

void RigidBody::ApplyForce(glm::vec2 force, glm::vec2 pos)
{
	velocity += force / mass;
	rotation += (force.y * (pos.x - position.x) - force.x * (pos.y - position.y)) / (moment); 
}

void RigidBody::ApplyContactForce(float penetration, glm::vec2 normal)
{
	position += penetration * normal;
}

void RigidBody::ResolveCollision(RigidBody* other, glm::vec2 contact, glm::vec2* direction)
{
	PhysicsApplication::singleStep = true;

	Gizmos::add2DCircle(contact, 1.0f, 12, glm::vec4(1, 1, 1, 1));
	
	if (awake || other->awake)
	{
		awake = true;
		other->awake = true;
	}

	hasContact = true;

	// find the vector between their centres
	glm::vec2 unitDisp = glm::normalize(direction ? *direction : other->position - position);
	// get the component along this axis for each object
	glm::vec2  unitParallel(unitDisp.y, -unitDisp.x);

	// determine the total velocity of the contact points, both linear and rotational, in the direction we're applying a force in
	float r1 = glm::dot(contact - position, unitParallel);
	float r2 = glm::dot(contact - other->position, -unitParallel);
	float v1 = glm::dot(velocity, unitDisp) + r1*rotation;
	float v2 = glm::dot(other->velocity, unitDisp) + r2*other->rotation;

	if (v1 > v2) // they're moving closer
	{
		Gizmos::add2DCircle(contact, 0.9f, 12, glm::vec4(0, 0, 0, 1));

		// calculate equal and opposite forces that will bring the contact points
		// to the same velocity for restituition = 0 case
		float mass1 = 1.0f / (1.0f / mass + (r1*r1) / moment);
		float mass2 = 1.0f / (1.0f / other->mass + (r2*r2) / other->moment);

		glm::vec2 force = (1.0f + restitution)*mass1*mass2 / (mass1 + mass2)*(v1-v2)*unitDisp;

		float ke1 = mass * glm::dot(velocity, velocity) + other->mass * glm::dot(other->velocity, other->velocity) 
			+ moment* rotation*rotation + other->moment * other->rotation * other->rotation;

		glm::vec2 force2 = (1.0f + restitution) * unitDisp * (v1 - v2) / (1.0f / mass + (r1*r1) / moment + 1.0f / other->mass + (r2*r2) / other->moment);
		//apply equal and opposite forces
		ApplyForce(-force, contact);
		other->ApplyForce(force, contact);

		float ke2 = mass * glm::dot(velocity, velocity) + other->mass * glm::dot(other->velocity, other->velocity)
			+ moment* rotation*rotation + other->moment * other->rotation * other->rotation;
		if (ke1 != ke2)
			printf("bounce!"); // this goes wrong in Box-Box collisions
	}
}

// http://www.myphysicslab.com/collision.html

float RigidBody::getEnergy(float& k, float& g, float &r)
{
	g = -mass * glm::dot(position, gravity);
	k = 0.5f * mass * glm::dot(velocity, velocity);
	r = 0.5f * moment * rotation * rotation;
	return   g + k + r;
}

glm::vec2 RigidBody::ToWorld(glm::vec2 pos)
{
	return position + localX * pos.x + localY * pos.y;
}
