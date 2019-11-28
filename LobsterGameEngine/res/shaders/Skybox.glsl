///VertexShader
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTex;

out vec3 TexCoords;

void main()
{
    vec4 position = sys_projection * sys_view * sys_world * vec4(aPos, 1.0);
    TexCoords = aPos;
    gl_Position = position.xyww;
}  

///FragmentShader
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{    
    FragColor = texture(skybox, TexCoords);
}