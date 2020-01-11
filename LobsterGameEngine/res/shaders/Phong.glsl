///VertexShader
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texcoord;
layout (location = 3) in vec3 in_tangent;
layout (location = 4) in vec3 in_bitangent;
layout (location = 5) in ivec4 in_boneId;
layout (location = 6) in vec4 in_boneWeight;

out vec3 frag_position;
out vec3 frag_normal;
out vec2 frag_texcoord;
out mat3 frag_TBN;

void main()
{
	// calculate bone influence
	mat4 BoneMatrix = mat4(1.0);
	if(sys_animate) {
		BoneMatrix = sys_bones[in_boneId[0]] * in_boneWeight[0];
		BoneMatrix += sys_bones[in_boneId[1]] * in_boneWeight[1];
		BoneMatrix += sys_bones[in_boneId[2]] * in_boneWeight[2];
		BoneMatrix += sys_bones[in_boneId[3]] * in_boneWeight[3];
	}

	// pass useful information into fragment shader
    frag_position = vec3(sys_world * BoneMatrix * vec4(in_position, 1.0));
    frag_normal = mat3(sys_world) * mat3(BoneMatrix) * in_normal;
	frag_texcoord = in_texcoord;

	// calculate tangent space
    vec3 T = normalize(vec3(sys_world * vec4(in_tangent, 0.0)));
    vec3 B = normalize(vec3(sys_world * vec4(in_bitangent, 0.0)));
    vec3 N = normalize(vec3(sys_world * vec4(in_normal, 0.0)));
    frag_TBN = mat3(T, B, N);

    gl_Position = sys_projection * sys_view * sys_world * BoneMatrix * vec4(in_position, 1.0);
}

///FragmentShader
in vec3 frag_position;
in vec3 frag_normal;
in vec2 frag_texcoord;
in mat3 frag_TBN;

out vec4 FragColor;

uniform float AmbientStrength = 0.25;
uniform sampler2D DiffuseMap;
uniform sampler2D NormalMap;
uniform vec4 DiffuseColor = vec4(0, 1, 0, 1);
uniform vec4 SpecularColor = vec4(1, 1, 1, 1);
uniform float Shininess = 0.5;

vec4 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir) 
{
    vec3 lightDir = normalize(light.direction);
    // diffuse shading
    float intensity = max(dot(normal, lightDir), AmbientStrength);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float multiplier = pow(max(dot(viewDir, reflectDir), 0.0), 16.0 * (Shininess + EPSILON));
    // combine results
    vec4 diffuse = TextureExists(DiffuseMap) ? texture(DiffuseMap, frag_texcoord) * DiffuseColor : DiffuseColor;
    vec4 specular = Shininess * multiplier * SpecularColor;
    vec3 color = (diffuse * intensity + specular).rgb;
    return vec4(color, diffuse.a);
}

void main()
{
    // calculate normal in tangent space
    vec3 normal = TextureExists(NormalMap) ? normalize(frag_TBN * normalize(texture(NormalMap, frag_texcoord).rgb * 2.0 - 1.0)) : normalize(frag_normal);
    vec3 viewDir = normalize(sys_cameraPosition - frag_position);

    vec4 result = vec4(0.0);
    for(int i = 0; i < Lights.directionalLightCount; ++i) {
        result += CalcDirectionalLight(Lights.directionalLights[i], normal, viewDir);
    }
    FragColor = vec4(result);
}