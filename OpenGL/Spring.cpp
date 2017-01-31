#include <gl_core_4_4.h>
#include <glm\glm\glm.hpp>
#include <aie/Gizmos.h>

#include "Spring.h"

void Spring::Update(float dt)
{
	glm::vec2 p2 = body2->ToWorld(contact2);
	glm::vec2 p1 = body1->ToWorld(contact1);
	glm::vec2 dist = p2 - p1;
	float len = sqrtf(dist.x*dist.x + dist.y* dist.y);

	// apply damping
	glm::vec2 dv = body2->velocity - body1->velocity;

	float damping = 0.1f;
	glm::vec2 force = dist * restoringForce * (restLength - len) - damping * dv;


	body1->ApplyForce(-force*dt, p1);
	body2->ApplyForce(force*dt, p2);
}

void Spring::Draw()
{
	Gizmos::add2DLine(body1->ToWorld(contact1), body2->ToWorld(contact2), glm::vec4(1, 1, 1, 1));
}
