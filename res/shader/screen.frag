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
    yuv.g = texture2D(Utexture, TexCoords).r;
    yuv.b = texture2D(Vtexture, TexCoords).r;
    yuv -= vec3(16. / 255., 128. / 255., 128. / 255.);
    rgb = mat3( 1,       1,         1,
                0,       -0.1873,   1.8556,
                1.5748,  -0.4681,   0) * yuv;

    //vec3 col = texture(texture, TexCoords).rgb;
    FragColor = vec4(rgb, 1.0);
}
