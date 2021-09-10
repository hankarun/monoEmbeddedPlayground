#pragma once
#include <iostream>
#include <sstream>
#include <vector>

#include "ImGuiFrame.h"
#include "Layer.h"

class Application
{
public:
    Application();

    void init();

    void run();

	void printInfo();

    static Application* instance();

	static Application* app;

    void setLayer(const std::shared_ptr<Layer>& layer);

private:
    ImGuiFrame frame;
    std::shared_ptr<Layer> layer;
};

std::stringstream& getStream();