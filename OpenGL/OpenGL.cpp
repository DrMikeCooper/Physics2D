// OpenGL.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <thread>

#include "PhysicsApplication.h"

using namespace glm;

int main()
{
	Application* app = new PhysicsApplication();

	if (!app->startup())
		return -1;

	while (app->update())
	{
		app->draw();
	}

	app->shutdown();

    return 0;
}

