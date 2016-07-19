
#version 150

attribute vec2 a_position;
attribute vec2 a_uvcoord;

varying vec2 fragTextureCoord;

uniform mat4 u_viewMatrix;

void main()
{
	gl_Position.xy = (u_viewMatrix * vec4(a_position, 0, 1.0f)).xy;
	gl_Position.z = 0.0;
	gl_Position.w = 1.0;

	fragTextureCoord = a_uvcoord;
}