///VertexShader
layout (location = 0) in vec3 in_position;
layout (location = 2) in vec2 in_texcoord;

out vec2 frag_texcoord;

void main()
{
    frag_texcoord = in_texcoord;
    gl_Position = vec4(in_position, 1.0); 
}  

///FragmentShader
in vec2 frag_texcoord;
out vec4 out_color;

uniform sampler2D screenTexture;

void main()
{ 
    out_color = texture(screenTexture, frag_texcoord);
}
