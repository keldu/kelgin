#pragma once

#include <kelgin/common.h>
#include <kelgin/error.h>

#include <filesystem>
#include <string>
#include <memory>

namespace gin {
class DynamicLibrary {
private:
	class Impl;
	gin::Own<Impl> impl;

	friend class ErrorOr<DynamicLibrary> loadDynamicLibrary(const std::filesystem::path&);
	DynamicLibrary(void* handle);
public:
	~DynamicLibrary();

	DynamicLibrary(const DynamicLibrary&) = delete;
	DynamicLibrary& operator=(const DynamicLibrary&) = delete;

	DynamicLibrary(DynamicLibrary&&);
	DynamicLibrary& operator=(DynamicLibrary&&);

	void* symbol(const std::string& sym);
};

ErrorOr<DynamicLibrary> loadDynamicLibrary(const std::filesystem::path& path);
}