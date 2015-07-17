#pragma once

#include "TypeDefinition.h"

namespace BitEngine{

class Sprite{

    public:
		Sprite()
			: uvrect(0,0,1,1), textureID(0)
		{}

        Sprite(const glm::vec4& _uvrect, uint32 texture)
            : uvrect(_uvrect), textureID(texture)
        {}


        /**
            xw--zw (1,1)
            |    |
            xy--zy
        (0,0)
        */
        glm::vec4 uvrect;

        uint32 textureID;
};

}
