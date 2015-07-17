#pragma once

#include <map>

#include "System.h"

#include "ShaderProgram.h"
#include "DefaultShaders\Sprite2DShader.h"

namespace BitEngine
{
	class RenderSystem : public System
	{
		public:
			bool Init() override
			{

			}

			void Update() override
			{
				ClearScreen();

				Renderers();

				FlipBuffers();
			}

			void Shutdown() override
			{
				
			}

			void Renderers()
			{
				// Ordered by priority
				for (auto& r : m_renderers)
					r.second->Render();
			}

			void AddRenderer(int priority, Renderer* renderer);
			void RemoveRenderer(Renderer* renderer);

			void SetClearColor(float r, float g, float b, float a){
				clearColor[0] = r;
				clearColor[1] = g;
				clearColor[2] = b;
				clearColor[3] = a;

				glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
			}
		protected:
			void ClearScreen(){
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			}
			void FlipBuffers(){

			}

		private:
			float clearColor[4];

			std::map<int, Renderer*> m_renderers; //!< Priority ordered
	};
}