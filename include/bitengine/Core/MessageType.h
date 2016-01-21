#pragma once

#include "Core/Window.h"

namespace BitEngine{

struct WindowClosed
{
	WindowClosed(Window *w) :window(w)
	{}

	Window* window;
};

struct WindowCreated
{
    WindowCreated(Window *w): window(w){
    }
    Window* window;
};


}
