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
in vec3 ourColor;
in float texCoord;
flat in int colorMode;
in vec3 lightFactor;

layout(location=0) out vec4 color;
uniform sampler1D u_sampler;

void main()
{
    if(colorMode==2 || colorMode==3)
        color = vec4(lightFactor*texture(u_sampler, texCoord).xyz, 1.0);
    else
        color = vec4(lightFactor*ourColor,1.0);
}