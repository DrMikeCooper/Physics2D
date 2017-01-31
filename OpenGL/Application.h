#pragma once

class Application
{
public:
	virtual bool startup() = 0;
	virtual void shutdown() = 0;
	virtual bool update() = 0;
	virtual void draw() = 0;

	GLFWwindow* window;
};
