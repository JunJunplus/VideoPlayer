#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D Ytexture;
uniform sampler2D Utexture;
uniform sampler2D Vtexture;

void main()
{
    vec3 rgb;
    vec3 yuv;
    yuv.r = texture2D(Ytexture, TexCoords).r;
    yuv.g = texture2D(Utexture, TexCoords).r - 0.5;
    yuv.b = texture2D(Vtexture, TexCoords).r - 0.5;
    rgb = mat3( 1,       1,         1,
                0,       -0.3455,   1.779,
                1.4075,  -0.7169,   0) * yuv;

    //vec3 col = texture(texture, TexCoords).rgb;
    FragColor = vec4(rgb, 1.0);
}
