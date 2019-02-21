// This file is part of Geoflow
// Copyright (C) 2018-2019  Ravi Peters, 3D geoinformation TU Delft

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

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

uniform float u_cam_pos;

uniform float u_value_max;
uniform float u_value_min;

uniform float u_pointsize;
uniform vec4 u_color;
uniform int u_color_mode; // 3==texture-direct 2==texture-gradient, 1==uniform, 0==pervertex

uniform float u_ambient;
uniform float u_diffuse;
uniform float u_specular;
uniform vec3 u_light_direction;
uniform vec4 u_light_color;

out vec3 ourColor;
out float texCoord;
out vec3 lightFactor;
flat out int colorMode;

void main()
{
    vec4 pos = u_mvp * vec4(position, 1.0);
    gl_Position = pos;
    
    vec3 n = normalize(u_mv_normal * normal); 

    float diffuse = u_diffuse*max(dot(n, normalize(-u_light_direction)), 0.0);
    
    vec3 viewDir = normalize(vec3(0,0,u_cam_pos) - pos.xyz);
    vec3 reflectDir = reflect(-u_light_direction, n);
    float specular = u_specular*pow(max(dot(viewDir, reflectDir), 0.0), 4);
    
    lightFactor = (u_ambient + diffuse + specular) * u_light_color.xyz;

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