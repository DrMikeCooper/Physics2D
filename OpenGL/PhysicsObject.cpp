#include <gl_core_4_4.h>
#include <glm\glm\glm.hpp>
#include <aie/Gizmos.h>
#include "PhysicsObject.h"

void PhysicsObject::CheckCollisions(PhysicsObject * other)
{
	if (other->oType == PLANE)
		CollideWithPlane((Plane*)other);
	else if (other->oType == CIRCLE)
		CollideWithCircle((Circle*)other);
	else if (other->oType == BOX)
		CollideWithBox((Box*)other);
}