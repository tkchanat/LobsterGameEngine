#pragma once
#include "Texture.h"

namespace Lobster
{

	struct RenderTargetDesc
	{
		int width, height;
		Formats format;
		Types type;
	};

	class FrameBuffer
	{
	private:
		int m_width, m_height;
		uint m_FBO;
		uint m_RBO;
		uint* m_renderTargets;
		int m_renderTargetsCount;
		std::vector<RenderTargetDesc> m_renderTargetsDesc;
	public:
		FrameBuffer(int width, int height, const std::vector<RenderTargetDesc>& renderTargetsDesc = std::vector<RenderTargetDesc>());
		~FrameBuffer();
		void Bind();
		void Unbind();
		void Resize(int width, int height);
		inline void* Get(int index) { return (void*)(intptr_t)m_renderTargets[index]; }
		inline glm::ivec2 GetSize() const { return glm::ivec2(m_width, m_height); }
		inline float GetAspectRatio() const { return (float)m_width / (float)m_height; }
	private:
		void Construct();
	};

}
