///VertexShader
layout (location = 0) in vec3 POSITION;
layout (location = 1) in vec3 NORMAL;
layout (location = 2) in vec2 TEXCOORD0;

void main()
{
    gl_Position = sys_projection * sys_view * sys_world * vec4(POSITION, 1.0f);
}

///FragmentShader
out vec4 FragColor;

uniform vec4 color;

void main()
{
	FragColor = color;
} 