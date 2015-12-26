
#include "VideoSystem.h"

#include "EngineLoggers.h"

#include <stdio.h>

#include "GLFW_VideoSystem.h"

std::map<Window*, VideoSystem*> GLFW_VideoSystem::resizeCallbackReceivers;
std::set<Window*> GLFW_VideoSystem::windowsOpen;
