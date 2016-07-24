#pragma once

#include <vector>

#include "OpenGLheaders.h"

#include "Common/TypeDefinition.h"
#include "Core/IVideoRenderer.h"

class FrameBuffer : public BitEngine::IRenderBuffer
{
public:
	struct RenderBuffer{
		RenderBuffer(GLuint a) : rbo(a) {}
		GLuint rbo;
	};
	struct TextureBuffer {
		TextureBuffer(GLuint a) : tbo(a){}
		GLuint tbo;
	};

	FrameBuffer()
		: fbo(0)
	{
	}

	~FrameBuffer()
	{
		if (fbo != 0) 
		{
			glDeleteFramebuffers(1, &fbo);
		}
	}

	void CreateFrameBuffer()
	{
		glGenFramebuffers(1, &fbo);
	}

	void Unbind() override
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Bind() override
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	}

	void BindDraw() override
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	}

	void BindRead() override
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	}

	bool Ready() override
	{
		return fbo != 0 && glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	}

	u32 AttachTexture2D(int width, int height, int format = GL_RGB, int attachMode = GL_COLOR_ATTACHMENT0)
	{
		GLuint tbo = 0;

		Bind();

		glGenTextures(1, &tbo);

		glBindTexture(GL_TEXTURE_2D, tbo);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, attachMode, GL_TEXTURE_2D, tbo, 0);

		texture.emplace_back(tbo);

		Unbind();

		return tbo;
	}

	u32 AttachRenderBuffer(int width, int height, int format = GL_DEPTH24_STENCIL8, int attachMode = GL_DEPTH_STENCIL_ATTACHMENT)
	{
		GLuint rbo = 0;
		Bind();

		glGenRenderbuffers(1, &rbo);

		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, format, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachMode, GL_RENDERBUFFER, rbo);

		render.emplace_back(rbo);

		Unbind();

		return rbo;
	}

private:
	GLuint fbo;

	std::vector<TextureBuffer> texture;
	std::vector<RenderBuffer> render;
};
