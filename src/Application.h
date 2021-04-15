#pragma once
#include <iostream>

class Application
{
public:
    Application();

	void printInfo();

    static Application* instance();

	static Application* app;
};