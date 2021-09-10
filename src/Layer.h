#pragma once

class Layer
{
public:
    virtual ~Layer() = default;
    virtual void load() = 0;
    virtual void draw() = 0;
    virtual void destroy() = 0;
};