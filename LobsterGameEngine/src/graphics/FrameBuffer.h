#pragma once

namespace Lobster
{

	class FrameBuffer
	{
	private:
		int m_width, m_height;
		uint m_FBO;
		uint m_RBO;
		uint m_renderTarget;
	public:
		FrameBuffer(int width, int height);
		~FrameBuffer();
		void Bind();
		void Unbind();
		void Resize(int width, int height);
		inline void* Get() { return (void*)(intptr_t)m_renderTarget; }
	private:
		void Construct();
	};

}
