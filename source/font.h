#pragma once

#include "./render/render.h"

#include <kelgin/common.h>

#include <map>
#include <set>
#include <string>

namespace gin {
class Glyph {
private:
public:
};

class Font {
public:
	class Page {
	public:
		struct Row {
			size_t height;
			size_t shift;
			size_t width;
		};
		
		class Info {
		private:
			std::vector<Row> rows;
		};
	private:
		Image image;
		Info info;
	public:
		Page(Image&& image, Info&& info);
	};
private:
	std::map<uint32_t, Page> pages;
protected:
public:
	virtual ~Font() = default;

	virtual Our<Page> generatePage(uint32_t size, const std::set<uint32_t>& code_points) = 0;
};

class FontFactory {
public:
	virtual ~FontFactory() = default;

	virtual gin::Own<Font> createFont(const std::string&) = 0;
};

gin::Own<FontFactory> createFontFactory();

}