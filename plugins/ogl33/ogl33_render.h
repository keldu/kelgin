#pragma once

#include "render/render.h"

namespace gin {
class Ogl33RenderWorld : public RenderWorld {

};

class Ogl33Render : public Render {
public:
	Own<RenderWorld> createWorld() override;
};
}
