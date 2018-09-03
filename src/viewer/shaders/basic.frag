#version 330 core
in vec3 ourColor;

out vec4 color;

uniform sampler1D u_sampler;

void main()
{
    color = vec4(ourColor, 1.0f);
    //color = texture(u_colormap, v_color_intensity);
}