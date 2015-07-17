#pragma once

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "Component.h"
#include "EntitySystem.h"

namespace BitEngine {

	class Camera2DComponent
		: public Component
	{
		public:
			Camera2DComponent();

			// Camera position in world coordinates
			void setPosition(const glm::vec3& pos);
			glm::vec3 getPosition() const;

			// Set camera focus position
			// This position will be at the center of the camera bounding view
			// To zoom on the center of the screen, use (screen_width/2, screen_height/2)
			void setLookAt(const glm::vec3& pos);
			glm::vec3 getLookAt() const;

			const glm::mat4& getMatrix() const;

			// Usually the screen resolution
			void setView(int width, int height);

			void setZoom(float z);
			float getZoom() const;

			static ComponentType getComponentType(){
				return COMPONENT_TYPE_CAMERA2D;
			}

			void setActive(bool a);
			
		protected:
			int m_width;
			int m_height;

			glm::vec3 m_position;
			glm::vec3 m_lookAt;
			float m_zoom;
			
			glm::mat4 m_cameraMatrix;
			glm::mat4 m_orthoMatrix;

			bool changed;

			bool m_active;

		private:
			friend class Camera2DProcessor;
			void recalculateMatrix();
	};


}
