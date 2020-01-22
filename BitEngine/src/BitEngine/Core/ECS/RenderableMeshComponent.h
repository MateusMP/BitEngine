#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "BitEngine/Core/ECS/Component.h"

#include "BitEngine/Core/Mesh.h"

namespace BitEngine{

	class RenderableMeshComponent : public Component<RenderableMeshComponent>
	{
        public:
            RenderableMeshComponent()
                : mesh(nullptr), material(nullptr)
            {}

            RenderableMeshComponent(Mesh* _mesh, Material* _material)
                : mesh(_mesh), material(_material)
            {}

            void setMesh(Mesh* _mesh, Material* _mat = nullptr){
                if (_mat == nullptr)
                    _mat = _mesh->getMaterial(); // Set default material

                mesh = _mesh;
                material = _mat;
            }

            Mesh* getMesh() const {
                return mesh;
            }

            Material* getMaterial() const {
                return material;
            }

        private:

            Mesh* mesh;
            Material* material; // Override mesh's default material

	};

}
