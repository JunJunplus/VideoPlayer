#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture;

void main()
{
    vec3 col = texture(texture, TexCoords).rgb;
    FragColor = vec4(col, 1.0);
}
