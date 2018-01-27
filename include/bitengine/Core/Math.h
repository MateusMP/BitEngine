#pragma once

#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace BitEngine {

	// Math data types
	typedef glm::fvec2 Vec2;
	typedef glm::fvec3 Vec3;
	typedef glm::fvec4 Vec4;

	typedef glm::mat3 Mat3;
	typedef glm::mat4 Mat4;


	namespace Math {

		template<typename T, typename V>
		static T scale(T&& a, V&& b) {
			return glm::scale(a, b);
		}

		template<typename T, typename V>
		static T translate(T&& a, V&& b) {
			return glm::translate(a, b);
		}
	}

}
