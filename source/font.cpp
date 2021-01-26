#include "font.h"

#include <cassert>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_BITMAP_H
#include FT_STROKER_H

namespace gin {
struct FreeTypeContext {
	FT_Library library = nullptr;
	FT_Stroker stroker = nullptr;

	FreeTypeContext(FT_Library library, FT_Stroker stroker):
		library{library},stroker{stroker}{}

	FreeTypeContext(const FreeTypeContext&) = delete;
	FreeTypeContext& operator=(const FreeTypeContext&) = delete;

	~FreeTypeContext(){
		assert(stroker);
		if(stroker){
			FT_Stroker_Done(stroker);
		}

		assert(library);
		if(library){
			FT_Done_FreeType(library);
		}
	}
};

class FreeTypeFont final : public Font {
private:
	Our<FreeTypeContext> context;
	FT_Face face;
public:
	FreeTypeFont(Our<FreeTypeContext> ctx, FT_Face face):context{std::move(ctx)}, face{face}{}
	~FreeTypeFont(){
		assert(face);
		if(face){
			FT_Done_Face(face);
		}
	}
};

class FreeTypeFontFactory final : public FontFactory {
private:
	gin::Our<FreeTypeContext> ctx;
public:
	FreeTypeFontFactory(FT_Library lib, FT_Stroker stroker):
		ctx{gin::share<FreeTypeContext>(lib, stroker)}{}

	gin::Own<Font> createFont(const std::string& path) override {
		FT_Face face;
		if(FT_New_Face(ctx->library, path.c_str(), 0, &face) != 0){
			return nullptr;
		}

		if(FT_Select_Charmap(face, FT_ENCODING_UNICODE) != 0){
			FT_Done_Face(face);
			return nullptr;
		}

		return gin::heap<FreeTypeFont>(ctx, face);
	}
};

gin::Own<FontFactory> createFontFactory() {
	FT_Library library;

	if(FT_Init_FreeType(&library) != 0){
		return nullptr;
	}

	FT_Stroker stroker;
	if(FT_Stroker_New(library, &stroker) != 0){
		FT_Done_FreeType(library);
		return nullptr;
	}

	return gin::heap<FreeTypeFontFactory>(library, stroker);
}
}