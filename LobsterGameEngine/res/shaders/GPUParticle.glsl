///VertexShader
layout (location = 0) in vec3 in_position;
out VS_OUT {
    float dist;
} vs_out;

uniform int EmissionShape;

void main()
{
    vs_out.dist = EmissionShape == 0 ? (in_position.y + 1.0) / 2.0 : length(in_position);
    gl_Position = sys_world * vec4(in_position, 1.0); 
}

///GeometryShader
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in VS_OUT {
    float dist;
} vs_out[];

out FS_OUT {
    float dist;
    vec2 texcoord;
} fs_out;

uniform int ParticleCutoff; // [0, 1024]
uniform float ParticleSize;
uniform mat4 ParticleOrientation;

void main()
{
    if(gl_PrimitiveIDIn >= ParticleCutoff) {
        EndPrimitive();
        return;
    }
    vec3 position = gl_in[0].gl_Position.xyz;
    vec3 forward = normalize(position - sys_cameraPosition);
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = cross(forward, up);
    mat3 spriteMat = mat3(right, up, forward);

    vec3 unit_right = spriteMat * mat3(ParticleOrientation) * vec3(1, 0, 0);
    vec3 unit_up = spriteMat * mat3(ParticleOrientation) * vec3(0, 1, 0);

    vec3 a = position - (unit_right - unit_up) * ParticleSize;
    gl_Position = sys_projection * sys_view * vec4(a, 1.0);
    fs_out.dist = vs_out[0].dist;
    fs_out.texcoord = vec2(0.0, 0.0);
    EmitVertex();

    vec3 b = position - (unit_right + unit_up) * ParticleSize;
    gl_Position = sys_projection * sys_view * vec4(b, 1.0);
    fs_out.dist = vs_out[0].dist;
    fs_out.texcoord = vec2(0.0, 1.0);
    EmitVertex();

    vec3 c = position + (unit_right + unit_up) * ParticleSize;
    gl_Position = sys_projection * sys_view * vec4(c, 1.0);
    fs_out.dist = vs_out[0].dist;
    fs_out.texcoord = vec2(1.0, 0.0);
    EmitVertex();
    
    vec3 d = position + (unit_right - unit_up) * ParticleSize;
    gl_Position = sys_projection * sys_view * vec4(d, 1.0);
    fs_out.dist = vs_out[0].dist;
    fs_out.texcoord = vec2(1.0, 1.0);
    EmitVertex();
  
    EndPrimitive();
}

///FragmentShader
in FS_OUT {
    float dist;
    vec2 texcoord;
} fs_out;
out vec4 out_color;

uniform vec4 ColorStartTransition;
uniform vec4 ColorEndTransition;
uniform sampler2D ParticleTexture;

void main()
{
    vec4 tex_color = texture(ParticleTexture, fs_out.texcoord);
    vec4 color = mix(ColorStartTransition, ColorEndTransition, fs_out.dist);
    if(tex_color.a == 0.0) discard;
    out_color = tex_color * color;
}