///VertexShader
#version 410 core
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texcoord;

out vec2 frag_texcoord;

void main()
{
    gl_Position = vec4(in_position.x, in_position.y, 0.0, 1.0); 
    frag_texcoord = in_texcoord;
}  

///FragmentShader
#version 410 core
in vec2 frag_texcoord;
out vec4 out_FinalColor;

uniform sampler2D screenTexture;

void main()
{ 
    out_FinalColor = texture(screenTexture, frag_texcoord);
}
