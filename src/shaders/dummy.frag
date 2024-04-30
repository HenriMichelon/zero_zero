/*
	Dummy shader outputing white fragments
*/
#version 450

layout (location = 0) out vec4 COLOR;

void main()
{
	COLOR = vec4(1.0, 1.0, 1.0, 1.0);
}