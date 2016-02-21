#pragma once

#include "Core/Message.h"
#include "Core/Window.h"

namespace BitEngine{

	class  MsgWindowClosed : public Message<MsgWindowClosed>
	{
	public:
		MsgWindowClosed(Window *w) :window(w)
		{}

		Window* window;
	};

	class MsgWindowCreated : public Message<MsgWindowCreated>
	{
	public:
		MsgWindowCreated(Window *w): window(w){
		}
		Window* window;
	};


}

