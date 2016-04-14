#version 420

in vec2 coord;
in vec4 pos;
out vec4 ocolor;
uniform vec4 clip;

#define pi 3.1415926535897932384626433832795
#define between(v,x1,x2) ((v) >= (x1) && (v) <= (x2))

uniform float L;
uniform sampler2D tex;

float normalize_angle(float a)
{
	if (a > pi * 2)
	{
		return a - pi * 2;
	}
	
	return a;
}

void main()
{
	if (any(bvec4(lessThan(pos.xy, clip.xy), greaterThan(pos.xy, clip.zw))))
	{
		discard;
	}

	vec2 pnt = (coord * 2.0) - 1.0;
	float angle = normalize_angle(atan(pnt.y, pnt.x) + 3 * pi / 2);

	if (!between(angle, 0, L))
	{
		discard;
	}

	ocolor = texture(tex, coord);
}