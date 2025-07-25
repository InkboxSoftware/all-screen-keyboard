#version 300 es

layout(location = 0) in vec2 vertPos;
layout(location = 1) in vec3 vertTexCoord;

out vec2 texCoord;
out float alpha;

void main(){
	texCoord = vec2(vertTexCoord[0], vertTexCoord[1]);
	alpha = vertTexCoord[2];
	gl_Position = vec4(vertPos, 0.0, 1.0);
}