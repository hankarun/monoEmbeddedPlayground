#include <iostream>
#include "Application.h"

#pragma comment(lib, "version.lib")

int main(int arg, char* argv[])
{
    Application app;
    app.init();
    app.run();
    return 0;
}
