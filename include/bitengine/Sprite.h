#pragma once

#include <glm/common.hpp>

#include "TypeDefinition.h"

#include <algorithm>

namespace BitEngine{

typedef unsigned short SpriteHandle;

class Sprite{

    public:
		Sprite()
			: textureID(0), width(16), height(16), 
				offsetX(0), offsetY(0), uvrect(0.0f, 0.0f, 1.0f, 1.0f), transparent(false)
		{
			calculateMaxRadius();
		}

		Sprite(uint32 texture, int w, int h, float oX, float oY, const glm::vec4& _uvrect, bool _transparent=false)
			: textureID(texture), width(w), height(h), offsetX(oX), offsetY(oY), uvrect(_uvrect), transparent(_transparent)
        {
			calculateMaxRadius();
		}

		float getMaxRadius() const {
			return maxRadius;
		}

		uint32 getTexture() const {
			return textureID;
		}

		int getWidth() const {
			return width;
		}

		int getHeight() const {
			return height;
		}

		float getOffsetX() const {
			return offsetX;
		}

		float getOffsetY() const {
			return offsetY;
		}

		bool isTransparent() const {
			return transparent;
		}

		const glm::vec4& getUV() const {
			return uvrect;
		}

	private:
		void calculateMaxRadius()
		{
			float maxW = static_cast<float>(width);
			if (offsetX <= 0){
				maxW *= 1 - offsetX;
			}
			else {
				maxW *= std::max(1.0f - offsetX, offsetX);
			}

			float maxH = static_cast<float>(height);
			if (offsetY <= 0){
				maxH *= 1 - offsetY;
			}
			else {
				maxH *= std::max(1.0f - offsetY, offsetY);
			}

			float max = std::max(maxW, maxH);
			maxRadius = std::sqrt(maxW*maxW + maxH*maxH);
		}

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
		bool transparent;

		float maxRadius;
};

}
