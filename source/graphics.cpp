#include "graphics.h"

#include <kelgin/async.h>
#include <kelgin/io.h>

namespace gin {
GraphicsService::GraphicsService()
{}

void GraphicsService::run(AsyncIoContext& async){
	while(running){
		async.wait_scope.wait(std::chrono::seconds{1});
	}
}

int GraphicsService::rcRun(AsyncIoContext& async){
	run(async);
	return return_code;
}

void GraphicsService::stop(){
	running = false;
}
}