#pragma once

#include <string>

#include "BitEngine/Common/TypeDefinition.h"
#include "BitEngine/Core/Messenger.h"

namespace BitEngine {

class Window;

struct ImGuiRenderEvent {
};

struct BE_API WindowClosedEvent {
    Window* window;
};

struct BE_API WindowResizedEvent {
    Window* window;
    int width;
    int height;
};

enum class BE_API KeyMod : unsigned char {
    NONE = 0x0,
    KFALSE = 0x0,

    SHIFT = 0x0001,
    CTRL = 0x0002,
    ALT = 0x0004,
    SUPER = 0x0008,

    ALT_SHIFT = ALT | SHIFT,
    CTRL_ALT = CTRL | ALT,
    CTRL_SHIFT = CTRL | SHIFT,
    SUPER_ALT = SUPER | ALT,
    SUPER_CTRL = SUPER | CTRL,
    SUPER_SHIFT = SUPER | SHIFT,

    ALT_SHIFT_SUPER = ALT | SHIFT | SUPER,
    SHIFT_CTRL_SUPER = SHIFT | CTRL | SUPER,
    CTRL_ALT_SHIFT = CTRL | ALT | SHIFT,
    CTRL_ALT_SUPER = CTRL | ALT | SUPER,

    KTRUE = 0x0080 // Used to find out the current state of the key_press/release/ispressed events

    // Example: KeyPress/Release with no modifiers
    // 1000 0000 => x = KTRUE
    // Example: KeyPress/Release with CTRL+ALT
    // 1000 0110 => x = KTRUE | CTRL | ALT

    // Receiving the value:
    // Ex1:
    // if (x & KeyMod::KTRUE) --> Doesn't care for modifiers
    //
    // Ex2:
    // if (x & (KeyMod::KTRUE | KeyMod::CTRL | KeyMod::ALT) ) --> Check for key + CTRL + ALT
};

enum class BE_API KeyAction {
    NONE = 0x0,
    RELEASE = 0x1,
    PRESS = 0x2,
    REPEAT = 0x4,

    BASIC = RELEASE | PRESS,
    ALL = RELEASE | PRESS | REPEAT
};

enum class BE_API MouseAction {
    NONE = 0x0,
    RELEASE = 0x1,
    PRESS = 0x2,
    MOVE = 0x4,

};

// Message
struct BE_API MsgKeyboardInput {
    MsgKeyboardInput()
        : key(0)
        , keyAction(KeyAction::NONE)
        , keyMod(KeyMod::KFALSE)
    {
    }
    MsgKeyboardInput(int k, KeyAction ka, KeyMod km)
        : key(k)
        , keyAction(ka)
        , keyMod(km)
    {
    }

    int key;
    KeyAction keyAction;
    KeyMod keyMod;
};

struct BE_API MsgMouseInput {
    MsgMouseInput()
        : button(0)
        , action(MouseAction::NONE)
        , keyMod(KeyMod::KFALSE)
        , x(0)
        , y(0)
    {
    }
    MsgMouseInput(double _x, double _y)
        : button(0)
        , action(MouseAction::MOVE)
        , keyMod(KeyMod::KFALSE)
        , x(_x)
        , y(_y)
    {
    }
    MsgMouseInput(int b, MouseAction ma, KeyMod km, double _x, double _y)
        : button(b)
        , action(ma)
        , keyMod(km)
        , x(_x)
        , y(_y)
    {
    }

    int button;
    MouseAction action;
    KeyMod keyMod;

    double x;
    double y;
};

class Window {
public:
    virtual ~Window(){};

    virtual void drawBegin() = 0;
    virtual void drawEnd() = 0;

    Messenger<WindowResizedEvent> windowResizedSignal;
    Messenger<WindowClosedEvent> windowClosedSignal;

    Messenger<MsgKeyboardInput> keyboardInputSignal;
    Messenger<MsgMouseInput> mouseInputSignal;
};

class WindowConfiguration {
public:
    bool m_Resizable;
    bool m_FullScreen;

    u32 m_Width;
    u32 m_Height;

    u32 m_RedBits;
    u32 m_GreenBits;
    u32 m_BlueBits;
    u32 m_AlphaBits;

    u32 m_DepthBits;
    u32 m_StencilBits;

    std::string m_Title;
};
}
