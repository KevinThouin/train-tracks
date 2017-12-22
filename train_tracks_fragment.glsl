#version 330

uniform vec3 vertexColor;

out vec4 outputColor;

void main() {
  outputColor = vec4(vertexColor, 1.0);
}
