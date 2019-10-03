#pragma once

#include <GLFW/glfw3.h>

#include "bitengine/Core/Messenger.h"
#include "Platform/glfw/GLFW_VideoSystem.h"

namespace BitEngine {

struct ImGuiRenderEvent {
	
};

class GLFW_ImGuiSystem : public MessengerEndpoint
{
	public:
		GLFW_ImGuiSystem(Messenger *m) : MessengerEndpoint(m)
		{
		}
		~GLFW_ImGuiSystem() {}

		/**
		* Initializes a window and openGL related stuff (Extensions and functions)
		* Currently using GLFW and GLEW
		*/
		bool setup(BitEngine::Window* window);
		void update();
};

}
