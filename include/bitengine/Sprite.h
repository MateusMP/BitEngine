#pragma once

#include <glm/common.hpp>

#include "TypeDefinition.h"

#include <algorithm>

namespace BitEngine{

typedef unsigned short SpriteHandle;

class Sprite{

    public:
		Sprite()
			: m_textureID(0), m_width(16), m_height(16),
			m_offsetX(0), m_offsetY(0), m_uvrect(0.0f, 0.0f, 1.0f, 1.0f), m_transparent(false)
		{
			calculateMaxRadius();
		}

		Sprite(uint32 texture, int w, int h, float oX, float oY, const glm::vec4& _uvrect, bool _transparent=false)
			: m_textureID(texture), m_width(w), m_height(h), m_offsetX(oX), m_offsetY(oY), m_uvrect(_uvrect), m_transparent(_transparent)
        {
			calculateMaxRadius();
		}

		float getMaxRadius() const {
			return m_maxRadius;
		}

		uint32 getTexture() const {
			return m_textureID;
		}

		int getWidth() const {
			return m_width;
		}

		int getHeight() const {
			return m_height;
		}

		float getOffsetX() const {
			return m_offsetX;
		}

		float getOffsetY() const {
			return m_offsetY;
		}

		bool isTransparent() const {
			return m_transparent;
		}

		const glm::vec4& getUV() const {
			return m_uvrect;
		}

		const Sprite& operator =(const Sprite& spr) {
			m_textureID = spr.m_textureID;
			m_width = spr.m_width;
			m_height = spr.m_height;
			m_offsetX = spr.m_offsetX;
			m_offsetY = spr.m_offsetY;
			m_uvrect = spr.m_uvrect;
			m_transparent = spr.m_transparent;

			calculateMaxRadius();
			return *this;
		}

	private:
		void calculateMaxRadius()
		{
			float maxW = static_cast<float>(m_width);
			if (m_offsetX <= 0){
				maxW *= 1 - m_offsetX;
			}
			else {
				maxW *= std::max(1.0f - m_offsetX, m_offsetX);
			}

			float maxH = static_cast<float>(m_height);
			if (m_offsetY <= 0){
				maxH *= 1 - m_offsetY;
			}
			else {
				maxH *= std::max(1.0f - m_offsetY, m_offsetY);
			}

			float max = std::max(maxW, maxH);
			m_maxRadius = std::sqrt(maxW*maxW + maxH*maxH);
		}

        uint32 m_textureID;
		int m_width;
		int m_height;
		float m_offsetX;
		float m_offsetY;

        /**
            xw--zw (1,1)
            |    |
            xy--zy
        (0,0)
        */
        glm::vec4 m_uvrect;
		bool m_transparent;

		float m_maxRadius;
};

}
