#pragma once

#include <kelgin/common.h>

#include <vector>

namespace gin {
struct DaemonConfig {
	struct ListenAddress {
		std::string address;
		uint16_t port_hint = 0;
	};

	std::vector<ListenAddress> addresses;
};

Own<Config> loadConfig();
}