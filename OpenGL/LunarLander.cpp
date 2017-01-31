#include <gl_core_4_4.h>
#include <glm\glm\glm.hpp>
#include <aie/Gizmos.h>

#include "PhysicsApplication.h"
#include "LunarLander.h"

void LunarLander::Update(float dt)
{
	Circle::Update(dt);

	pod1 = position + (-localX - 0.5f * localY)*radius;
	pod2 = position + (localX - 0.5f * localY)*radius;

	GLFWwindow* window = PhysicsApplication::theApp->window;

	if (glfwGetKey(window, GLFW_KEY_Z))
	{
		Circle* c = new Circle(pod1 + 0.5f*localY*radius, -10 * localY, 0.1f, 0);
		c->lifeSpan = 100;
		PhysicsApplication::theApp->m_physicsObjects.push_front(c);
		this->ApplyForce(localY, pod1);
	}
	if (glfwGetKey(window, GLFW_KEY_C))
	{
		Circle* c = new Circle(pod2 + 0.5f*localY*radius, -10*localY, 0.1f, 0);
		c->lifeSpan = 100;
		PhysicsApplication::theApp->m_physicsObjects.push_front(c);
		this->ApplyForce(localY, pod2);
	}
}

void LunarLander::Draw()
{
	Circle::Draw();
	Gizmos::add2DCircle(pod1, radius*0.5f, 16, vec4(0,1,1,1));
	Gizmos::add2DCircle(pod2, radius*0.5f, 16, vec4(0, 1, 1, 1));
}
