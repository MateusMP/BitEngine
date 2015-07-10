
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "Camera.h"

namespace BitEngine {

	class Camera2D
		: public Camera
	{
		public:
			Camera2D();

			// Camera position in world coordinates
			void setPosition(const glm::vec3& pos) override;
			glm::vec3 getPosition() const override;

			// Set camera focus position
			// This position will be at the center of the camera bounding view
			// To zoom on the center of the screen, use (screen_width/2, screen_height/2)
			void setLookAt(const glm::vec3& pos) override;
			glm::vec3 getLookAt() const override;

			glm::mat4 getMatrix() const override;

			// Usually the screen resolution
			void setView(int width, int height);

			void setZoom(float z);
			float getZoom() const;

			void Update() override;

		protected:
			int m_width;
			int m_height;

			glm::vec3 m_position;
			glm::vec3 m_lookAt;
			float m_zoom;
			
			glm::mat4 m_cameraMatrix;
			glm::mat4 m_orthoMatrix;

			bool changed;
	};


}
