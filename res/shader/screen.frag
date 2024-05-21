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
    // yuv.r = texture2D(Ytexture, TexCoords).r;
    // yuv.g = texture2D(Utexture, TexCoords).r;
    // yuv.b = texture2D(Vtexture, TexCoords).r;
    // yuv -= vec3(16. / 255., 128. / 255., 128. / 255.);

    yuv.x = texture2D(Ytexture, TexCoords.st).r;
    yuv.y = texture2D(Utexture, TexCoords.st).r - 0.5;
    yuv.z = texture2D(Utexture, TexCoords.st).g - 0.5;

    rgb = mat3(1.0, 1.0, 1.0,
               0.0, -0.39465, 2.03211,
               1.13983, -0.58060, 0.0) * yuv;

    //vec3 col = texture(texture, TexCoords).rgb;
    FragColor = vec4(rgb, 1.0);
}
