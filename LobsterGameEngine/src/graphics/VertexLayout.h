#pragma once

namespace Lobster {
    
	//	This struct defines what a single element looks like.
	//	Most likely, you don't have to deal with it.
    struct LayoutElement
    {
        LayoutElement(const char* n, uint s, uint t, bool m, uint c, uint o) :
            name(n),
            typeSize(s),
            type(t),
            normalized(m),
            count(c),
            offset(o)
        {
        }
        ~LayoutElement() = default;
        std::string name;       //  Just for visual recognition
        uint typeSize;          //  Size of type (e.g. float, uint, etc.)
        uint type;              //  Type of element in GLenum
        bool normalized;        //  Normalized?
        uint count;             //  Number of element in vertex (e.g. float2, float3, etc.)
        uint offset;            //  Where the data begin in one stride (in byte)
    };
    
	//	This is the class you are looking for.
	//	This class defines the layout of a vertex buffer, equivalent with OpenGL's vertex attribute object and DirectX's input layout.
	//	The defined layout should match the layout defined in the shader code (https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)).
    class VertexLayout
    {
    private:
        uint m_stride;
        std::vector<LayoutElement> m_layout;
    public:
        VertexLayout();
        ~VertexLayout();
        void Bind() const;
        void Unbind() const;
        
        template<typename T>
		void Add(const char* name, uint count);
	};
    
}
