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

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	// pass useful information into fragment shader
    frag_position = vec3(world * vec4(in_position, 1.0));
    frag_normal = mat3(world) * in_normal;
	frag_texcoord = in_texcoord;

    gl_Position = projection * view * world * vec4(in_position, 1.0);
}

///FragmentShader
#version 410 core
in vec3 frag_position;
in vec3 frag_normal;
in vec2 frag_texcoord;
out vec4 out_color;

layout (std140) uniform ubo_PBR
{
	vec3 Albedo;		float Metallic;
	float Roughness;	float AmbientOcclusion;
	bool UseAlbedoMap;		bool UseNormalMap;
	bool UseRoughnessMap;	bool UseMetallicMap;
	bool UseAmbientOcclusionMap; bool padding0;
} PBR;

uniform vec3 cameraPosition;
uniform vec3 lightPosition;
uniform vec3 lightDirection;
uniform vec3 lightColor;

uniform sampler2D AlbedoMap;
uniform sampler2D NormalMap;
uniform sampler2D RoughnessMap;
uniform sampler2D MetallicMap;
uniform sampler2D AmbientOcclusionMap;

const float PI = 3.14159265359;

float Distribution(vec3 n, vec3 h, float roughness)
{
	float a = roughness * roughness;
    float a2 = a * a;
    float nh = max(dot(n, h), 0.0);
    float nh2 = nh * nh;
	
    float nom = a2;
    float denom = (nh2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return nom / max(denom, 0.001);
}

float SchlickGGX(float nv, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom = nv;
    float denom = nv * (1.0 - k) + k;
	
    return nom / max(denom, 0.001);
}
  
float Geometry(vec3 n, vec3 v, vec3 l, float roughness)
{
    float nv = max(dot(n, v), 0.0);
    float nl = max(dot(n, l), 0.0);
    float ggx1 = SchlickGGX(nv, roughness);
    float ggx2 = SchlickGGX(nl, roughness);
	
    return ggx1 * ggx2;
}

vec3 Fresnel(float cosTheta, vec3 f0)
{
	return f0 + (1.0 - f0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
	// texture maps
	vec3 albedo = mix(vec3(1.0), texture(AlbedoMap, frag_texcoord).rgb, float(PBR.UseAlbedoMap));

	vec3 n = normalize(frag_normal);
	vec3 v = normalize(cameraPosition - frag_position);

	// choose initial reflectance value at normal incidence
	vec3 f0 = vec3(0.04);
	f0 = mix(f0, albedo, PBR.Metallic);

	// reflectance equation
	vec3 Lo = vec3(0.0);
	// for(int i = 0; i < 1; ++i)
	// {
		// Radiance
		vec3 l = normalize(lightPosition);
		vec3 h = normalize(v + l);
		float distance = length(lightPosition - frag_position);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = lightColor * attenuation;

		// BRDF
		float D = Distribution(n, h, PBR.Roughness);
		float G = Geometry(n, v, l, PBR.Roughness);
		vec3 F = Fresnel(clamp(dot(h, v), 0.0, 1.0), f0);
		vec3 nom = D * G * F;
		float denom = max(dot(n, v), 0.0) * max(dot(n, l), 0.0);
		vec3 specular = nom / max(denom, 0.001);

		// Energy conservation
		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - PBR.Metallic;
		
		float intensity = max(dot(n, l), 0.0);
		Lo += (kD * albedo / PI + specular) * radiance * intensity;
	// }

	vec3 ambient = PBR.Albedo * PBR.AmbientOcclusion * albedo;
	vec3 color = ambient + Lo;

	// HDR tonemapping
    // color = color / (color + vec3(1.0));
    // gamma correct
    // color = pow(color, vec3(1.0/2.2)); 

	out_color = vec4(color, 1.0);
} 