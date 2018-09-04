#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in float value;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

uniform float u_value_max;
uniform float u_value_min;

uniform float u_pointsize;
uniform vec4 u_color;
uniform int u_color_mode; // 2==texture, 1==uniform, 0==pervertex

out vec3 ourColor;
out float texCoord;
out int colorMode;

void main()
{
    gl_Position = u_projection * u_view * u_model * vec4(position, 1.0);
    texCoord = (value-u_value_min)/(u_value_max-u_value_min);
    colorMode = u_color_mode;
    if(u_color_mode==1)
        ourColor = u_color.xyz;
    else if(u_color_mode==0)
        ourColor = color;
    gl_PointSize = u_pointsize;
}