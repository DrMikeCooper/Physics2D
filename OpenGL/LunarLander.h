#pragma once
#include "Circle.h"

class LunarLander : public Circle 
{
public:
	LunarLander() {}
	LunarLander(glm::vec2 p)
	{
		position = p;
		radius = 1;
		fixed = false;
		awake = true;
		radius = 1;
		mass = 2;
		angle = 0; 
		rotation = 0;
		oType = CIRCLE;
		moment = 15.0f * 0.5f* mass * radius*radius;
	}

	virtual void Update(float dt);
	virtual void Draw();

	vec2 pod1;
	vec2 pod2;

};
