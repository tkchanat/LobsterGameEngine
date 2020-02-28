#include "pch.h"
#include "FrameBuffer.h"
#include "graphics/Texture.h"

namespace Lobster
{

	FrameBuffer::FrameBuffer(int width, int height, const std::vector<RenderTargetDesc>& renderTargetsDesc) :
		m_width(width),
		m_height(height),
		m_renderTargets(nullptr),
		m_renderTargetsDesc(renderTargetsDesc)
	{
		// create frame and render buffers
		glGenFramebuffers(1, &m_FBO);
		glGenRenderbuffers(1, &m_RBO);
		// create a single render target by default
		if (m_renderTargetsDesc.empty()) {
			RenderTargetDesc desc;
			m_renderTargetsDesc.push_back(desc);
		}
		// create render targets according to RenderTargetDesc
		m_renderTargets = new uint[m_renderTargetsDesc.size()];
		for (int i = 0; i < m_renderTargetsDesc.size(); ++i) {
			uint targetId;
			RenderTargetDesc& desc = m_renderTargetsDesc[i];
			glGenTextures(1, &targetId);
			m_renderTargets[i] = targetId;
		}
		AssembleFrameBuffer();
	}

	FrameBuffer::~FrameBuffer()
	{
		glDeleteFramebuffers(1, &m_FBO);
		glDeleteRenderbuffers(1, &m_RBO);
		glDeleteTextures(m_renderTargetsDesc.size(), m_renderTargets);
		if (m_renderTargets) delete[] m_renderTargets;
		m_renderTargets = nullptr;
	}

	void FrameBuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	}

	void FrameBuffer::BindRead()
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);
	}

	void FrameBuffer::BindDraw()
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
	}

	void FrameBuffer::BindAndClear(uint flag)
	{
		// Bind
		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
		// Clear
		GLenum glFlag = 0;
		glFlag |= (flag & ClearFlag::COLOR) ? GL_COLOR_BUFFER_BIT : 0;
		glFlag |= (flag & ClearFlag::DEPTH) ? GL_DEPTH_BUFFER_BIT : 0;
		glFlag |= (flag & ClearFlag::STENCIL) ? GL_STENCIL_BUFFER_BIT : 0;
		glClearColor(0.25, 0.25, 0.25, 1);
		glClear(glFlag);
	}

	void FrameBuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void FrameBuffer::Resize(int width, int height)
	{
		m_width = width;
		m_height = height;
		glViewport(0, 0, width, height);
		for (int i = 0; i < m_renderTargetsDesc.size(); ++i) {
			RenderTargetDesc& desc = m_renderTargetsDesc[i];
			glBindTexture(GL_TEXTURE_2D, m_renderTargets[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, (GLint)desc.format, m_width, m_height, 0, (GLint)desc.format, (GLint)desc.type, NULL);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		AssembleFrameBuffer();
	}

	void FrameBuffer::AssembleFrameBuffer()
	{
		Bind();

		bool depthOnly = std::any_of(m_renderTargetsDesc.begin(), m_renderTargetsDesc.end(), [](const RenderTargetDesc& desc) { return desc.depthOnly; });

		// Configure render targets
		for (int i = 0; i < m_renderTargetsDesc.size(); ++i) {
			RenderTargetDesc& desc = m_renderTargetsDesc[i];
			glBindTexture(GL_TEXTURE_2D, m_renderTargets[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, (GLint)desc.internalFormat, m_width, m_height, 0, (GLint)desc.format, (GLint)desc.type, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)desc.minFilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)desc.magFilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint)desc.wrappingS);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint)desc.wrappingT);
			if (depthOnly) {
				float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
				glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_renderTargets[i], 0);
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
			}
			else {
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_renderTargets[i], 0);
			}
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		// Multiple render targets (if applicable)
		std::vector<uint> attachments(m_renderTargetsDesc.size());
		std::iota(std::begin(attachments), std::end(attachments), GL_COLOR_ATTACHMENT0);
		glDrawBuffers(m_renderTargetsDesc.size(), attachments.data());

		// Configure render buffer object
		if (!depthOnly) {
			glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RBO);
		}

		// Check completeness
		int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			LOG("Framebuffer is not complete, you dumb ass!");
		}

		Unbind();
	}

}
