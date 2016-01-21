#include "Defaultbackends/glfw/GLFW_InputDriver.h"

std::unordered_map<GLFW_VideoDriver::Window_glfw*, BitEngine::InputReceiver> GLFW_InputDriver::inputReceivers;


void GLFW_InputDriver::inputWindowCreated(BitEngine::Window* window)
{
	GLFW_VideoDriver::Window_glfw* w = static_cast<GLFW_VideoDriver::Window_glfw*>(window);

	// Creates instance for this window
	inputReceivers[w];

	// Define callback for functions
	glfwSetKeyCallback(w->m_glfwWindow, GlfwKeyCallback);
	glfwSetMouseButtonCallback(w->m_glfwWindow, GlfwMouseCallback);
	glfwSetCursorPosCallback(w->m_glfwWindow, GlfwMousePosCallback);
}

void GLFW_InputDriver::inputWindowDestroyed(BitEngine::Window* window)
{
	inputReceivers.erase(static_cast<GLFW_VideoDriver::Window_glfw*>(window));
}


BitEngine::InputReceiver::KeyMod GLFW_InputDriver::isKeyPressed(int key)
{
	auto w = inputReceivers.begin();
	if (w == inputReceivers.end())
		return BitEngine::InputReceiver::KeyMod::FALSE;

	return w->second.isKeyPressed(key);
}

BitEngine::InputReceiver::KeyMod GLFW_InputDriver::keyReleased(int key)
{
	auto w = inputReceivers.begin();
	if (w == inputReceivers.end())
		return BitEngine::InputReceiver::KeyMod::FALSE;

	return w->second.keyReleased(key);
}

double GLFW_InputDriver::getMouseX() const
{
	auto w = inputReceivers.begin();
	if (w == inputReceivers.end())
		return -1;

	return w->second.getMouseX();
}

double GLFW_InputDriver::getMouseY() const
{
	auto w = inputReceivers.begin();
	if (w == inputReceivers.end())
		return -1;

	return w->second.getMouseY();
}

void GLFW_InputDriver::poolEvents()
{
	glfwPollEvents();
}

GLFW_VideoDriver::Window_glfw* GLFW_InputDriver::FindGLFWwindow(GLFWwindow* window)
{
	for (auto& it = inputReceivers.begin(); it != inputReceivers.end(); ++it) {
		GLFW_VideoDriver::Window_glfw* glfwW = it->first;
		if (glfwW->m_glfwWindow == window) {
			return glfwW;
		}
	}

	return nullptr;
}


void GLFW_InputDriver::GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_UNKNOWN) {
		LOGTO(Warning) << "Unknown key: scancode: " << scancode << " Action: " << action << " Mods: " << mods << BitEngine::endlog;
	}

	// LOGTO(Verbose) << "Key Input on window " << window << " key: " << key << " scancode: " << scancode << " Action: " << action << " Mods: " << mods << endlog;
	GLFW_VideoDriver::Window_glfw *w = FindGLFWwindow(window);

	if (w == nullptr) {
		LOGTO(Warning) << "Unhandled key callback!" << BitEngine::endlog;
		return;
	}

	auto it = inputReceivers.find(w);
	if (it != inputReceivers.end()) {
		it->second.keyboardInput(key, scancode, action, mods);
	}
	else {
		LOGTO(Error) << "Invalid window input!" << BitEngine::endlog;
	}
}

void GLFW_InputDriver::GlfwMouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	GLFW_VideoDriver::Window_glfw *w = FindGLFWwindow(window);

	if (w == nullptr) {
		LOGTO(Warning) << "Unhandled mouse callback!" << BitEngine::endlog;
		return;
	}

	auto it = inputReceivers.find(w);
	if (it != inputReceivers.end()) {
		it->second.mouseInput(button, action, mods);
	}
	else {
		LOGTO(Error) << "Invalid window input!" << BitEngine::endlog;
	}
}

void GLFW_InputDriver::GlfwMousePosCallback(GLFWwindow* window, double x, double y) {
	GLFW_VideoDriver::Window_glfw *w = FindGLFWwindow(window);

	if (w == nullptr) {
		LOGTO(Warning) << "Unhandled mouse pos callback!" << BitEngine::endlog;
		return;
	}

	auto it = inputReceivers.find(w);
	if (it != inputReceivers.end()) {
		it->second.mouseInput(x, y);
	}
	else {
		LOGTO(Error) << "Invalid window input!" << BitEngine::endlog;
	}
}
