#pragma once
#include <glfw-3.1.2\include\GLFW\glfw3.h>
#include <glm/glm/glm.hpp>
using namespace glm;

class Camera
{
public:
	mat4 getView();
	mat4 getProjection();

	void update(GLFWwindow* window);

	float angle = 0;
	float radius = 1;

	float getDistance() { return 10 * radius; }
};