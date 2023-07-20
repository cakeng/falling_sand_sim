#version 430 core
out vec4 FragColor;
in vec3 vtxCol;

void main()
{
    FragColor = vec4 (vtxCol, 0.0);
}