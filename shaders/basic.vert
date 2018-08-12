#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

uniform float u_pointsize;
uniform vec4 u_color;
uniform int u_color_mode; // 1==uniform, 0==pervertex

out vec3 ourColor;

void main()
{
    gl_Position = u_projection * u_view * u_model * vec4(position, 1.0);
    if(u_color_mode==1)
        ourColor = u_color.xyz;
    else if(u_color_mode==0)
        ourColor = color;
    gl_PointSize = u_pointsize;
}