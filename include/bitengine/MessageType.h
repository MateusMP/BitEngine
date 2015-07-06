#pragma once


#include "Graphics.h"

namespace BitEngine{

struct WindowClose
{
};

struct WindowCreated
{
    WindowCreated(GLFWwindow *w): window(w){
    }
    GLFWwindow* window;
};


}

