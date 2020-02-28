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
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
in vec3 frag_position;
in vec3 frag_normal;
in vec2 frag_texcoord;
in mat3 frag_TBN;

uniform sampler2D AlbedoMap;
uniform sampler2D NormalMap;

void main()
{    
    gPosition = frag_position;
    gNormal = TextureExists(NormalMap) ? normalize(frag_TBN * normalize(texture(NormalMap, frag_texcoord).rgb * 2.0 - 1.0)) : normalize(frag_normal);
    gAlbedo = texture(AlbedoMap, frag_texcoord);
}  