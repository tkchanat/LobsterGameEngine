///VertexShader
layout (location = 0) in vec4 in_data; // <x, y, w, h>
out vec2 frag_texcoord;

void main()
{
    frag_texcoord = in_data.zw;
    gl_Position = sys_projection * sys_world * vec4(in_data.xy, 0.0, 1.0); 
}  

///FragmentShader
in vec2 frag_texcoord;
out vec4 out_color;

uniform sampler2D sys_spriteTexture;

void main()
{ 
    vec4 color = texture(sys_spriteTexture, frag_texcoord); 
    if(color.a == 0.0) discard;
    out_color = color;
}
