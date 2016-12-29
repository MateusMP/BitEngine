#pragma once

#include <string>
#include "bitengine/Common/TypeDefinition.h"

namespace BitEngine {
	
	enum VideoAdapterType{
		NONE,
		OPENGL_2 = 1 << 1,
		OPENGL_4 = 1 << 2,

		VULKAN_1 = 1 << 3,

		GL_ANY = OPENGL_2 | OPENGL_4,
		GL_2_OR_LOWER = OPENGL_2,
		GL_2_OR_GREATER = OPENGL_2 | OPENGL_4,

		VULKAN_ANY = VULKAN_1,
	};

	enum DataType
	{
		INVALID_DATA_TYPE,
		TEXTURE_1D,
		TEXTURE_2D,
		TEXTURE_3D,
		TEXTURE_CUBE,

		LONG,
		FLOAT,

		// Vertex only
		VEC2,
		VEC3,
		VEC4,

		MAT2,
		MAT3,
		MAT4,

		TOTAL_DATA_TYPES
	};

	enum DataUseMode : u32 {
		Vertex,
		Uniform,

		TotalModes
	};

	enum BufferClearBitMask
	{
		COLOR = 1,
		DEPTH = 2,

		COLOR_DEPTH = COLOR | DEPTH
	};


	enum BlendEquation {
		ADD,
		SUBTRACT,
		SUBTRACT_REVERSE,

		TOTAL_BLEND_EQUATIONS,
	};

	enum class VertexRenderMode {
		TRIANGLES,
		TRIANGLE_STRIP,
	};

	enum BlendFunc {
		ZERO,
		ONE,
		SOURCE_COLOR,
		ONE_MINUS_SOURCE_COLOR,
		DST_COLOR,
		ONE_MINUS_DST_COLOR,
		SRC_ALPHA,
		ONE_MINUS_SRC_ALPHA,
		DST_ALPHA,
		ONE_MINUS_DST_ALPHA,
		CONST_COLOR,
		ONE_MINUS_CONST_COLOR,
		CONST_ALPHA,
		ONE_MINUS_CONST_ALPHA,
		SRC_ALPHA_SATURATE,

		TOTAL_BLEND_MODES,
	};

	enum class RenderConfig : u8 {
		BLEND,
		ALPHA_TEST,
		CULL_FACE,
		DEPTH_TEST,
		
		MULTISAMPLE,

		TEXTURE_1D,
		TEXTURE_2D,
		TEXTURE_3D,
		TEXTURE_CUBE,

		TOTAL_RENDER_CONFIGS,
	};

	enum BlendConfig {
		BLEND_NONE,
		BLEND_ALL, // color and alpha 
		BLEND_SEPARATE, // color separated from alpha
	};

	enum CullFaceConfig {
		CULL_FACE_NONE,
		BACK_FACE,
		FRONT_FACE,
		FRONT_AND_BACK,
	};

	enum DepthConfig {
		DEPTH_TEST_DISABLED			= 1 << 0,
		DEPTH_TEST_ENABLED			= 1 << 1,
		DEPTH_TEST_WRITE_ENABLED	= 1 << 2,

		DEPTH_WITHOUT_WRITE = DEPTH_TEST_ENABLED,
		DEPTH_WITH_WRITE = DEPTH_TEST_ENABLED & DEPTH_TEST_WRITE_ENABLED,
	};
	
	template<unsigned channels, unsigned R, unsigned G, unsigned B, unsigned A>
	struct Color
	{
		static_assert(R != G && G != B && B != A, "Invalid color index");

		public:
			enum Channel { RED = R, GREEN = G, BLUE = B, ALPHA = A, CHANNELS = channels };
			float& operator [] (Channel x) {
				return colors[x];
			}
			inline float r() const { return colors[R]; }
			inline float g() const { return colors[G]; }
			inline float b() const { return colors[B]; }
			inline float a() const { return colors[A]; }

		private:
			float colors[channels];
	};
	typedef Color<4, 0, 1, 2, 3> ColorRGBA;
	typedef Color<3, 0, 1, 2, 3> ColorRGB;

	class RenderBuffer
	{
		virtual ~RenderBuffer() {}
		virtual void unbind() = 0;
		virtual void bind() = 0;
		virtual void bindDraw() = 0;
		virtual void bindRead() = 0;
		virtual bool ready() = 0;
	};


	DataUseMode useModeFromString(const std::string& str);
	DataType dataTypeFromString(const std::string& str);
}
