#pragma once

#include "bitengine/Core/Window.h"

namespace BitEngine{

	struct MsgWindowClosed
	{
		MsgWindowClosed(Window *w) :window(w)
		{}

		Window* window;
	};

	struct MsgWindowCreated
	{
		MsgWindowCreated(Window *w): window(w)
		{}

		Window* window;
	};


}

