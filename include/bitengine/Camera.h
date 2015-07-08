
#include <glm/common.hpp>

namespace BitEngine{

	class Camera{

		public:
			
			virtual void setPosition(const glm::vec3& pos) = 0;
			virtual glm::vec3 getPosition() const = 0;

			virtual void setLookAt(const glm::vec3& pos) = 0;
			virtual glm::vec3 getLookAt() const = 0;
		
			virtual glm::mat4 getMatrix() const = 0;

			virtual void Update() = 0;

		private:

	};



}