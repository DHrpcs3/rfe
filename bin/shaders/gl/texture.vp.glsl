#version 420

in vec4 position;
in vec2 icoord;

uniform mat4 MVP = mat4(1);

out vec2 coord;
out vec4 pos;

void main()
{
	coord = icoord;
	pos = MVP * position;
	gl_Position = pos;
}