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

uniform mat4 sys_world;
uniform mat4 sys_view;
uniform mat4 sys_projection;

void main()
{
    // pass useful information into fragment shader
    frag_position = vec3(sys_world * vec4(in_position, 1.0));
    frag_normal = mat3(sys_world) * in_normal;
	frag_texcoord = in_texcoord;

    // calculate tangent space
    vec3 T = normalize(vec3(sys_world * vec4(in_tangent, 0.0)));
    vec3 B = normalize(vec3(sys_world * vec4(in_bitangent, 0.0)));
    vec3 N = normalize(vec3(sys_world * vec4(in_normal, 0.0)));
    frag_TBN = mat3(T, B, N);

    gl_Position = sys_projection * sys_view * sys_world * vec4(in_position, 1.0);
}

///FragmentShader
#version 410 core
in vec3 frag_position;
in vec3 frag_normal;
in vec2 frag_texcoord;
in mat3 frag_TBN;

out vec4 FragColor;

uniform vec3 sys_cameraPosition;
uniform vec3 sys_lightDirection;
uniform vec4 sys_lightColor;

uniform sampler2D DiffuseMap;
uniform sampler2D NormalMap;
uniform vec4 DiffuseColor;
uniform vec4 SpecularColor;
uniform float Shininess;

void main()
{
    bool UseDiffuseMap = textureSize(DiffuseMap, 0).x > 1;
    bool UseNormalMap = textureSize(NormalMap, 0).x > 1;

    // calculate normal in tangent space
    vec3 normal = UseNormalMap ? normalize(frag_TBN * normalize(texture(NormalMap, frag_texcoord).rgb * 2.0 - 1.0)) : normalize(frag_normal);

    // ambient
    vec4 ambient = vec4(vec3(0.15), 1.0);
  	
    // diffuse 
    float intensity = max(dot(normal, -sys_lightDirection), 0.0);
    vec4 diffuse = UseDiffuseMap ? texture(DiffuseMap, frag_texcoord) : DiffuseColor;

    // specular
    vec3 viewDirection = normalize(sys_cameraPosition - frag_position);
    vec3 reflectDirection = reflect(sys_lightDirection, normal);
    float multiplier = pow(max(dot(viewDirection, reflectDirection), 0.0), 16);
    vec4 specular = Shininess * multiplier * SpecularColor;

    vec4 result = (ambient + diffuse * intensity + specular) * sys_lightColor;
    FragColor = vec4(result.rgb, diffuse.a);
}