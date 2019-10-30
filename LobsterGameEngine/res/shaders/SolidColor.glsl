///VertexShader
#version 410 core
layout (location = 0) in vec3 POSITION;
layout (location = 1) in vec3 NORMAL;
layout (location = 2) in vec2 TEXCOORD0;

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * world * vec4(POSITION, 1.0f);
}

///FragmentShader
#version 410 core
out vec4 FragColor;

layout (std140) uniform ubo_SolidColor
{
    vec4 color;
} SolidColor;

void main()
{
	FragColor = SolidColor.color;
} 