#include <iostream>
#include "Application.h"
#include "InitialLayer.h"

#pragma comment(lib, "version.lib")

int main(int arg, char* argv[])
{
    Application app;
    app.init();
    app.setLayer(std::make_shared<InitialLayer>());
    app.run();
    return 0;
}
