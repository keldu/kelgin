#pragma once

#include "./render/render.h"

#include <kelgin/common.h>

#include <map>
#include <string>

namespace gin {
class Glyph {

};

class Font {
private:
	struct Row {
		size_t height;
		size_t shift;
		size_t width;
	};

	struct Page {
		std::vector<Row> rows;
	};

	std::map<uint32_t, Page> pages;
public:
	virtual ~Font() = default;
};

class FontFactory {
public:
	virtual ~FontFactory() = default;

	virtual gin::Own<Font> createFont(const std::string&) = 0;
};

gin::Own<FontFactory> createFontFactory();

}