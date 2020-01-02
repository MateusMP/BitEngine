#version 150

in vec2 a_vertexPosition;
in vec2 a_textureCoord;

out vec2 fragTextureCoord;

void main()
{
	gl_Position.xy = a_vertexPosition;
	gl_Position.z = 0.0;
	gl_Position.w = 1.0;
	
	fragTextureCoord = a_textureCoord;
}