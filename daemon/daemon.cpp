#include "graphics.h"

#include <iostream>

int main(int argc, char **argv) {
	using namespace gin;

	ErrorOr<AsyncIoContext> err_async = setupAsyncIo();
	if (err_async.isError()) {
		std::cerr << "Couldn't setup AsyncIoContext" << std::endl;
		return -1;
	}
	AsyncIoContext &async = err_async.value();
	WaitScope wait_scope{async.event_loop};

	GraphicsService service{Graphics{loadAllRenderPluginsIn("bin/plugins/")}};

	async.event_port.onSignal(Signal::Terminate)
		.then([&service]() {
			service.stop();
			std::cout << "Shutdown requested" << std::endl;
		})
		.detach();

	return service.rcRun(async, wait_scope);
}