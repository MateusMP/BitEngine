#version 150

in vec2 fragTextureCoord;

out vec4 finalColor;

uniform sampler2D u_texDiffuse;

void main()
{
	finalColor = texture(u_texDiffuse, fragTextureCoord);
}
