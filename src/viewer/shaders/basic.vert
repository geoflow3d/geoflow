#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in float value;
layout (location = 3) in float identifier;
layout (location = 4) in vec3 normal;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_mvp;
uniform mat3 u_mv_normal;

uniform float u_value_max;
uniform float u_value_min;

uniform float u_pointsize;
uniform vec4 u_color;
uniform int u_color_mode; // 3==texture-direct 2==texture-gradient, 1==uniform, 0==pervertex

uniform float u_ambient;
uniform float u_diffuse;
uniform vec3 u_light_direction;
uniform vec4 u_light_color;

out vec3 ourColor;
out float texCoord;
out vec3 lightFactor;
flat out int colorMode;

void main()
{
    gl_Position = u_mvp * vec4(position, 1.0);
    
    vec3 n = normalize(u_mv_normal * normal); 

    float diffuse = u_diffuse*max(dot(n, normalize(-u_light_direction)), 0.0);
    lightFactor = (u_ambient + diffuse) * u_light_color.xyz;

    colorMode = u_color_mode;
    if(u_color_mode==1)
        ourColor = u_color.xyz;
    if(u_color_mode==2)
        texCoord = (value-u_value_min)/(u_value_max-u_value_min);
    if(u_color_mode==3)
        texCoord = identifier;
    else if(u_color_mode==0)
        ourColor = color;
    gl_PointSize = u_pointsize;
}