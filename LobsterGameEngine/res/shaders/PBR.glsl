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
out vec4 out_color;

uniform sampler2D AlbedoMap;
uniform sampler2D NormalMap;
uniform sampler2D RoughnessMap;
uniform sampler2D MetallicMap;
uniform sampler2D AmbientOcclusionMap;

uniform vec3 Albedo;
uniform float Metallic;
uniform float Roughness;
uniform float AmbientOcclusion;

float Distribution(float cosLh, float roughness)
{
	float a = roughness * roughness;
    float a2 = a * a;
    float nh2 = cosLh * cosLh;
    float denom = (nh2 * (a2 - 1.0) + 1.0);
	
    return a2 / max(PI * denom * denom, 0.001);
}

float SchlickGGX(float cosTheta, float k)
{
    return cosTheta / (cosTheta * (1.0 - k) + k);
}
  
float Geometry(float cosLi, float nv, float roughness)
{
    float r = roughness + 1.0;
	float k = (r * r) / 8.0;
    return SchlickGGX(cosLi, k) * SchlickGGX(nv, k);
}

vec3 Fresnel(float cosTheta, vec3 f0)
{
	return f0 + (1.0 - f0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
	// texture maps
	vec3 albedo = TextureExists(AlbedoMap) ? texture(AlbedoMap, frag_texcoord).rgb : Albedo;
	vec3 normal = TextureExists(NormalMap) ? normalize(frag_TBN * normalize(texture(NormalMap, frag_texcoord).rgb * 2.0 - 1.0)) : normalize(frag_normal);
	float metallic = TextureExists(MetallicMap) ? texture(MetallicMap, frag_texcoord).r : Metallic;

	// choose initial reflectance value at normal incidence
	vec3 f0 = mix(vec3(0.04), albedo, metallic);

	// reflectance equation
	vec3 Lo = vec3(0.0);
	for(int i = 0; i < MAX_DIRECTIONAL_LIGHTS; ++i)
	{
		vec3 n = normal;
		vec3 v = normalize(sys_cameraPosition - frag_position);
		vec3 l = normalize(Lights.directionalLights[i].direction);
		vec3 h = normalize(v + l);
		float cosLi = max(dot(n, l), 0.0);
		float cosLh = max(dot(n, h), 0.0);
		float nv = max(dot(n, v), 0.0);

		// Radiance
		float distance = length(Lights.directionalLights[i].direction - frag_position);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = vec3(Lights.directionalLights[i].color);// * attenuation;

		// BRDF
		vec3 F = Fresnel(clamp(dot(h, v), 0.0, 1.0), f0);
		float D = Distribution(cosLh, Roughness);
		float G = Geometry(cosLi, nv, Roughness);
		vec3 specular = (F * D * G) / max(4.0 * nv * cosLi, 0.001);

		// Energy conservation
		vec3 kS = F;
		vec3 kD = (1.0 - F) * (1.0 - metallic);
		vec3 diffuse = kD * albedo;
		
		Lo += (diffuse + specular) * radiance * cosLi;
	}

	out_color = vec4(Lo, 1.0);
} 