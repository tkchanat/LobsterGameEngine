///VertexShader
layout (location = 0) in vec3 in_position;
out VS_OUT {
    vec4 position;
} vs_out;

void main()
{
    vs_out.position = sys_world * vec4(in_position, 1.0);
    gl_Position = sys_world * vec4(in_position, 1.0); 
}

///GeometryShader
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in VS_OUT {
    vec4 position;
} vs_out[];

out FS_OUT {
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
    fs_out.texcoord = vec2(0.0, 0.0);
    EmitVertex();

    vec3 b = position - (unit_right + unit_up) * ParticleSize;
    gl_Position = sys_projection * sys_view * vec4(b, 1.0);
    fs_out.texcoord = vec2(0.0, 1.0);
    EmitVertex();

    vec3 c = position + (unit_right + unit_up) * ParticleSize;
    gl_Position = sys_projection * sys_view * vec4(c, 1.0);
    fs_out.texcoord = vec2(1.0, 0.0);
    EmitVertex();
    
    vec3 d = position + (unit_right - unit_up) * ParticleSize;
    // d = spriteMat * d;
    gl_Position = sys_projection * sys_view * vec4(d, 1.0);
    fs_out.texcoord = vec2(1.0, 1.0);
    EmitVertex();
  
    EndPrimitive();
}

///FragmentShader
in FS_OUT {
    vec2 texcoord;
} fs_out;
out vec4 out_color;

uniform sampler2D ParticleTexture;

void main()
{
    vec4 color = texture(ParticleTexture, fs_out.texcoord);
    if(color.a == 0.0) discard;
    out_color = color;
}