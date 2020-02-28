///VertexShader
layout (location = 0) in vec3 in_position;
layout (location = 2) in vec2 in_texcoord;

out vec2 frag_texcoord;

void main()
{
    frag_texcoord = in_texcoord;
    gl_Position = vec4(in_position, 1.0); 
}

///FragmentShader
in vec2 frag_texcoord;
out vec4 out_color;

// const float 0.25 = 0.25;
// const float 0.5 = 0.5;

uniform sampler2D sys_positionGBuffer;
uniform sampler2D sys_normalGBuffer;
uniform sampler2D sys_albedoGBuffer;

vec4 CalcDirectionalLight(DirectionalLight light, vec3 position, vec3 normal, vec4 albedo, vec3 viewDir) 
{
    vec3 lightDir = normalize(light.direction);
    // diffuse shading
    float intensity = max(dot(normal, lightDir), 0.25);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float multiplier = pow(max(dot(viewDir, reflectDir), 0.0), 16.0 * (0.5 + EPSILON));
    // // combine results
    vec3 specular = 0.5 * multiplier * light.color;
    vec3 color = (albedo.rgb * intensity + specular);
    return vec4(color, albedo.a);
}

vec4 CalcPointLight(PointLight light, vec3 position, vec3 normal, vec4 albedo, vec3 viewDir) 
{
    vec3 lightDir = normalize(light.position - position);
    // attenuation
    float distance = length(light.position - position);
	float quadratic_attenuation = (light.attenuation + 0.05) * distance * distance;
	float attenuation = 1.0 / quadratic_attenuation;
    vec3 radiance = vec3(light.color) * attenuation;
    // diffuse shading
    float intensity = max(dot(normal, lightDir), 0.25);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float multiplier = pow(max(dot(viewDir, reflectDir), 0.0), 16.0 * (0.5 + EPSILON));
    // combine results
    vec3 specular = 0.5 * multiplier * light.color;
    vec3 color = (albedo.rgb * intensity + specular) * radiance;
    return vec4(color, albedo.a);
}

void main()
{
    // calculate normal in tangent space
    vec3 position = texture(sys_positionGBuffer, frag_texcoord).rgb;
    vec3 normal = texture(sys_normalGBuffer, frag_texcoord).rgb;
    vec4 albedo = texture(sys_albedoGBuffer, frag_texcoord).rgba;
    vec3 viewDir = normalize(sys_cameraPosition - position);

    vec4 result = vec4(0.0);
    for(int i = 0; i < Lights.directionalLightCount; ++i) {
        // Shadow
        vec4 lightSpacePosition = Lights.lightSpaceMatrix[i] * vec4(position, 1.0);
        lightSpacePosition /= lightSpacePosition.w;
        lightSpacePosition = lightSpacePosition * 0.5 + 0.5;
        float closestDepth = texture(sys_shadowMap[i], lightSpacePosition.xy).r; 
        float currentDepth = lightSpacePosition.z;
        float bias = max(0.05 * (1.0 - dot(normal, Lights.directionalLights[i].direction)), 0.001);
        float shadow = currentDepth - bias > closestDepth ? 0.6 : 0.0;
        shadow = lightSpacePosition.z > 1.0 ? 0.0 : shadow;
        if(i > MAX_DIRECTIONAL_SHADOW) shadow = 0.0;

        // merge lightings
        result += (1.0 - shadow) * Lights.directionalLights[i].intensity * CalcDirectionalLight(Lights.directionalLights[i], position, normal, albedo, viewDir);
    }
    for(int i = 0; i < Lights.pointLightCount; ++i) {
        result += CalcPointLight(Lights.pointLights[i], position, normal, albedo, viewDir);
    }
    out_color = vec4(result);
}
