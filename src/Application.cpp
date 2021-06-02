#include "Application.h"

Application* Application::app = nullptr;

Application::Application()
{
    app = this;
}

void Application::printInfo()
{
	printf("Application info.\n");
}

Application * Application::instance()
{
    return app;
}

std::stringstream& getStream()
{
    static std::stringstream outputStream;
    return outputStream;
}
