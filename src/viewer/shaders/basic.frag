#version 330 core
in vec3 ourColor;
in float texCoord;
flat in int colorMode;
in vec3 lightFactor;

out vec4 color;
uniform sampler1D u_sampler;

void main()
{
    if(colorMode==2 || colorMode==3)
        color = vec4(lightFactor*texture(u_sampler, texCoord).xyz, 1.0);
    else
        color = vec4(lightFactor*ourColor,1.0);
}