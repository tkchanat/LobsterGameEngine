///VertexShader
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texcoord;

out vec3 frag_texcoord;

void main()
{
    vec4 position = sys_projection * sys_view * sys_world * vec4(in_position, 1.0);
    frag_texcoord = in_position;
    gl_Position = position.xyww;
}  

///FragmentShader
in vec3 frag_texcoord;

out vec4 out_color;

uniform samplerCube skybox;

void main()
{    
    out_color = texture(skybox, frag_texcoord);
}