#pragma once

#include <conio.h>
#include <gl_core_4_4.h>
#include <glfw-3.1.2\include\GLFW\glfw3.h>
#include <aie/Gizmos.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/ext.hpp>
#include <list>

#include "Application.h"
#include "Camera.h"
#include "Model.h"
#include "PhysicsObject.h"

class PhysicsApplication : public Application
{
public:
	static PhysicsApplication* theApp;
	PhysicsApplication() { theApp = this; }

	virtual bool startup();
	virtual void shutdown();
	virtual bool update();
	virtual void draw();

	void Reset();
	void ResetPool();
	void ResetLunarLander();
	void ResetSprings();
	void ResetBasic();
	void ResetTwoBoxes();

	int day = 0;

	Camera camera;
	Model sun;

	GLuint shaderID;

	float dt = 1.0f / 60.0f;

	std::list<PhysicsObject*> m_physicsObjects;

	glm::vec2 m_contactPoint;
	glm::vec2 m_mousePoint;
	bool m_mouseDown;

	static bool singleStep;
};
