///VertexShader
layout (location = 0) in vec3 in_position;
layout (location = 2) in vec2 in_texcoord;

out vec2 frag_texcoord;
out vec3 frag_viewRay;

void main()
{
    frag_texcoord = in_texcoord;
    vec3 farPlaneCoord = in_position + vec3(0, 0, 1);
    vec4 positionWorldSpace = inverse(sys_projection * sys_view) * vec4(farPlaneCoord, 1.0);
    positionWorldSpace /= positionWorldSpace.w;
    frag_viewRay = positionWorldSpace.xyz - sys_cameraPosition;
    gl_Position = vec4(in_position, 1.0); 
}

///FragmentShader
in vec2 frag_texcoord;
in vec3 frag_viewRay;
out vec4 out_color;

uniform sampler2D sys_gNormalDepth;
uniform sampler2D sys_gMetalRoughAO;
uniform sampler2D sys_gAlbedo;

// vec4 CalcDirectionalLight(DirectionalLight light, vec3 position, vec3 normal, vec4 albedo, vec3 viewDir) 
// {
//     vec3 lightDir = normalize(light.direction);
//     // diffuse shading
//     float intensity = max(dot(normal, lightDir), 0.25);
//     // specular shading
//     vec3 reflectDir = reflect(-lightDir, normal);
//     float multiplier = pow(max(dot(viewDir, reflectDir), 0.0), 16.0 * (0.5 + EPSILON));
//     // // combine results
//     vec3 specular = 0.5 * multiplier * light.color;
//     vec3 color = (albedo.rgb * intensity + specular);
//     return vec4(color, albedo.a);
// }

// vec4 CalcPointLight(PointLight light, vec3 position, vec3 normal, vec4 albedo, vec3 viewDir) 
// {
//     vec3 lightDir = normalize(light.position - position);
//     // attenuation
//     float distance = length(light.position - position);
// 	float quadratic_attenuation = (light.attenuation + 0.05) * distance * distance;
// 	float attenuation = 1.0 / quadratic_attenuation;
//     vec3 radiance = vec3(light.color) * attenuation;
//     // diffuse shading
//     float intensity = max(dot(normal, lightDir), 0.25);
//     // specular shading
//     vec3 reflectDir = reflect(-lightDir, normal);
//     float multiplier = pow(max(dot(viewDir, reflectDir), 0.0), 16.0 * (0.5 + EPSILON));
//     // combine results
//     vec3 specular = 0.5 * multiplier * light.color;
//     vec3 color = (albedo.rgb * intensity + specular) * radiance;
//     return vec4(color, albedo.a);
// }

// void main()
// {
//     // calculate normal in tangent space
//     vec3 normal = texture(sys_gNormalDepth, frag_texcoord).rgb;
//     float depth = texture(sys_gNormalDepth, frag_texcoord).a;
//     vec3 position = sys_cameraPosition + normalize(frag_viewRay) * depth;
//     vec4 albedo = texture(sys_gAlbedo, frag_texcoord);
//     vec3 viewDir = normalize(sys_cameraPosition - position);

//     vec4 result = vec4(0.0);
//     for(int i = 0; i < Lights.directionalLightCount; ++i) {
//         // Shadow
//         vec4 lightSpacePosition = Lights.lightSpaceMatrix[i] * vec4(position, 1.0);
//         lightSpacePosition /= lightSpacePosition.w;
//         lightSpacePosition = lightSpacePosition * 0.5 + 0.5;
//         float closestDepth = texture(sys_shadowMap[i], lightSpacePosition.xy).r; 
//         float currentDepth = lightSpacePosition.z;
//         float bias = max(0.05 * (1.0 - dot(normal, Lights.directionalLights[i].direction)), 0.001);
//         float shadow = currentDepth - bias > closestDepth ? 0.6 : 0.0;
//         shadow = lightSpacePosition.z > 1.0 ? 0.0 : shadow;
//         if(i > MAX_DIRECTIONAL_SHADOW) shadow = 0.0;

//         // merge lightings
//         result += (1.0 - shadow) * Lights.directionalLights[i].intensity * CalcDirectionalLight(Lights.directionalLights[i], position, normal, albedo, viewDir);
//     }
//     for(int i = 0; i < Lights.pointLightCount; ++i) {
//         result += CalcPointLight(Lights.pointLights[i], position, normal, albedo, viewDir);
//     }
//     out_color = vec4(result);
// }

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

vec3 DirectionalLighting(DirectionalLight light, vec3 f0, vec3 view, vec3 albedo, vec3 normal, float roughness, float metallic)
{
	vec3 n = normal;
	vec3 v = view;

	vec3 result = vec3(0.0);
	vec3 l = normalize(light.direction);
	vec3 h = normalize(v + l);
	float cosLi = max(dot(n, l), 0.0);
	float cosLh = max(dot(n, h), 0.0);
	float nv = max(dot(n, v), 0.0);

	// Radiance
	vec3 radiance = vec3(light.color);

	// BRDF
	vec3 F = Fresnel(f0, max(dot(n, v), 0.0));
	float D = Distribution(cosLh, roughness);
	float G = Geometry(cosLi, nv, roughness);
	vec3 specular = (F * D * G) / max(4.0 * nv * cosLi, EPSILON);

	// Energy conservation
	vec3 kD = (1.0 - F) * (1.0 - metallic);
	vec3 diffuse = kD * albedo;
	
	result += (diffuse + specular) * radiance * cosLi;
	return result;
}

vec3 PointLighting(PointLight light, vec3 f0, vec3 view, vec3 albedo, vec3 position, vec3 normal, float roughness, float metallic)
{
	vec3 n = normal;
	vec3 v = view;

	vec3 result = vec3(0.0);
	vec3 l = normalize(light.position - position);
	vec3 h = normalize(v + l);
	float cosLi = max(dot(n, l), 0.0);
	float cosLh = max(dot(n, h), 0.0);
	float nv = max(dot(n, v), 0.0);

	// Radiance
	float distance = length(light.position - position);
	float quadratic_attenuation = (light.attenuation + 0.05) * distance * distance;
	float attenuation = 1.0 / quadratic_attenuation;
	vec3 radiance = vec3(light.color) * attenuation;

	// BRDF
	vec3 F = Fresnel(f0, nv);
	float D = Distribution(cosLh, roughness);
	float G = Geometry(cosLi, nv, roughness);
	vec3 specular = (F * D * G) / max(4.0 * nv * cosLi, EPSILON);

	// Energy conservation
	vec3 kD = (1.0 - F) * (1.0 - metallic);
	vec3 diffuse = kD * albedo;
	
	result += (diffuse + specular) * radiance * cosLi;
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
    vec3 normal = texture(sys_gNormalDepth, frag_texcoord).rgb;
    float depth = texture(sys_gNormalDepth, frag_texcoord).a;
    vec3 position = sys_cameraPosition + normalize(frag_viewRay) * depth;
    vec3 albedo = texture(sys_gAlbedo, frag_texcoord).rgb;
    vec3 viewDir = normalize(sys_cameraPosition - position);
    float metallic = texture(sys_gMetalRoughAO, frag_texcoord).r;
    float roughness = texture(sys_gMetalRoughAO, frag_texcoord).g;
    float ambientOcclusion = texture(sys_gMetalRoughAO, frag_texcoord).b;
	const vec3 Fdielectric = vec3(0.04);
	vec3 view = normalize(sys_cameraPosition - position);
	vec3 Lr = 2.0 * max(dot(normal, view), 0.0) * normal - view;
	vec3 f0 = mix(Fdielectric, albedo, metallic); // Fresnel reflectance
	
	vec3 LightContribution = vec3(0.0);
	for(int i = 0; i < Lights.directionalLightCount; ++i) {
		// Shadow
        vec4 lightSpacePosition = Lights.lightSpaceMatrix[i] * vec4(position, 1.0);
        lightSpacePosition /= lightSpacePosition.w;
        lightSpacePosition = lightSpacePosition * 0.5 + 0.5;
        float closestDepth = texture(sys_shadowMap[i], lightSpacePosition.xy).r; 
        float currentDepth = lightSpacePosition.z;
        float bias = max(0.05 * (1.0 - dot(normal, Lights.directionalLights[i].direction)), 0.005);
        float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
        shadow = lightSpacePosition.z > 1.0 ? 0.0 : shadow;
        if(i > MAX_DIRECTIONAL_SHADOW) shadow = 0.0;
		// merge lightings
		LightContribution += (1.0 - shadow) * Lights.directionalLights[i].intensity * DirectionalLighting(Lights.directionalLights[i], f0, view, albedo, normal, roughness, metallic);
	}
	for(int i = 0; i < Lights.pointLightCount; ++i) {
		LightContribution += PointLighting(Lights.pointLights[i], f0, view, albedo, position, normal, roughness, metallic);
    }

	vec3 IBLContribution = IBL(f0, Lr, view, albedo, normal, roughness, metallic, ambientOcclusion);
	vec4 result = vec4(LightContribution + IBLContribution, 1.0);
	out_color = vec4(result);
} 