#include <glm\glm\ext.hpp>
#include "PhysicsApplication.h"
#include "Circle.h"
#include "Plane.h"
#include "Box.h"
#include "Spring.h"
#include "LunarLander.h"

using namespace glm;

PhysicsApplication* PhysicsApplication::theApp = NULL;
bool PhysicsApplication::singleStep = false;

bool PhysicsApplication::startup()
{
	glfwInit();

	window = glfwCreateWindow(1280, 720,
		"Computer Graphics",
		nullptr, nullptr);
	if (window == nullptr) {
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);

	if (ogl_LoadFunctions() == ogl_LOAD_FAILED) {
		glfwDestroyWindow(window);
		glfwTerminate();
		return false;
	}

	auto major = ogl_GetMajorVersion();
	auto minor = ogl_GetMinorVersion();
	printf("GL: %i.%i\n", major, minor);


	glClearColor(0.0f, 0.0f, 0.25f, 1);
	glEnable(GL_DEPTH_TEST); // enables the depth buffer

	Gizmos::create(65335U, 65535U, 65535U, 65535U);

	Reset();

	camera.radius = 1;

	return true;
}

void PhysicsApplication::shutdown()
{
	Gizmos::destroy();

	glfwDestroyWindow(window);


	glfwTerminate();
}

bool PhysicsApplication::update()
{
	// moved here for visual debugging tricks
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Gizmos::clear();

	camera.update(window);

	float energy = 0;
	float g0=0, k0=0, r0 = 0, g, k, r;

	if (glfwGetKey(window, GLFW_KEY_O))
		singleStep = false;

	if (!singleStep)
	{
		for (auto it = m_physicsObjects.begin(); it != m_physicsObjects.end(); it++)
		{
			PhysicsObject* obj = *it;

			if (obj->lifeSpan > 0)
			{
				obj->lifeSpan--;
				if (obj->lifeSpan == 0)
				{
					delete obj;
					m_physicsObjects.erase(it++);
					continue;
				}
			}
			obj->Update(dt);

			// accumulate all energy components
			energy += obj->getEnergy(k, g, r);
			k0 += k; g0 += g; r0 += r;

			// collisions - check this object with everything further up the list
			for (auto it2 = it; it2 != m_physicsObjects.end(); it2++)
			{
				if (it != it2)
				{
					PhysicsObject* obj2 = *it2;
					obj2->CheckCollisions(obj);

				}
			}
		}
	}
	
	printf("%10.3f + %10.3f + %10.3f = %10.3f\n", k0, r0, g0, k0+r0+g0);
	Sleep(1000*dt);

	// check for mousedown
	bool mouseDown = glfwGetMouseButton(window, 0);

	double x0, y0;
	glfwGetCursorPos(window, &x0, &y0);
	mat4 view = camera.getView();
	mat4 projection = camera.getProjection();

	glm::vec3 windowCoordinates = glm::vec3(x0, y0, 0);
	glm::vec4 viewport = glm::vec4(0.0f, 0.0f, 1280, 720);
	glm::vec3 worldCoordinates = glm::unProject(windowCoordinates, view, projection, viewport);

	m_mousePoint = vec2(worldCoordinates[0] * camera.getDistance(), worldCoordinates[1] * (-camera.getDistance()));

	if (mouseDown != m_mouseDown)
	{
		if (mouseDown)
		{
			m_contactPoint = m_mousePoint;
		}
		else
		{
			//m_physicsObjects.push_back(new Box(m_mousePoint, vec2(0, 0), 1, 0, 0.5f, 0.5f));
			for (auto it = m_physicsObjects.begin(); it != m_physicsObjects.end(); it++)
			{
				PhysicsObject* obj = *it;
				if (obj->IsInside(m_mousePoint))
				{
					// must be a RigidBody, so we can safely cast!
					RigidBody* rb = (RigidBody*)obj;
					rb->ApplyForce(2.0f*(m_mousePoint - m_contactPoint), m_contactPoint);
				}
			}
		}
		m_mouseDown = mouseDown;
	}

	if (glfwGetKey(window, GLFW_KEY_P))
		Reset();

	return (glfwWindowShouldClose(window) == false && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS);
}

void PhysicsApplication::draw()
{
	mat4 view = camera.getView();
	mat4 projection = camera.getProjection();

	//draw a background grid
	Gizmos::addTransform(glm::mat4(1));
	vec4 orange(1, 0.7f, 0.2f, 1.0f);
	vec4 white(1);
	vec4 black(0, 0, 0, 1);
	for (int i = 0; i < 21; ++i) 
	{
		Gizmos::add2DLine(vec2(-10 + i, 10), vec2(-10 + i, -10), i == 10 ? orange : white);
		Gizmos::add2DLine(vec2(10, -10 + i), vec2(-10, -10 + i), i == 10 ? orange : white);
	}

	// add a gizmo for every object in the scene
	for (auto it = m_physicsObjects.begin(); it != m_physicsObjects.end(); it++)
	{
		PhysicsObject* obj = *it;
		obj->Draw();
	}

	if (m_mouseDown)
		Gizmos::add2DLine(m_contactPoint, m_mousePoint, white);

	// draw the 2D objects
	Gizmos::draw2D(projection * view);
	
	glfwSwapBuffers(window);
	glfwPollEvents();

	day++;
}

void PhysicsApplication::Reset()
{
	m_physicsObjects.clear();
	//ResetPool();
	//ResetLunarLander();
	//ResetSprings();
	//ResetBasic();
	ResetTwoBoxes();
}

void PhysicsApplication::ResetPool()
{
	RigidBody::gravity.y = 0;
	m_physicsObjects.push_back(new Box(vec2(-10, 0), vec2(0, 0), 0, 1.0f, 10.0f, 10, true));
	m_physicsObjects.push_back(new Box(vec2(10, 0), vec2(0, 0), 0, 1.0f, 10.0f, 10, true));
	m_physicsObjects.push_back(new Box(vec2(-5, 7), vec2(0, 0), 0, 8.0f, 1.0f, 10, true));
	m_physicsObjects.push_back(new Box(vec2(5, 7), vec2(0, 0), 0, 8.0f, 1.0f, 10, true));
	m_physicsObjects.push_back(new Box(vec2(-5, -7), vec2(0, 0), 0, 8.0f, 1.0f, 10, true));
	m_physicsObjects.push_back(new Box(vec2(5, -7), vec2(0, 0), 0, 8.0f, 1.0f, 10, true));

	m_physicsObjects.push_back(new Circle(vec2(-5, 0), vec2(0, 0), 0.5f));

	m_physicsObjects.push_back(new Circle(vec2(5, 0), vec2(0, 0), 0.5f));
	m_physicsObjects.push_back(new Circle(vec2(6, 1), vec2(0, 0), 0.5f));
	m_physicsObjects.push_back(new Circle(vec2(6, -1), vec2(0, 0), 0.5f));
}

void PhysicsApplication::ResetLunarLander()
{
	RigidBody::gravity.y = -9;
	m_physicsObjects.push_back(new LunarLander(vec2(0, 0)));

	m_physicsObjects.push_back(new Plane(vec2(0, -5), vec2(0, 1)));
}

void PhysicsApplication::ResetSprings()
{
	Circle* c[25];
	for (int x = 0; x < 5; x++)
	{
		for (int y = 0; y < 5; y++)
		{
			Circle* c0 = new Circle(vec2(x * 2 - 5, y * 2), vec2(0, 0), 1.0f);
			c[x + y * 5] = c0;
			m_physicsObjects.push_back(c0);
			if (x > 0)
				m_physicsObjects.push_back(new Spring(c0, c[(x - 1) + y * 5], 2.0f, 150.0f, vec2(-0.5, 0), vec2(0.5, 0)));
			if (y > 0)
				m_physicsObjects.push_back(new Spring(c0, c[x + (y - 1) * 5], 2.0f, 150.0f, vec2(0, -0.5), vec2(0, 0.5)));
		}
	}

	m_physicsObjects.push_back(new Plane(vec2(0, -5), vec2(0, 1)));
	m_physicsObjects.push_back(new Plane(vec2(-7.1f, 0.0f), vec2(0.707f, 0.707f)));
	m_physicsObjects.push_back(new Plane(vec2(7.1f, 0.0f), vec2(-1, 0)));
}

void PhysicsApplication::ResetBasic()
{
	Circle* c1 = new Circle(vec2(0, 5), vec2(-1, 0), 2.0f);
	Circle* c2 = new Circle(vec2(2, 5), vec2(1, 0), 1.0f);
	Circle* c3 = new Circle(vec2(-2, 5), vec2(2, 0), 0.5f, 100.0f);
	m_physicsObjects.push_back(c1);
	m_physicsObjects.push_back(c2);
	m_physicsObjects.push_back(c3);
	m_physicsObjects.push_back(new Spring(c1, c2, 6, 10));
	m_physicsObjects.push_back(new Spring(c2, c3, 6, 10));
	m_physicsObjects.push_back(new Spring(c1, c3, 6, 10));

	m_physicsObjects.push_back(new Circle(vec2(0, 8), vec2(0.5f, 0), 0.5f));
	m_physicsObjects.push_back(new Circle(vec2(0, 10), vec2(-0.5f, 0), 0.5f));
	m_physicsObjects.push_back(new Circle(vec2(0, 12), vec2(0.5f, 0), 0.5f));

	m_physicsObjects.push_back(new Box(vec2(-1, -4), vec2(0, 0), 0, 0.5f, 2.0f));
	m_physicsObjects.push_back(new Box(vec2(1, -4), vec2(0, 0), 0, 0.5f, 2.0f));
	m_physicsObjects.push_back(new Box(vec2(0, -2.75f), vec2(0, 0), 0.05f, 3.0f, 0.5f));

	m_physicsObjects.push_back(new Box(vec2(0, 20), vec2(0, 0), 0, 1.0f, 3.0f));

	m_physicsObjects.push_back(new Circle(vec2(0, 22), vec2(0.5f, 0), 2.0f));

	m_physicsObjects.push_back(new Box(vec2(0, 4), vec2(0, 0), 0, 1.0f, 3.0f));

	m_physicsObjects.push_back(new Plane(vec2(0, -5), vec2(0, 1)));
	m_physicsObjects.push_back(new Plane(vec2(-7.1f, 0.0f), vec2(0.707f, 0.707f)));
	m_physicsObjects.push_back(new Plane(vec2(7.1f, 0.0f), vec2(-1, 0)));
}

void PhysicsApplication::ResetTwoBoxes()
{
	static float angle1 = 0;
	static float angle2 = 0;

	RigidBody::gravity.y = -9;
	//m_physicsObjects.push_back(new Box(vec2(0, 1.5f), vec2(0, 0), 0, 0.5f, 0.5f, 10));
	//m_physicsObjects.push_back(new Box(vec2(0, -0.5), vec2(0, 0), 0, 0.5f, 0.5f, 10));
	m_physicsObjects.push_back(new Box(vec2(1, 5), vec2(0, 0), angle1 * 3.1415f/180.0f, 3.0f, 1.0f));
	m_physicsObjects.push_back(new Box(vec2(0, 9), vec2(0, 0), angle2 * 3.1415f/180.0f, 3.0f, 1.0f));
	//m_physicsObjects.push_back(new Box(vec2(-4, 11), vec2(0, 0), -45, 3.0f, 1.0f));

	//m_physicsObjects.push_back(new Circle(vec2(0, 22), vec2(0.5f, 0), 2.0f, 0.1f));

	m_physicsObjects.push_back(new Plane(vec2(0, -5), vec2(0, 1)));
	m_physicsObjects.push_back(new Plane(vec2(-7.1f, 0.0f), vec2(0.707f, 0.707f)));
	m_physicsObjects.push_back(new Plane(vec2(7.1f, 0.0f), vec2(-1, 0)));
}