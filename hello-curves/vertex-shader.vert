#version 410

// Declara as variáveis de entrada (inputs) do shader
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 tex_coord;
layout (location = 3) in vec3 normal;

// Declara as variáveis uniformes do shader
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Declara as variáveis de saída (outputs) do shader
out vec3 finalColor;
out vec3 scaledNormal;
out vec2 textureCoord;
out vec3 fragmentPosition;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
    scaledNormal = normal;
    finalColor = color;
    textureCoord = vec2(tex_coord.x, 1 - tex_coord.y);
    fragmentPosition = vec3(model * vec4(position, 1.0));
}