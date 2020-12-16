#pragma once

#include <string>

const std::string default_vertex_shader = R"(#version 330 core

layout (location = 0) in vec2 vertices;
layout (location = 1) in vec2 uvs;

out vec2 tex_coord;

uniform mat3 mvp;

void main(){
	vec3 transformed = mvp * vec3(vertices, 1.0);
	gl_Position = vec4(transformed.x, transformed.y, -1.0, transformed.z);
	tex_coord = uvs;
}
)";

const std::string default_fragment_shader = R"(#version 330 core

in vec2 tex_coord;

out vec4 frag_colour;

uniform sampler2D texture_sampler;

void main(){
	vec4 tex_colour = texture(texture_sampler, tex_coord);
	frag_colour = vec4(1.0, 0.0, 0.0, 1.0);
}
)";