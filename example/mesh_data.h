#pragma once

#include "render/render.h"

const gin::MeshData default_mesh = {{
										1.0f, 1.0f,   // top right
										1.0f, -1.0f,  // bottom right
										-1.0f, -1.0f, // bottom left
										-1.0f, 1.0f   // top left
									},
									{1.f, 0.f, 1.f, 1.f, 0.f, 1.f, 0.f, 0.f},
									{0, 1, 2, 2, 0, 3}};

const gin::MeshData bg_mesh = {
	{40.f, 40.f, 40.f, -40.f, -40.f, -40.f, -40.f, 40.f},
	{1.f, 0.f, 1.f, 1.f, 0.f, 1.f, 0.f, 0.f},
	{0, 1, 2, 2, 0, 3}};
