#include "pch.h"
#include "FrameBuffer.h"
#include "graphics/Texture.h"

namespace Lobster
{

	FrameBuffer::FrameBuffer(int width, int height, const std::vector<RenderTargetDesc>& renderTargetsDesc) :
		m_width(width),
		m_height(height),
		m_renderTargets(nullptr),
		m_renderTargetsCount(renderTargetsDesc.size()),
		m_renderTargetsDesc(renderTargetsDesc)
	{
		// create frame and render buffers
		glGenFramebuffers(1, &m_FBO);
		glGenRenderbuffers(1, &m_RBO);
		// create a single render target by default
		if (m_renderTargetsDesc.empty()) {
			m_renderTargetsDesc.push_back({ width, height, Formats::RGBAFormat, Types::UnsignedByteType });
			m_renderTargetsCount = 1;
		}
		// create render targets according to RenderTargetDesc
		m_renderTargets = new uint[m_renderTargetsCount];
		for (int i = 0; i < m_renderTargetsCount; ++i) {
			uint targetId;
			glGenTextures(1, &targetId);
			glBindTexture(GL_TEXTURE_2D, targetId);
			glTexImage2D(GL_TEXTURE_2D, 0, (GLint)m_renderTargetsDesc[i].format, m_renderTargetsDesc[i].width, m_renderTargetsDesc[i].height, 0, (GLint)m_renderTargetsDesc[i].format, (GLint)m_renderTargetsDesc[i].type, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			m_renderTargets[i] = targetId;
		}
		Construct();
	}

	FrameBuffer::~FrameBuffer()
	{
		glDeleteFramebuffers(1, &m_FBO);
		glDeleteRenderbuffers(1, &m_RBO);
		glDeleteTextures(m_renderTargetsCount, m_renderTargets);
		if (m_renderTargets) delete[] m_renderTargets;
		m_renderTargets = nullptr;
	}

	void FrameBuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
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
		for (int i = 0; i < m_renderTargetsCount; ++i) {
			glBindTexture(GL_TEXTURE_2D, m_renderTargets[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, (GLint)m_renderTargetsDesc[i].format, width, height, 0, (GLint)m_renderTargetsDesc[i].format, (GLint)m_renderTargetsDesc[i].type, NULL);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		Construct();
	}

	void FrameBuffer::Construct()
	{
		Bind();

		// Configure render buffer object
		glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RBO);

		// Configure render targets
		for (int i = 0; i < m_renderTargetsCount; ++i) {
			glBindTexture(GL_TEXTURE_2D, m_renderTargets[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_renderTargets[i], 0);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		std::vector<uint> attachments(m_renderTargetsCount);
		std::iota(std::begin(attachments), std::end(attachments), GL_COLOR_ATTACHMENT0);
		glDrawBuffers(m_renderTargetsCount, attachments.data());

		// Check completeness
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			WARN("Framebuffer is not complete, you dumb ass!");
		}

		Unbind();
	}

}
