#pragma once

#include <kelgin/common.h>

#include <filesystem>
#include <string>

namespace gin {
class DynamicLibrary {
private:
	class Impl;
	gin::Own<Impl> impl;

	friend class ErrorOr<DynamicLibrary> loadDynamicLibrary(const std::filesystem::path&);
	DynamicLibrary(void* handle);
public:
	~DynamicLibrary();

	DynamicLibrary(DynamicLibrary&&) = default;
	DynamicLibrary& operator=(DynamicLibrary&&) = default;

	void* symbol(const std::string& sym);
};

ErrorOr<DynamicLibrary> loadDynamicLibrary(const std::filesystem::path& path);
}