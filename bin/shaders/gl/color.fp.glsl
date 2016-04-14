#version 420

in vec4 color;
in vec4 pos;
out vec4 ocolor;
uniform vec4 clip;

void main()
{
	if (any(bvec4(lessThan(vec2(1, -1) * pos.xy, clip.xy), greaterThan(vec2(1, -1) * pos.xy, clip.zw))))
	{
		discard;
	}

	ocolor = color;
}