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

uniform vec3 Albedo = vec3(1, 1, 1);
uniform float Metallic = 0.0;
uniform float Roughness = 0.0;

float Distribution(float cosLh, float roughness)
{
	float a = roughness * roughness;
    float a2 = a * a;
    float nh2 = cosLh * cosLh;
    float denom = (nh2 * (a2 - 1.0) + 1.0);
	
    return a2 / max(PI * denom * denom, EPSILON);
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

vec3 Fresnel(vec3 f0, float cosTheta)
{
	return f0 + (1.0 - f0) * pow(1.0 - cosTheta, 5.0);
}

vec3 FresnelRoughness(vec3 f0, float cosTheta, float roughness)
{
    return f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(1.0 - cosTheta, 5.0);
}

vec3 Lighting(vec3 f0, vec3 view, vec3 albedo, vec3 normal, float roughness, float metallic)
{
	vec3 n = normal;
	vec3 v = view;

	vec3 result = vec3(0.0);
	for(int i = 0; i < Lights.directionalLightCount; ++i)
	{
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
		vec3 F = Fresnel(f0, max(dot(n, v), 0.0));
		float D = Distribution(cosLh, roughness);
		float G = Geometry(cosLi, nv, roughness);
		vec3 specular = (F * D * G) / max(4.0 * nv * cosLi, EPSILON);

		// Energy conservation
		vec3 kD = (1.0 - F) * (1.0 - metallic);
		vec3 diffuse = kD * albedo;
		
		result += (diffuse + specular) * radiance * cosLi;
	}
	return result;
}

vec3 IBL(vec3 f0, vec3 Lr, vec3 view, vec3 albedo, vec3 normal, float roughness, float metallic, float ao)
{
	float nv = max(dot(normal, view), 0.0);
	vec3 irradiance = texture(sys_irradianceMap, normal).rgb;
	vec3 F = FresnelRoughness(f0, nv, roughness);
	vec3 kD = (1.0 - F) * (1.0 - metallic);
	vec3 diffuseIBL = albedo * irradiance;

	const float maxLodLevel = 4.0;
	vec3 R = reflect(-view, normal);
	vec3 specularIrradiance = textureLod(sys_prefilterMap, R, roughness * roughness * maxLodLevel).rgb;
	vec2 specularBRDF = texture(sys_brdfLUTMap, vec2(nv, roughness * roughness)).rg;
	vec3 specularIBL = specularIrradiance * (F * specularBRDF.x + specularBRDF.y);

	return (kD * diffuseIBL + specularIBL) * ao;
}

void main()
{
	// texture maps
	vec3 albedo = TextureExists(AlbedoMap) ? texture(AlbedoMap, frag_texcoord).rgb : Albedo;
	vec3 normal = TextureExists(NormalMap) ? normalize(frag_TBN * normalize(texture(NormalMap, frag_texcoord).rgb * 2.0 - 1.0)) : normalize(frag_normal);
	float roughness = TextureExists(RoughnessMap) ? texture(RoughnessMap, frag_texcoord).r : Roughness;
	float metallic = TextureExists(MetallicMap) ? texture(MetallicMap, frag_texcoord).r : Metallic;
	float ambientOcclusion = TextureExists(AmbientOcclusionMap) ? texture(AmbientOcclusionMap, frag_texcoord).r : 1.0;

	const vec3 Fdielectric = vec3(0.04);
	vec3 view = normalize(sys_cameraPosition - frag_position);
	vec3 Lr = 2.0 * max(dot(normal, view), 0.0) * normal - view;
	vec3 f0 = mix(Fdielectric, albedo, metallic); // Fresnel reflectance
	
	vec3 LightContribution = Lighting(f0, view, albedo, normal, roughness, metallic);
	vec3 IBLContribution = IBL(f0, Lr, view, albedo, normal, roughness, metallic, ambientOcclusion);
	out_color = vec4(LightContribution + IBLContribution, 1.0);
} 