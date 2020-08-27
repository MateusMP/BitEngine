
#include "BitEngine/Core/Input.h"
#include "BitEngine/Core/Messenger.h"
#include "BitEngine/Core/Logger.h"

namespace BitEngine {
namespace Input {

    void InputReceiver::keyboardInput(int key, int scancode, KeyAction action, int mods)
    {
        switch (action) {
        case KeyAction::REPEAT:
            m_keyDown[key] = (KeyMod)(((unsigned char)KeyMod::KTRUE) | (unsigned char)mods);
            m_keyReleased[key] = KeyMod::KFALSE;
            break;

        case KeyAction::PRESS:
            m_keyDown[key] = (KeyMod)(((unsigned char)KeyMod::KTRUE) | (unsigned char)mods);
            m_keyReleased[key] = KeyMod::KFALSE;
            break;

        case KeyAction::RELEASE:
            m_keyReleased[key] = (KeyMod)(((unsigned char)KeyMod::KTRUE) | (unsigned char)mods);
            m_keyDown[key] = KeyMod::KFALSE;
            break;

        default:
            LOG(EngineLog, BE_LOG_WARNING) << "Invalid key action: " << ((int)action);
            return;
        }

        window->keyboardInputSignal.emit(MsgKeyboardInput(key, action, (KeyMod)mods));
    }

    void InputReceiver::mouseInput(int button, MouseAction action, int mods)
    {
        switch (action) {
        case MouseAction::PRESS:
            m_mouseDown[button] = (KeyMod)(((unsigned char)KeyMod::KTRUE) | (unsigned char)mods);
            m_mouseReleased[button] = KeyMod::KFALSE;
            break;

        case MouseAction::RELEASE:
            m_mouseReleased[button] = (KeyMod)(((unsigned char)KeyMod::KTRUE) | (unsigned char)mods);
            m_mouseDown[button] = KeyMod::KFALSE;
            break;

        default:
            LOG(EngineLog, BE_LOG_WARNING) << "Invalid mouse action: " << ((int)action);
            return;
        }

        window->mouseInputSignal.emit(MsgMouseInput(button, action, (KeyMod)mods, cursorInScreenX, cursorInScreenY));
    }

    void InputReceiver::mouseInput(double x, double y)
    {
        cursorInScreenX = x;
        cursorInScreenY = y;

        window->mouseInputSignal.emit(MsgMouseInput(cursorInScreenX, cursorInScreenY));
    }

    KeyMod InputReceiver::isKeyPressed(int key)
    {
        auto k = m_keyDown.find(key);
        if (k != m_keyDown.end())
            return k->second;

        return KeyMod::KFALSE;
    }

    KeyMod InputReceiver::keyReleased(int key)
    {
        auto k = m_keyReleased.find(key);
        if (k != m_keyReleased.end())
            return k->second;

        return KeyMod::KFALSE;
    }

    double InputReceiver::getMouseX() const
    {
        return cursorInScreenX;
    }

    double InputReceiver::getMouseY() const
    {
        return cursorInScreenY;
    }
}
}