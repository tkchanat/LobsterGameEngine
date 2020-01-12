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

uniform int ParticleCount; // [0, 1024]
uniform float ParticleSize;

void main()
{
    if(gl_PrimitiveIDIn >= ParticleCount) {
        EndPrimitive();
        return;
    }
    mat4 MV = sys_view * sys_world;
    vec3 right = vec3(MV[0][0], MV[1][0], MV[2][0]);
    vec3 up = vec3(MV[0][1], MV[1][1], MV[2][1]);
    vec3 position = gl_in[0].gl_Position.xyz;
    mat4 VP = sys_projection * sys_view;

    vec3 a = position - (right - up) * ParticleSize;
    gl_Position = VP * vec4(a, 1.0);
    fs_out.texcoord = vec2(0.0, 0.0);
    EmitVertex();

    vec3 b = position - (right + up) * ParticleSize;
    gl_Position = VP * vec4(b, 1.0);
    fs_out.texcoord = vec2(0.0, 1.0);
    EmitVertex();

    vec3 c = position + (right + up) * ParticleSize;
    gl_Position = VP * vec4(c, 1.0);
    fs_out.texcoord = vec2(1.0, 0.0);
    EmitVertex();
    
    vec3 d = position + (right - up) * ParticleSize;
    gl_Position = VP * vec4(d, 1.0);
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