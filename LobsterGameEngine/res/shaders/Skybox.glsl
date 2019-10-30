///VertexShader
#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTex;

out vec3 TexCoords;

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 position = projection * view * world * vec4(aPos, 1.0);
    TexCoords = aPos;
    gl_Position = position.xyww;
}  

///FragmentShader
#version 410 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{    
    FragColor = texture(skybox, TexCoords);
}