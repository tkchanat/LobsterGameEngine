///VertexShader
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texcoord;
layout (location = 3) in vec3 in_tangent;
layout (location = 4) in vec3 in_bitangent;

out vec3 frag_position;
out vec3 frag_normal;
out vec2 frag_texcoord;
out mat3 frag_TBN;

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
in vec3 frag_position;
in vec3 frag_normal;
in vec2 frag_texcoord;
in mat3 frag_TBN;

out vec4 FragColor;

uniform float AmbientStrength = 0.45;
uniform sampler2D DiffuseMap;
uniform sampler2D NormalMap;
uniform vec4 DiffuseColor = vec4(0, 1, 0, 1);
uniform vec4 SpecularColor = vec4(1, 1, 1, 1);
uniform float Shininess = 0.5;

vec4 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir) 
{
    vec3 lightDir = normalize(light.direction);
    // diffuse shading
    float intensity = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float multiplier = pow(max(dot(viewDir, reflectDir), 0.0), Shininess + EPSILON);
    // combine results
    vec4 ambient = vec4(vec3(AmbientStrength), 1.0);
    vec4 diffuse = TextureExists(DiffuseMap) ? texture(DiffuseMap, frag_texcoord) * DiffuseColor : DiffuseColor;
    vec4 specular = Shininess * multiplier * SpecularColor;
    return vec4((ambient + diffuse * intensity + specular).rgb, diffuse.a);
}

void main()
{
    // calculate normal in tangent space
    vec3 normal = TextureExists(NormalMap) ? normalize(frag_TBN * normalize(texture(NormalMap, frag_texcoord).rgb * 2.0 - 1.0)) : normalize(frag_normal);
    vec3 viewDir = normalize(sys_cameraPosition - frag_position);

    vec4 result = vec4(0.0);
    for(int i = 0; i < Lights.directionalLightCount; ++i) {
        // if(i >= Lights.directionalLightCount) break;
        result += CalcDirectionalLight(Lights.directionalLights[i], normal, viewDir);
    }
    FragColor = vec4(result);
}