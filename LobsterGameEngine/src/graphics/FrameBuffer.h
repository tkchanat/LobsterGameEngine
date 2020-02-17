#pragma once
#include "graphics/Texture.h"

namespace Lobster
{

	enum ClearFlag : uint
	{
		NONE	= 0,
		COLOR   = 1<<0,
		DEPTH   = 1<<1,
		STENCIL = 1<<2
	};

	struct RenderTargetDesc
	{
		bool depthOnly = false;
		Formats format = Formats::RGBAFormat;
		Types type = Types::UnsignedByteType;
		MinMagFilters minFilter = MinMagFilters::LinearFilter;
		MinMagFilters magFilter = MinMagFilters::LinearFilter;
		WrappingModes wrappingS = WrappingModes::ClampToEdgeWrapping;
		WrappingModes wrappingT = WrappingModes::ClampToEdgeWrapping;
	};

	class FrameBuffer
	{
	private:
		int m_width, m_height;
		uint m_FBO;
		uint m_RBO;
		uint* m_renderTargets;
		std::vector<RenderTargetDesc> m_renderTargetsDesc;
	public:
		FrameBuffer(int width, int height, const std::vector<RenderTargetDesc>& renderTargetsDesc = std::vector<RenderTargetDesc>());
		~FrameBuffer();
		void Bind();
		void BindAndClear(uint flag);
		void Unbind();
		void Resize(int width, int height);
		inline void* Get(int index) { return (void*)(intptr_t)m_renderTargets[index]; }
		inline glm::ivec2 GetSize() const { return glm::ivec2(m_width, m_height); }
		inline float GetAspectRatio() const { return (float)m_width / (float)m_height; }
	private:
		void AssembleFrameBuffer();
	};

}
