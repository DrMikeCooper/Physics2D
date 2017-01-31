#include <gl_core_4_4.h>
#include <glm\glm\glm.hpp>
#include <aie/Gizmos.h>

#include "Circle.h"
#include "Plane.h"
#include "Box.h"

void Circle::CollideWithPlane(Plane* plane)
{
	float distFromPlane = (position.x - plane->origin.x)* plane->normal.x + (position.y - plane->origin.y)* plane->normal.y;
	float velocityIntoPlane = velocity.x * plane->normal.x + velocity.y * plane->normal.y;

	// one sided plane is infinitely thick behind the normal, and thus ejects anything inside it no matter how far in
	if (plane->oneSided)
	{
		if (distFromPlane < radius && velocityIntoPlane < 0)
		{
			ApplyForce(-mass*(plane->normal * (1.0f + restitution) * velocityIntoPlane), position);
			ApplyContactForce(radius - distFromPlane, plane->normal);
		}
	}
	else // two sided plane - which side you're on is determined by where the centre is. Easier to set up but can lead to tunnelling at high speeds.
	{
		if (distFromPlane > 0 && distFromPlane < radius && velocityIntoPlane < 0)
		{
			ApplyForce(-mass*(plane->normal * (1.0f + restitution) * velocityIntoPlane), position);
		}

		if (distFromPlane < 0 && distFromPlane > -radius && velocityIntoPlane > 0)
		{
			ApplyForce(-mass*(plane->normal *  (1.0f + restitution) * velocityIntoPlane), position);
		}
	}
}

void Circle::CollideWithCircle(Circle* circle)
{
	// find the vector between their centres
	glm::vec2 disp = circle->position - position;
	// and get its length, so we can normalise later
	float d = sqrtf(disp.x*disp.x + disp.y*disp.y);

	// if we're closer than the two radii, we're touching
	// note that the position we pass in here is incorrect (its the midpoint bewteen the radii rather than the actual contact point)
	// but anywhere on the line between the radii will be OK because no torque will be applied.
	if (d > 0 && d < (radius + circle->radius))
		ResolveCollision(circle, 0.5f*(position+circle->position));
}

void Circle::CollideWithBox(Box* box)
{
	box->CollideWithCircle(this);
}

void Circle::Draw()
{
	Gizmos::add2DCircle(position, radius, 32, color);
	// add a "highlight" marker so we can see rotation
	Gizmos::add2DCircle(position + radius*0.5f*localX, radius*0.25f, 16, glm::vec4(1, 1, 1, 1));
}

bool Circle::IsInside(glm::vec2 pt)
{
	pt -= position;
	return ((pt.x*pt.x+pt.y*pt.y) < radius*radius);
}