#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

uniform float u_pointsize;

out vec3 ourColor;

void main()
{
    gl_Position = u_projection * u_view * u_model * vec4(position, 1.0);
    ourColor = color;
    gl_PointSize = u_pointsize;
}