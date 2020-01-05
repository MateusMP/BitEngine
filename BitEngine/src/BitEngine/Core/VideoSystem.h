#pragma once

#include <set>

#include "BitEngine/Core/Window.h"
#include "BitEngine/Core/Messenger.h"

#include "BitEngine/Core/Graphics/VideoRenderer.h"

namespace BitEngine {

class Material;

class VideoConfiguration
{
};

/** Default class for Video configuration
 * Basic video initialization
 * Uses just one window
 */
class VideoSystem
{
public:
    VideoSystem()
    {
        //getConfig("Fullscreen", "false")->setDescription("Use fullscreen mode, true, false ");
    }

    virtual ~VideoSystem() {}

    virtual bool init() = 0;
    virtual void update() = 0;
    virtual void shutdown() = 0;

    virtual VideoAdapterType getVideoAdapter() = 0;

    virtual Window* createWindow(const BitEngine::WindowConfiguration& wc) = 0;

    virtual void closeWindow(Window* window) = 0;

};

}


