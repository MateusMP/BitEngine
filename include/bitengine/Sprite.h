#pragma once

#include <glm/common.hpp>

#include "TypeDefinition.h"

namespace BitEngine{

typedef unsigned short SpriteHandle;

class Sprite{

    public:
		Sprite()
			: textureID(0), width(16), height(16), 
			  offsetX(0), offsetY(0), uvrect(0.0f, 0.0f, 1.0f, 1.0f)
		{}

		Sprite(uint32 texture, int w, int h, float oX, float oY, const glm::vec4& _uvrect)
			: textureID(texture), width(w), height(h), offsetX(oX), offsetY(oY), uvrect(_uvrect)
        {}


        uint32 textureID;

		int width;
		int height;
		float offsetX;
		float offsetY;

        /**
            xw--zw (1,1)
            |    |
            xy--zy
        (0,0)
        */
        glm::vec4 uvrect;

};

}
