#pragma once

#include "Core/Message.h"
#include "Core/Window.h"

namespace BitEngine{

	class  WindowClosed : public Message<WindowClosed>
	{
	public:
		WindowClosed(Window *w) :window(w)
		{}

		Window* window;
	};

	class WindowCreated : public Message<WindowCreated>
	{
	public:
		WindowCreated(Window *w): window(w){
		}
		Window* window;
	};


}

