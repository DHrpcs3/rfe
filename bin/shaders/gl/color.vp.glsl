#version 420

in vec4 position;
in vec4 icolor;

uniform mat4 MVP = mat4(1);

out vec4 color;
out vec4 pos;

void main()
{
	color = icolor;
	pos = MVP * position;
	gl_Position = pos;
}