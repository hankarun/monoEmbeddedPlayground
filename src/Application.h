#pragma once
#include <iostream>

class Application
{
public:
	Application()
	{
		app = this;
	}

	void printInfo();

	static Application* instance()
	{
		return app;
	}

	static Application* app;
};