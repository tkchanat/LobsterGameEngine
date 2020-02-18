///VertexShader
layout (location = 0) in vec4 in_data; // <x, y, w, h>
out vec2 frag_texcoord;

void main()
{
    frag_texcoord = in_data.zw;
    gl_Position = sys_projection * sys_world * vec4(in_data.xy, 0.0, 1.0); 
}  

///FragmentShader
in vec2 frag_texcoord;
out vec4 out_color;

uniform float alpha;
uniform vec4 blend;
uniform sampler2D sys_background;
uniform sampler2D sys_spriteTexture;

void main()
{ 
	vec4 base = texelFetch(sys_background, ivec2(gl_FragCoord.xy), 0);
    vec4 color = texture(sys_spriteTexture, frag_texcoord);	
    color = color * (1 - blend.a) + blend * blend.a;
    color.a *= alpha;

	if (color.a < 0.05) discard;
	out_color = vec4(vec3(color) * color.a + vec3(base) * (1.0 - color.a), alpha);    	
}
