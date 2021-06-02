#pragma once
#include <iostream>
#include <sstream>

class Application
{
public:
    Application();

	void printInfo();

    static Application* instance();

	static Application* app;
};

std::stringstream& getStream();