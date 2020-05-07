///VertexShader
layout (location = 0) in vec3 in_position;
layout (location = 2) in vec2 in_texcoord;

out vec2 frag_texcoord;
out vec3 frag_viewRay;
out mat4 frag_viewProjection;

void main()
{
    frag_texcoord = in_texcoord;
    vec3 farPlaneCoord = in_position + vec3(0, 0, 1);
    vec4 positionWorldSpace = inverse(sys_projection * sys_view) * vec4(farPlaneCoord, 1.0);
    positionWorldSpace /= positionWorldSpace.w;
    frag_viewRay = positionWorldSpace.xyz - sys_cameraPosition;
    frag_viewProjection = sys_projection * sys_view;
    gl_Position = vec4(in_position, 1.0); 
}  

///FragmentShader
in vec2 frag_texcoord;
in vec3 frag_viewRay;
in mat4 frag_viewProjection;
out vec4 out_color;

const float offset = 1.0f / 256.0f;
const vec2 offsets[9] = vec2[9](
    vec2( -offset,  offset  ),  // top-left
    vec2(  0.0f,    offset  ),  // top-center
    vec2(  offset,  offset  ),  // top-right
    vec2( -offset,  0.0f    ),  // center-left
    vec2(  0.0f,    0.0f    ),  // center-center
    vec2(  offset,  0.0f    ),  // center - right
    vec2( -offset, -offset  ),  // bottom-left
    vec2(  0.0f,   -offset  ),  // bottom-center
    vec2(  offset, -offset  )   // bottom-right    
);
const float blur_kernel[9] = float[9](
    1.0/16.0, 2.0/16.0, 1.0/16.0,
    2.0/16.0, 4.0/16.0, 2.0/16.0,
    1.0/16.0, 2.0/16.0, 1.0/16.0
);

uniform sampler2D screenTexture;
uniform sampler2D sys_gNormalDepth;
uniform bool sys_ppBlur;
uniform bool sys_ppSSR;
uniform bool sys_ppUseKernel;
uniform bool sys_ppBlend;
uniform bool sys_ppSobel;
uniform float sys_ppSobelThreshold;
uniform mat3 sys_ppKernel;
uniform vec4 sys_ppBlendColor;

vec3 Blur() {
    vec3 color = vec3(0.0);
    vec3 samples[9];
    for(int i = 0; i < 9; i++)
        samples[i] = texture(screenTexture, frag_texcoord + offsets[i]).rgb;
    for(int i = 0; i < 9; i++)
        color += samples[i] * blur_kernel[i];
    return color;
}

vec3 SSR() {
    vec3 normal = texture(sys_gNormalDepth, frag_texcoord).rgb;
    float depth = texture(sys_gNormalDepth, frag_texcoord).a;
    vec3 rayDir = normalize(frag_viewRay);

    const int MAX_ITERATION = 128;
    float stepSize = 10.0;
    vec3 specularColor = vec3(0);
    vec3 diffuseColor = texture(screenTexture, frag_texcoord).rgb;
    vec3 position = sys_cameraPosition + rayDir * depth;
    rayDir = reflect(rayDir, normal);
    if(depth > 1.0) {
        for(int i = 0; i < MAX_ITERATION; ++i) {
            vec3 worldPos = position + rayDir * stepSize;
            vec4 screenPos = frag_viewProjection * vec4(worldPos, 1.0);
            if(screenPos.w <= 0.0) break; // outside screen
            screenPos /= screenPos.w;
            if(screenPos.x < -1.0 || screenPos.x > 1.0) break;
            if(screenPos.y < -1.0 || screenPos.y > 1.0) break;
            vec2 screenUV = (screenPos.xy + 1.0) / 2.0;
            float worldDepth = distance(worldPos, sys_cameraPosition);
            float screenDepth = texture(sys_gNormalDepth, screenUV).a;
            if(worldDepth > screenDepth) {
                stepSize *= 0.5;
                continue; // overshoot
            }
            else if(abs(worldDepth-screenDepth) < EPSILON) {
                specularColor = texture(screenTexture, screenUV).rgb;
                break; // reached
            }
            position = worldPos;
        }
    }
    return diffuseColor + 0.5 * specularColor;
}

// Apply a user-specified 3x3 kernel on the screen
vec3 Kernel3x3() {
    vec3 color = vec3(0.0);
    vec3 samples[9];
    for(int i = 0; i < 9; i++)
        samples[i] = texture(screenTexture, frag_texcoord + offsets[i]).rgb;
    for(int i = 0; i < 3; i++)
        for(int j = 0; j < 3; j++)
            color += samples[i*3+j] * sys_ppKernel[i][j];
    return color;
}

vec3 SobelEdgeDetector(float threshold) {
    const float sobel_x[9] = float[9](
    1, 0, -1,
    2, 0, -2,
    1, 0, -1
    );
    const float sobel_y[9] = float[9](
    1, 2, 1,
    0, 0, 0,
    -1, -2, -1
    );
    vec3 color_x = vec3(0.0);
    vec3 color_y = vec3(0.0);
    vec3 samples[9];
    for(int i = 0; i < 9; i++) {
        samples[i] = texture(screenTexture, frag_texcoord + offsets[i]).rgb;
        color_x += samples[i] * sobel_x[i];
        color_y += samples[i] * sobel_y[i];
    }
    vec3 c = sqrt(color_x * color_x + color_y * color_y);
    if (threshold > 0.0) {
        // set to completely white if larger than the non-negative threshold
        if (c.r > threshold) c = vec3(1.0, 1.0, 1.0);        
        else c = vec3(0, 0, 0);
    }
    return c;
}


void main()
{ 
    int effectCount = 0;
    vec3 color = vec3(0.0);
    vec3 blur = vec3(0.0);
    vec3 ssr = vec3(0.0);
    vec3 kernelProcess = vec3(0.0);
    vec3 sobel = vec3(0.0);
    if(sys_ppSSR) { blur = SSR(); effectCount++; }
    if(sys_ppBlur) { ssr = Blur(); effectCount++; }
    if(sys_ppUseKernel) { kernelProcess = Kernel3x3(); effectCount++; }
    if(sys_ppSobel) { sobel = SobelEdgeDetector(sys_ppSobelThreshold); effectCount++; }

    // merge color
    if(effectCount > 0) {
        color = (kernelProcess + blur + ssr + sobel) / effectCount;        
    }
    else {
        color = texture(screenTexture, frag_texcoord).rgb;
    }
    if (sys_ppBlend) {
        color += sys_ppBlendColor.rgb * sys_ppBlendColor.a;
    }
    out_color = vec4(color, 1.0);
}
