#pragma once

#include "render/render.h"

const gin::MeshData default_mesh = {
	{
		0.5f,  0.5f,	// top right
		0.5f, -0.5f,	// bottom right
		-0.5f, -0.5f,	// bottom left
		-0.5f,  0.5f	// top left
	},
	{
		1.f, 1.f,
		1.f, 0.f,
		0.f, 0.f,
		0.f, 1.f
	},
	{
		0, 1, 2,
		2, 1, 3
	}
};