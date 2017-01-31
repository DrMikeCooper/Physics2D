#include <gl_core_4_4.h>
#include <glm\glm\glm.hpp>
#include <aie/Gizmos.h>
#include <stdio.h>
#include <math.h>

#include "Box.h"
#include "Plane.h"
#include "Circle.h"

void Box::CollideWithPlane(Plane* plane)
{
	if (fixed)
		return;

	int numContacts = 0;
	glm::vec2 contact(0, 0);
	float contactV = 0;
	float radius = 0.5f * std::fminf(width, height);

	// which side is the centre of mass on?
	float comFromPlane = glm::dot(position - plane->origin, plane->normal);
	float penetration = 0;

	// check all four corners to see if we've hit the plane
	for (float x = -width / 2; x < width; x += width)
	{
		for (float y = -height / 2; y < height; y += height)
		{
			// get the position of the corner in world space
			glm::vec2 p = position + x*localX + y*localY;
			float distFromPlane = glm::dot(p - plane->origin, plane->normal);

			// this is the velocity of the point, taken from linear velocity and angular velocity
			float velocityIntoPlane = glm::dot(velocity + rotation*(-y*localX+ x*localY), plane->normal);

			// if this corner is on the opposite side from the COM, and moving further in, we need to resolve the collision
			if ((distFromPlane >= 0 && comFromPlane < 0 && velocityIntoPlane >= 0) 
				|| (distFromPlane <= 0 && comFromPlane > 0 && velocityIntoPlane <= 0))
			{
				numContacts++;
				contact += p;
				contactV += velocityIntoPlane;
				if (comFromPlane >= 0)
				{
					if (penetration > distFromPlane)
						penetration = distFromPlane;
				}
				else
				{
					if (penetration < distFromPlane)
						penetration = distFromPlane;
				}
			}
		}
	}

	// we've had a hit - typically only two corners can contact
	if (numContacts > 0)
	{
		// get the average collision velocity into the plane (covers linear and rotational velcoity of all corners involved)
		float collisionV = contactV / (float)numContacts;

		// get the acceleration required to stop (restitution = 0) or reverse (restitution = 1) the average velocity into the plane
		glm::vec2 acceleration = -plane->normal * ((1.0f + restitution) * collisionV);
		// and the average position at which we'll apply the force (corner or edge centre)
		glm::vec2 localContact = (contact / (float)numContacts);
		// this is the perpendicular distance we apply the force at relative to the COM, so Torque = F*r
		float r = glm::dot(localContact - position, glm::vec2(plane->normal.y, -plane->normal.x));
		// work out the "effective mass" - this is a combination of moment of inertia and mass, and tells us how much the contact point velocity 
		// will change with the force we're applying
		float mass0 = 1.0f / (1.0f / mass + (r*r) / moment);

		// and apply the force
		ApplyForce(acceleration*mass0, localContact);
		position -= plane->normal* penetration;
		hasContact = true;
	}
}

void Box::CollideWithCircle(Circle* circle)
{
	glm::vec2 circlePos = circle->position - position;
	float w2 = width / 2, h2 = height / 2;

	int numContacts = 0;
	// contact is in our box coordinates
	glm::vec2 contact(0, 0);


	// check the four corners to see if any of them are inside the circle
	for (float x = -w2; x < width; x += width)
	{
		for (float y = -h2; y < height; y += height)
		{
			glm::vec2 p = x*localX + y*localY;
			glm::vec2 dp = p - circlePos;
			if (dp.x*dp.x + dp.y*dp.y < circle->radius*circle->radius)
			{
				numContacts++;
				contact += glm::vec2(x,y);
			}
		}
	}

	// check the four edges for contact points. We transform the circle into the box's coordinate space and compare 
	// its position to the edges in both the x and y direction
	glm::vec2* direction = NULL;

	// get the local position of the circle centre
	glm::vec2 localPos(glm::dot(localX, circlePos), glm::dot(localY, circlePos));
	if (localPos.y < h2 && localPos.y > -h2)
	{
		if (localPos.x > 0 && localPos.x < w2 + circle->radius)
		{
			numContacts++;
			contact += glm::vec2(w2, localPos.y);
			direction = new glm::vec2(localX);
		}
		if (localPos.x < 0 && localPos.x > -(w2 + circle->radius))
		{
			numContacts++;
			contact += glm::vec2(-w2, localPos.y);
			direction = new glm::vec2(-localX);
		}
	}
	if (localPos.x < w2 && localPos.x > -w2)
	{
		if (localPos.y > 0 && localPos.y < h2 + circle->radius)
		{
			numContacts++;
			contact += glm::vec2(localPos.x, h2);
			direction = new glm::vec2(localY);
		}
		if (localPos.y < 0 && localPos.y > -(h2 + circle->radius))
		{
			numContacts++;
			contact += glm::vec2(localPos.x, -h2);
			direction = new glm::vec2(-localY);
		}
	}
	
	if (numContacts > 0)
	{
		// average, and convert back into world coords
		contact = position + (1.0f / numContacts) * (localX*contact.x + localY*contact.y);
		ResolveCollision(circle, contact, direction);
	}

	delete direction;
}

void Box::CollideWithBox(Box* box)
{
	// separating axis theorem
	// for each box, check the extents of the other box, looking for the smallest overlap
	float overlap = FLT_MAX;
	glm::vec2 normal;
	glm::vec2 contact;

	// check both ways
	if (CheckOverlap(box, overlap, contact, normal) && box->CheckOverlap(this, overlap, contact, normal))
	{
		ResolveCollision(box, contact, &normal);
	}

}

// returns true if there is overlap
bool Box::CheckOverlap(Box* box, float& overlap, glm::vec2& contact, glm::vec2& normal)
{
	// our extents in our space are (-w2, w2) in x and (-h2, h2) in y
	float w2 = width / 2, h2 = height / 2;

	float xmin, xmax, ymin, ymax;
	bool first = true;
	glm::vec2 xminPos, xmaxPos, yminPos, ymaxPos;

	// convert every corner of the other box into our space and check extents
	for (float x = -box->width / 2; x < box->width; x += box->width)
	{
		for (float y = -box->height / 2; y < box->height; y += box->height)
		{
			glm::vec2 p = box->position + x*box->localX + y*box->localY; // position in worldspace
			glm::vec2 p0(glm::dot(p - position, localX), glm::dot(p - position, localY)); // position in our box's space

			if (first || p0.x < xmin) {	xmin = p0.x; xminPos = p; }
			if (first || p0.x > xmax) { xmax = p0.x; xmaxPos = p; }
			if (first || p0.y < ymin) { ymin = p0.y; yminPos = p; }
			if (first || p0.y > ymax) { ymax = p0.y; ymaxPos = p; }

			first = false;
		}
	}

	// find any overlap, checking each of our four planes as a potential separating axis
	float overlap0, overlap1, overlap2, overlap3;
	if (xmax < -w2)
		return false;
	else
	{
		overlap0 = xmax - (-w2);
		if (overlap0 < overlap)
		{
			contact = xmaxPos;
			overlap = overlap0;
			normal = -localX;
		}
	}

	if (xmin > w2)
		return false;
	else
	{
		overlap1 = w2 - xmin;
		if (overlap1 < overlap)
		{
			contact = xminPos;
			overlap = overlap1;
			normal = localX;
		}
	}

	if (ymax < -h2)
		return false;
	else
	{
		overlap2 = ymax - (-h2);
		if (overlap2 < overlap)
		{
			contact = ymaxPos;
			overlap = overlap2;
			normal = -localY;
		}
	}

	if (ymin > h2)
		return false;
	else
	{
		overlap3 = h2 - ymin;
		if (overlap3 < overlap)
		{
			contact = yminPos;
			overlap = overlap3;
			normal = localY;
		}
	}

	return true;
}


/*
void Box::CollideWithBox(Box* box)
{
	glm::vec2 boxPos = box->position - position;
	
	{
		glm::vec2 norm;
		glm::vec2 contact;
		float pen = 0;
		int numContacts = 0;
		CheckBoxCorners2(box, contact, numContacts, pen, norm);
		if (box->CheckBoxCorners2(this, contact, numContacts, pen, norm))
			norm = -norm;

		if (pen > 0)
		{
			ResolveCollision(box, contact/float(numContacts), &norm);
			float numDynamic = (fixed ? 0 : 1) + (box->fixed ? 0 : 1);
			if (numDynamic > 0)
			{
				glm::vec2 contactForce = norm * pen / numDynamic;
				if (!fixed) 
					position -= contactForce;
				if (!box->fixed)
					box->position += contactForce;
			}
		}
	}
	return;

	int numContacts = 0;
	// contact is in our box coordinates
	glm::vec2 contact(0, 0);

	glm::vec2 n1, n2;
	
	CheckBoxCorners(box, contact, numContacts, n1);
	box->CheckBoxCorners(this, contact, numContacts, n2);

	// we subtract because they're facing in different directions
	glm::vec2 edgeNormal = glm::normalize(n1-n2); 

	if (numContacts > 0)
	{
		//CheckBoxCorners(box, contact, numContacts, n1);
		//box->CheckBoxCorners(this, contact, numContacts, n2);

		// average, and convert back into world coords
		contact = (1.0f / numContacts) * contact;
		//glm::vec2 axis = -localX;
		ResolveCollision(box, contact, &edgeNormal);
	}
}
*/

// check if any of the other boxes corners are inside us
void Box::CheckBoxCorners(Box* box, glm::vec2& contact, int& numContacts, glm::vec2& edgeNormal)
{
	// get the centre of mass of the other box in our space
	glm::vec2 com(glm::dot(box->position - position, localX), glm::dot(box->position - position, localY));

	for (float x = -box->width / 2; x < box->width; x += box->width)
	{
		for (float y = -box->height / 2; y < box->height; y += box->height)
		{
			glm::vec2 p = box->position + x*box->localX + y*box->localY; // position in worldspace
			glm::vec2 p0(glm::dot(p - position, localX), glm::dot(p - position, localY)); // position in our box's space

			float w2 = width / 2, h2 = height / 2;
			float penetration = width + height; // large value
			if (p0.y < h2 && p0.y > -h2)
			{
				if (com.x > 0 && p0.x < w2)
				{
					numContacts++;
					contact += position + w2 * localX + p0.y * localY;
					edgeNormal = localX;
					penetration = w2 - p0.x;
				}
				if (com.x < 0 && p0.x > -w2)
				{
					numContacts++;
					contact += position - w2 * localX + p0.y * localY;
					edgeNormal = -localX;
					penetration = w2 + p0.x;
				}
			}
			if (p0.x < w2 && p0.x > -w2)
			{
				if (com.y > 0 && p0.y < h2)
				{
					numContacts++;
					contact += position + p0.x * localX + h2 * localY;
					float pen0 = h2 - p0.y;
					if (pen0 < penetration)
						edgeNormal = localY;
				}
				if (com.y < 0 && p0.y > -h2)
				{
					numContacts++;
					contact += position + p0.x * localX - h2 * localY;
					float pen0 = h2 + p0.y;
					if (pen0 < penetration)
						edgeNormal = -localY;
				}
			}

		}
	}

}

// check if any of the other boxes corners are inside us
bool Box::CheckBoxCorners2(Box* box, glm::vec2& contact, int& numContacts, float &pen, glm::vec2& edgeNormal)
{
	float minX, maxX, minY, maxY;
	float w2 = width / 2, h2 = height / 2;
	int numLocalContacts = 0;
	glm::vec2 localContact;

	bool first = true;
	for (float x = -box->width / 2; x < box->width; x += box->width)
	{
		for (float y = -box->height / 2; y < box->height; y += box->height)
		{
			glm::vec2 p = box->position + x*box->localX + y*box->localY; // position in worldspace
			glm::vec2 p0(glm::dot(p - position, localX), glm::dot(p - position, localY)); // position in our box's space

			if (first || p0.x < minX) minX = p0.x;
			if (first || p0.x > maxX) maxX = p0.x;
			if (first || p0.y < minY) minY = p0.y;
			if (first || p0.y > maxY) maxY = p0.y;

			if (p0.x >= -w2 && p0.x <= w2 && p0.y >=-h2 && p0.y <= h2)
			{
				numLocalContacts++;
				localContact += p0;
			}
	 		first = false;
		}
	} 

	if (maxX <-w2 || minX >w2 || maxY<-h2 || minY >h2)
		return false;
	if (numLocalContacts == 0)
		return false;
	
	bool res = false;

	contact += position + (localContact.x*localX + localContact.y*localY) / (float)numLocalContacts;
	numContacts++;

	float pen0 = w2 - minX;
	if (pen0 > 0 && (pen0 < pen || pen==0))
	{
		edgeNormal = localX;
		pen = pen0;
		res = true;
	}
	pen0 = maxX + w2;
	if (pen0 > 0 && (pen0 < pen || pen == 0))
	{
		edgeNormal = -localX;
		pen = pen0;
		res = true;
	}
	pen0 = h2 - minY;
	if (pen0 > 0 && (pen0 < pen || pen == 0))
	{
		edgeNormal = localY;
		pen = pen0;
		res = true;
	}
	pen0 = maxY + h2;
	if (pen0 > 0 && (pen0 < pen || pen == 0))
	{
		edgeNormal = -localY;
		pen = pen0;
		res = true;
	}
	return res;
}

void Box::Draw()
{
	glm::vec2 p1 = position - localX * width / 2.0f - localY * height / 2.0f;
	glm::vec2 p2 = position + localX * width / 2.0f - localY * height / 2.0f;
	glm::vec2 p3 = position - localX * width / 2.0f + localY * height / 2.0f;
	glm::vec2 p4 = position + localX * width / 2.0f + localY * height / 2.0f;
	glm::vec4 col = awake ? color : glm::vec4(0,1,1,1);
	glm::vec4 col2 = awake ? glm::vec4(1, 1, 0, 1) : glm::vec4(0, 1, 1, 1);
	Gizmos::add2DTri(p1, p2, p4, col);
	Gizmos::add2DTri(p1, p4, p3, col2);
}

bool Box::IsInside(glm::vec2 pt)
{
	pt -= position;
	glm::vec2 boxPt(glm::dot(pt, localX), glm::dot(pt, localY));
	return (fabs(boxPt.x) < width*0.5f && fabs(boxPt.y) < height*0.5f);
}