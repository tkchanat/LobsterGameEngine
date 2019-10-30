///VertexShader
#version 410 core
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texcoord;
layout (location = 3) in vec3 in_tangent;
layout (location = 4) in vec3 in_bitangent;

out vec3 frag_position;
out vec3 frag_normal;
out vec2 frag_texcoord;
out mat3 frag_TBN;

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // pass useful information into fragment shader
    frag_position = vec3(world * vec4(in_position, 1.0));
    frag_normal = mat3(world) * in_normal;
	frag_texcoord = in_texcoord;

    // calculate tangent space
    vec3 T = normalize(vec3(world * vec4(in_tangent, 0.0)));
    vec3 B = normalize(vec3(world * vec4(in_bitangent, 0.0)));
    vec3 N = normalize(vec3(world * vec4(in_normal, 0.0)));
    frag_TBN = mat3(T, B, N);

    gl_Position = projection * view * world * vec4(in_position, 1.0);
}

///FragmentShader
#version 410 core
in vec3 frag_position;
in vec3 frag_normal;
in vec2 frag_texcoord;
in mat3 frag_TBN;

out vec4 FragColor;

layout (std140) uniform ubo_Phong
{
    bool UseDiffuseMap;   bool UseNormalMap;
    vec2 padding0;
    vec3 DiffuseColor;    float padding1;
    vec3 SpecularColor;   float Shininess;
} Phong;

uniform vec3 cameraPosition;
uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform sampler2D DiffuseMap;
uniform sampler2D NormalMap;

void main()
{
    // calculate normal in tangent space
    vec3 normal = normalize(frag_normal);
    if(Phong.UseNormalMap)
    {
        normal = texture(NormalMap, frag_texcoord).rgb;
        normal = normalize(normal * 2.0 - 1.0);
        normal = normalize(frag_TBN * normal);
    }

    // ambient
    vec3 ambient = 0.15 * lightColor;
  	
    // diffuse 
    float intensity = max(dot(normal, -lightDirection), 0.0);
    vec3 diffuse = intensity * lightColor * Phong.DiffuseColor;

    // specular
    vec3 viewDirection = normalize(cameraPosition - frag_position);
    vec3 reflectDirection = reflect(lightDirection, normal);
    float multiplier = pow(max(dot(viewDirection, reflectDirection), 0.0), 16);
    vec3 specular = Phong.Shininess * multiplier * lightColor * Phong.SpecularColor;

    vec3 result = ambient + diffuse + specular;
    if(Phong.UseDiffuseMap)
        FragColor = texture(DiffuseMap, frag_texcoord) * vec4(result, 1.0);
    else
        FragColor = vec4(result, 1.0);
} 