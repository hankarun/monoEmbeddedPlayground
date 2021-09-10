#pragma once
#include "Layer.h"

class InitialLayer : public Layer
{
public:
    virtual void load() override;

    virtual void draw() override;

    virtual void destroy() override;

};