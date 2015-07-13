
#include "InputSystem.h"

#include "MessageChannel.h"
#include "EngineLoggers.h"

namespace BitEngine{

std::unordered_map<GLFWwindow*, InputReceiver> InputSystem::inputReceivers;


InputSystem::InputSystem()
    : System("Input")
{
    Channel::AddListener<WindowCreated>(this);
}

InputSystem::~InputSystem(){
}

bool InputSystem::Init()
{
	return true;
}

void InputSystem::Update()
{

}

void InputSystem::Shutdown()
{
	for (auto it : inputReceivers)
		glfwSetKeyCallback(it.first, nullptr);
}

void InputSystem::GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_UNKNOWN){
		LOGTO(Warning) << "Unknown key: scancode: " << scancode << " Action: " << action << " Mods: " << mods << endlog;

	}

	LOGTO(Verbose) << "Key Input on window " << window << " key: " << key << " scancode: " << scancode << " Action: " << action << " Mods: " << mods << endlog;

	auto it = inputReceivers.find(window);
	if (it != inputReceivers.end()){
		it->second.keyboardInput(key, scancode, action, mods);
	} else {
		LOGTO(Error) << "Invalid window input!" << endlog;
	}
}

void InputSystem::Message(const WindowCreated& wndcr)
{
    glfwSetKeyCallback(wndcr.window, GlfwKeyCallback);
    printf("MENSAGEM RECEBIDA: WindowCreated\n");

	// Creates instance for this window
	inputReceivers[wndcr.window];
}

InputReceiver::KeyMod InputSystem::isKeyPressed(int key)
{
	auto w = inputReceivers.begin();
	if (w == inputReceivers.end())
		return InputReceiver::KeyMod::FALSE;

	return w->second.isKeyPressed(key);
}

InputReceiver::KeyMod InputSystem::keyReleased(int key)
{
	auto w = inputReceivers.begin();
	if (w == inputReceivers.end())
		return InputReceiver::KeyMod::FALSE;

	return w->second.keyReleased(key);
}


void InputReceiver::keyboardInput(int key, int scancode, int action, int mods)
{
	switch (action){

		case GLFW_REPEAT:
			m_keyDown[key] = (KeyMod)(((unsigned char)KeyMod::TRUE) | (unsigned char)mods);
			m_keyReleased[key] = KeyMod::FALSE;

			LOGTO(Verbose) << "Key repeat" << endlog;
		break;

		case GLFW_PRESS:
			m_keyDown[key] = (KeyMod)(((unsigned char)KeyMod::TRUE) | (unsigned char)mods);
			m_keyReleased[key] = KeyMod::FALSE;

			LOGTO(Verbose) << "Key down" << endlog;
		break;

		case GLFW_RELEASE:
			m_keyReleased[key] = (KeyMod)(((unsigned char)KeyMod::TRUE) | (unsigned char)mods);
			m_keyDown[key] = KeyMod::FALSE;

			LOGTO(Verbose) << "Key Release" << endlog;
		break;

		default:
			LOGTO(Warning) << "Invalid key action: " << action << endlog;
		break;
	}

	LOGTO(Verbose) << "State down: " << (unsigned int)m_keyDown[key] << endlog;
	LOGTO(Verbose) << "State up: " << (unsigned int)m_keyReleased[key] << endlog;
}

InputReceiver::KeyMod InputReceiver::isKeyPressed(int key)
{
	auto k = m_keyDown.find(key);
	if (k != m_keyDown.end())
		return k->second;

	return KeyMod::FALSE;
}

InputReceiver::KeyMod InputReceiver::keyReleased(int key)
{
	auto k = m_keyReleased.find(key);
	if (k != m_keyReleased.end())
		return k->second;

	return KeyMod::FALSE;
}

}
