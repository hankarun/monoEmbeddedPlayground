#include "Application.h"
#include "../imgui/imgui.h"
#include "windows.h"

Application* Application::app = nullptr;

Application::Application()
{
    app = this;
}

void Application::init()
{
    printf("Engine initializing\n");
    frame.Initialize();
}

void Application::run()
{
    printf("Engine running\n");
    bool runing = true;
    while (runing) {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                runing = false;
        }

        if (!runing)
            break;

        frame.NewFrame();

        layer->draw();

        frame.Render();
    }
    printf("Engine closing\n");

    layer->destroy();

    frame.Destroy();
}

void Application::printInfo()
{
    printf("Application info.\n");
}

Application* Application::instance()
{
    return app;
}

void Application::setLayer(const std::shared_ptr<Layer>& layer)
{
    this->layer = layer;
    layer->load();
}

std::stringstream& getStream()
{
    static std::stringstream outputStream;
    return outputStream;
}
