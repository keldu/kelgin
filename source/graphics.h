#pragma once

#include "service.h"

#include <kelgin/common.h>

#include <unordered_map>

namespace gin {
/*
* Service since it is the core class running the main
* aspects
*/
class GraphicsService final : public Service {
private:
	int return_code = 0;
	bool running = true;
public:	
	GraphicsService();

	int rcRun(AsyncIoContext& async);

	void run(AsyncIoContext& async) override;
	void stop() override;
};
}