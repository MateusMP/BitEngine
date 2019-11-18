#pragma once

#include <unordered_map>

#include "BitEngine/Core/Input.h"
#include "BitEngine/Core/Window.h"

namespace BitEngine {

class InputSystem
{
public:
    InputSystem() {}
    virtual ~InputSystem() {}

    virtual bool init() = 0;
    virtual void shutdown() = 0;
    virtual void update() = 0;

    virtual KeyMod isKeyPressed(int key) = 0;
    virtual KeyMod keyReleased(int key) = 0;

    virtual double getMouseX() const = 0;
    virtual double getMouseY() const = 0;

};
}
