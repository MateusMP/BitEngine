#pragma once

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "Core/ECS/Component.h"

#include "Core/Mesh.h"

namespace BitEngine{

	class RenderableMeshComponent : public Component
	{
	public:

		RenderableMeshComponent()
			: mesh(nullptr), material(nullptr)
		{}

		RenderableMeshComponent(const Mesh* _mesh, Material* _material)
			: mesh(_mesh), material(_material)
		{}

		void setMesh(const Mesh* _mesh, Material* _mat = nullptr){
			if (_mat == nullptr)
				_mat = _mesh->getMaterial(); // Set default material

			mesh = _mesh;
			material = _mat;
		}

		const Mesh* getMesh() const {
			return mesh;
		}

		Material* getMaterial() const {
			return material;
		}

	private:

		const Mesh* mesh;
		Material* material; // Override mesh's default material

	};

}