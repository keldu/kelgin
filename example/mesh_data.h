#pragma once

#include "render/render.h"

const gin::MeshData default_mesh = {
	{{
		 {1.f, 1.f}, {1.f, 0.f}, // top right
	 },
	 {
		 {1.f, -1.f}, {1.f, 1.f}, // bottom right
	 },
	 {
		 {-1.f, -1.f}, {0.f, 1.f}, // bottom left
	 },
	 {
		 {-1.f, 1.f}, {0.f, 0.f}, // top left
	 }},
	{2, 1, 0, 2, 0, 3}};

const gin::MeshData bg_mesh = {{
								   {{40.f, 40.f}, {1.f, 0.f}},
								   {{40.f, -40.f}, {1.f, 1.f}},
								   {{-40.f, -40.f}, {0.f, 1.f}},
								   {{-40.f, 40.f}, {0.f, 0.f}},
							   },
							   {2, 1, 0, 2, 0, 3}};
