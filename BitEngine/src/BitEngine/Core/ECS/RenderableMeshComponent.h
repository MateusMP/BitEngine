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
                : material(nullptr)
            {}

            RenderableMeshComponent(RR<Model> _model, Material* _material=nullptr)
                : model(_model), material(_material)
            {}

            RR<Model> getModel() const {
                return model;
            }

            RR<Mesh> getMesh() const {
                return mesh;
            }

            Material* getMaterial() const {
                return material;
            }

        private:
            RR<Model> model;
            RR<Mesh> mesh;
            Material* material; // Override model's default material

	};

}
