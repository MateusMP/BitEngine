#pragma once

#include <vector>
#include "GLFW_Headers.h"
#include "bitengine/Core/VideoSystem.h"

namespace BitEngine
{

class GLFW_Window : public MessengerEndpoint, public Window
{
public:
	GLFW_Window(Messenger *m, GLFWwindow *w)
		: MessengerEndpoint(m), window(w)
	{
	}
	virtual ~GLFW_Window();

	virtual void drawBegin() override;
	virtual void drawEnd() override;

private:
    friend class GLFW_ImGuiSystem;
	friend class GLFW_VideoSystem;
	friend class GLFW_InputSystem;
	GLFWwindow *window;
};


class GLFW_VideoSystem : public VideoSystem
{
public:
	GLFW_VideoSystem(Messenger *m)
		: VideoSystem(m), glewStarted(false)
	{
	}
	~GLFW_VideoSystem() {}

	/**
		* Initializes a window and openGL related stuff (Extensions and functions)
		* Currently using GLFW and GLEW
		*/
	bool init() override;
	void update() override;
	void shutdown() override;

	virtual u32 getVideoAdapter() override
	{
		return m_driver.getVideoAdapter();
	}
	virtual VideoDriver *getDriver() override
	{
		return &m_driver;
	}

	virtual Window *createWindow(const WindowConfiguration &wc) override;

	virtual void closeWindow(Window *window) override;

protected:
	/** Helper function
		* Swap buffers drawing new screen
		*/
	bool checkWindowClosed(GLFWwindow *window);

private:
	bool glewStarted;
	VideoDriver m_driver;
	std::vector<GLFW_Window *> m_windows;

	GLFW_Window *createGLFWWindow(const WindowConfiguration &wndConf);

	static void GLFW_ErrorCallback(int error, const char *description);

	LOG_CLASS(GLFW_VideoSystem, EngineLog);
};

} // namespace BitEngine