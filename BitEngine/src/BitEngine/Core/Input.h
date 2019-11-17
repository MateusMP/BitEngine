#pragma once

#include <unordered_map>

#include "bitengine/Core/Messenger.h"
#include "bitengine/Core/Window.h"

namespace BitEngine {
namespace Input {


class BE_API InputReceiver
{
public:
    InputReceiver(Window* window)
        : cursorInScreenX(0.0), cursorInScreenY(0.0)
    {}

    void keyboardInput(int key, int scancode, KeyAction action, int mods);
    void mouseInput(int button, MouseAction action, int mods);
    void mouseInput(double x, double y);

    KeyMod isKeyPressed(int key);
    KeyMod keyReleased(int key);

    double getMouseX() const;
    double getMouseY() const;

    Window* getWindow() const { return window; }

private:
    std::unordered_map<unsigned int, KeyMod> m_keyDown;
    std::unordered_map<unsigned int, KeyMod> m_keyReleased;

    std::unordered_map<unsigned int, KeyMod> m_mouseDown;
    std::unordered_map<unsigned int, KeyMod> m_mouseReleased;

    double cursorInScreenX;
    double cursorInScreenY;

    Window* window;
};
}
}
