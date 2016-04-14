#version 420

in vec4 icoord;
out vec2 coord;

uniform mat4 MVP = mat4(1);
out vec4 pos;

void main()
{
	pos = MVP * vec4(icoord.xy, 0, 1);
	gl_Position = pos;
	coord = icoord.zw;
}