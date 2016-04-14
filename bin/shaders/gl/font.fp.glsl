#version 420

in vec2 coord;
in vec4 pos;
uniform vec4 color;
out vec4 ocolor;
uniform vec4 clip;

uniform sampler2D tex;

void main()
{
	if (any(bvec4(lessThan(vec2(1, -1) * pos.xy, clip.xy), greaterThan(vec2(1, -1) * pos.xy, clip.zw))))
	{
		//discard;
	}

	ocolor = vec4(color.xyz, color.w * texture(tex, coord).x);
}