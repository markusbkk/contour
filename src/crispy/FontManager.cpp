/**
 * This file is part of the "libterminal" project
 *   Copyright (c) 2019 Christian Parpart <christian@parpart.family>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <crispy/FontManager.h>
#include <crispy/times.h>
#include <crispy/algorithm.h>

#include <fmt/format.h>

#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>

#include <stdexcept>
#include <map>
#include <iostream>

#include <cctype>

#if defined(__linux__) || defined(__APPLE__)
#define HAVE_FONTCONFIG
#endif

#if defined(HAVE_FONTCONFIG)
#include <fontconfig/fontconfig.h>
#endif

using namespace std;

namespace crispy::text {

namespace {
    static string freetypeErrorString(FT_Error _errorCode)
    {
        #undef __FTERRORS_H__
        #define FT_ERROR_START_LIST     switch (_errorCode) {
        #define FT_ERRORDEF(e, v, s)    case e: return s;
        #define FT_ERROR_END_LIST       }
        #include FT_ERRORS_H
        return "(Unknown error)";
    }

    static bool endsWithIgnoreCase(string const& _text, string const& _suffix)
    {
        if (_text.size() < _suffix.size())
            return false;

        auto const* text = &_text[_text.size() - _suffix.size()];
        for (size_t i = 0; i < _suffix.size(); ++i)
            if (tolower(text[i]) != tolower(_suffix[i]))
                return false;

        return true;
    }

    static vector<string> getFontFilePaths([[maybe_unused]] string const& _fontPattern)
    {
        if (endsWithIgnoreCase(_fontPattern, ".ttf") || endsWithIgnoreCase(_fontPattern, ".otf"))
            return {_fontPattern};

        #if defined(HAVE_FONTCONFIG)
        string const& pattern = _fontPattern; // TODO: append bold/italic if needed

        FcConfig* fcConfig = FcInitLoadConfigAndFonts();
        FcPattern* fcPattern = FcNameParse((FcChar8 const*) pattern.c_str());

        FcDefaultSubstitute(fcPattern);
        FcConfigSubstitute(fcConfig, fcPattern, FcMatchPattern);

        FcResult fcResult = FcResultNoMatch;

        vector<string> paths;

        // find exact match - TODO: this isn't working. Emoji Noto font is still first! FIXME
        FcPattern* matchedPattern = FcFontMatch(fcConfig, fcPattern, &fcResult);
        optional<string> primaryFontPath;
        if (fcResult == FcResultMatch && matchedPattern)
        {
            char* resultPath{};
            if (FcPatternGetString(matchedPattern, FC_FILE, 0, (FcChar8**) &resultPath) == FcResultMatch)
            {
                primaryFontPath = resultPath;
                cout << fmt::format("exact match found: {}\n", *primaryFontPath);
                paths.emplace_back(*primaryFontPath);
            }
            FcPatternDestroy(matchedPattern);
        }
        if (paths.empty())
            cout << fmt::format("No exact match found!\n");

        // find fallback fonts
        FcCharSet* fcCharSet = nullptr;
        FcFontSet* fcFontSet = FcFontSort(fcConfig, fcPattern, /*trim*/FcTrue, &fcCharSet, &fcResult);
        for (int i = 0; i < fcFontSet->nfont; ++i)
        {
            FcChar8* fcFile = nullptr;
            if (FcPatternGetString(fcFontSet->fonts[i], FC_FILE, 0, &fcFile) == FcResultMatch)
            {
                // FcBool fcColor = false;
                // FcPatternGetBool(fcFontSet->fonts[i], FC_COLOR, 0, &fcColor);
                if (fcFile && primaryFontPath && *primaryFontPath != (char const*) fcFile)
                    paths.emplace_back((char const*) fcFile);
            }
        }
        FcFontSetDestroy(fcFontSet);
        FcCharSetDestroy(fcCharSet);

        FcPatternDestroy(fcPattern);
        FcConfigDestroy(fcConfig);
        return paths;
        #endif

        #if defined(_WIN32)
        // TODO: Read https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-enumfontfamiliesexa
        // This is pretty damn hard coded, and to be properly implemented once the other font related code's done,
        // *OR* being completely deleted when FontConfig's windows build fix released and available via vcpkg.
        if (_fontPattern.find("bold italic") != string::npos)
            return {"C:\\Windows\\Fonts\\consolaz.ttf"};
        else if (_fontPattern.find("italic") != string::npos)
            return {"C:\\Windows\\Fonts\\consolai.ttf"};
        else if (_fontPattern.find("bold") != string::npos)
            return {"C:\\Windows\\Fonts\\consolab.ttf"};
        else
            return {"C:\\Windows\\Fonts\\consola.ttf"};
        #endif
    }

    constexpr bool glyphMissing(GlyphPosition const& _gp) noexcept
    {
        return _gp.glyphIndex == 0;
    }

    unsigned computeMaxAdvance(FT_Face _face)
    {
        if (FT_Load_Char(_face, 'M', FT_LOAD_BITMAP_METRICS_ONLY) == FT_Err_Ok)
            return _face->glyph->advance.x >> 6;

        unsigned long long maxAdvance = 0;
        unsigned count = 0;
        for (unsigned glyphIndex = 0; glyphIndex < _face->num_glyphs; ++glyphIndex)
        {
            if (FT_Load_Glyph(_face, glyphIndex, FT_LOAD_BITMAP_METRICS_ONLY) == FT_Err_Ok)// FT_LOAD_BITMAP_METRICS_ONLY);
            {
                maxAdvance += _face->glyph->advance.x >> 6;
                count++;
            }
        }
        return maxAdvance / count;
    }
}

FontManager::FontManager() :
    ft_{},
    fonts_{}
{
    if (FT_Init_FreeType(&ft_))
        throw runtime_error{ "Failed to initialize FreeType." };
}

FontManager::~FontManager()
{
    fonts_.clear();
    FT_Done_FreeType(ft_);
}

FontList FontManager::load(string const& _fontPattern, unsigned _fontSize)
{
    vector<string> const filePaths = getFontFilePaths(_fontPattern);

    Font& primaryFont = loadFromFilePath(filePaths.front(), _fontSize);
    FontFallbackList fallbackList;
    for (size_t i = 1; i < filePaths.size(); ++i)
        fallbackList.push_back(loadFromFilePath(filePaths[i], _fontSize));

    cout << fmt::format("FontManager.load : {} size:{}\n", _fontPattern, _fontSize);
    cout << fmt::format("    primary font : {}\n", primaryFont.filePath());
    cout << fmt::format("  fallback count : {}\n", fallbackList.size());
    for (size_t i = 1; i < filePaths.size(); ++i)
        cout << fmt::format("{:>16} : {}\n", i, filePaths[i].c_str());

    return {primaryFont, fallbackList};
}

Font& FontManager::loadFromFilePath(std::string const& _path, unsigned _fontSize)
{
    if (auto k = fonts_.find(_path); k != fonts_.end())
        return k->second;
    else
        return fonts_.emplace(make_pair(_path, Font(ft_, _path, _fontSize))).first->second;
}

void Font::setFontSize(unsigned int _fontSize)
{
    if (fontSize_ != _fontSize)
    {
        if (hasColor())
        {
            // FIXME i think this one can be omitted?
            FT_Error const ec = FT_Select_Size(face_, 0);
            if (ec != FT_Err_Ok)
                throw runtime_error{fmt::format("Failed to FT_Select_Size. {}", freetypeErrorString(ec))};
        }
        else
        {
            FT_Error const ec = FT_Set_Pixel_Sizes(face_, 0, static_cast<FT_UInt>(_fontSize));
            if (ec)
                throw runtime_error{ string{"Failed to set font pixel size. "} + freetypeErrorString(ec) };
        }

        fontSize_ = _fontSize;

        // update bitmap width/height
        if (FT_IS_SCALABLE(face_))
        {
            bitmapWidth_ = FT_MulFix(face_->bbox.xMax - face_->bbox.xMin, face_->size->metrics.x_scale) >> 6;
            bitmapHeight_ = FT_MulFix(face_->bbox.yMax - face_->bbox.yMin, face_->size->metrics.y_scale) >> 6;
        }
        else
        {
            bitmapWidth_ = (face_->available_sizes[0].width);
            bitmapHeight_ = (face_->available_sizes[0].height);
        }

        maxAdvance_ = computeMaxAdvance(face_);

        loadGlyphByIndex(0);
    }
}

// -------------------------------------------------------------------------------------------------------

Font::Font(FT_Library _ft, std::string _fontPath, unsigned int _fontSize) :
    ft_{ _ft },
    face_{},
    fontSize_{ 0 },
    filePath_{ move(_fontPath) },
    hashCode_{ hash<string>{}(filePath_)}
{
    if (FT_New_Face(ft_, filePath_.c_str(), 0, &face_))
        throw runtime_error{ "Failed to load font." };

    FT_Error ec = FT_Select_Charmap(face_, FT_ENCODING_UNICODE);
    if (ec)
        throw runtime_error{ string{"Failed to set charmap. "} + freetypeErrorString(ec) };

    setFontSize(_fontSize);

    loadGlyphByIndex(0);
    // XXX Woot, needed in order to retrieve maxAdvance()'s field,
    // as max_advance metric seems to be broken on at least FiraCode (Regular),
    // which is twice as large as it should be, but taking
    // a regular face's advance value works.
}

Font::Font(Font&& v) noexcept :
    ft_{ v.ft_ },
    face_{ v.face_ },
    fontSize_{ v.fontSize_ },
    bitmapWidth_{ v.bitmapWidth_ },
    bitmapHeight_{ v.bitmapHeight_ },
    maxAdvance_{ v.maxAdvance_ },
    filePath_{ move(v.filePath_) },
    hashCode_{ v.hashCode_ }
{
    v.ft_ = nullptr;
    v.face_ = nullptr;
    v.fontSize_ = 0;
    v.bitmapWidth_ = 0;
    v.bitmapHeight_ = 0;
    v.filePath_ = {};
    v.hashCode_ = 0;
}

Font& Font::operator=(Font&& v) noexcept
{
    // TODO: free current resources, if any

    ft_ = v.ft_;
    face_ = v.face_;
    fontSize_ = v.fontSize_;
    maxAdvance_ = v.maxAdvance_;
    bitmapWidth_ = v.bitmapWidth_;
    bitmapHeight_ = v.bitmapHeight_;
    filePath_ = move(v.filePath_);
    hashCode_ = v.hashCode_;

    v.ft_ = nullptr;
    v.face_ = nullptr;
    v.fontSize_ = 0;
    v.bitmapWidth_ = 0;
    v.bitmapHeight_ = 0;
    v.filePath_ = {};
    v.hashCode_ = 0;

    return *this;
}

Font::~Font()
{
    if (face_)
        FT_Done_Face(face_);
}

GlyphBitmap Font::loadGlyphByIndex(unsigned int _glyphIndex)
{
    FT_Int32 flags = FT_LOAD_DEFAULT;
    if (FT_HAS_COLOR(face_))
        flags |= FT_LOAD_COLOR;

    FT_Error ec = FT_Load_Glyph(face_, _glyphIndex, flags);
    if (ec != FT_Err_Ok)
        throw runtime_error{ string{"Error loading glyph. "} + freetypeErrorString(ec) };

    // NB: colored fonts are bitmap fonts, they do not need rendering
    if (!FT_HAS_COLOR(face_))
        if (FT_Render_Glyph(face_->glyph, FT_RENDER_MODE_NORMAL) != FT_Err_Ok)
            return GlyphBitmap{};

    auto const width = face_->glyph->bitmap.width;
    auto const height = face_->glyph->bitmap.rows;
    auto const buffer = face_->glyph->bitmap.buffer;

    vector<uint8_t> bitmap;
    if (!hasColor())
    {
        auto const pitch = face_->glyph->bitmap.pitch;
        bitmap.resize(height * width);
        for (unsigned i = 0; i < height; ++i)
            for (unsigned j = 0; j < face_->glyph->bitmap.width; ++j)
                bitmap[i * face_->glyph->bitmap.width + j] = buffer[i * pitch + j];
    }
    else
    {
        bitmap.resize(height * width * 4);
        copy(
            buffer,
            buffer + height * width * 4,
            bitmap.begin()
        );
    }

    return GlyphBitmap{
        width,
        height,
        move(bitmap)
    };
}

// ====================================================================================================

TextShaper::TextShaper(Font& _font, FontFallbackList const& _fallbackList) :
    font_{ _font },
    fallbackList_{ _fallbackList }
{
    hb_buf_ = hb_buffer_create();
}

TextShaper::~TextShaper()
{
    clearCache();
    hb_buffer_destroy(hb_buf_);
}

void TextShaper::setFont(Font& _font, FontFallbackList const& _fallbackList)
{
    font_ = _font;
    fallbackList_ = _fallbackList;
    clearCache();
}

void TextShaper::setFontSize(unsigned _fontSize)
{
    font_.get().setFontSize(_fontSize);

    for (reference_wrapper<Font>& fallback : fallbackList_)
        fallback.get().setFontSize(_fontSize);

    clearCache();
}

GlyphPositionList const* TextShaper::shape(CodepointSequence const& _codes)
{
    if (auto i = cache_.find(_codes); i != cache_.end())
        return &i->second;

    GlyphPositionList result;
    if (shape(_codes, font_.get(), ref(result)))
        return &(cache_[_codes] = move(result));

    size_t i = 1;
    for (reference_wrapper<Font>& fallback : fallbackList_)
    {
#if 1 // FIXME: ugly hack to make sure our fallback is color fonts (emojis)
        if (!fallback.get().hasColor())
            continue;
#endif

        if (shape(_codes, fallback.get(), ref(result)))
            return &(cache_[_codes] = move(result));
        ++i;
    }

    string joinedCodes;
    for (Codepoint code : _codes)
    {
        if (!joinedCodes.empty())
            joinedCodes += " ";
        joinedCodes += fmt::format("{:<6x}", unsigned(code.value));
    }
    cout << fmt::format("Shaping failed for {} codepoints: {}\n", _codes.size(), joinedCodes);

    shape(_codes, font_.get(), ref(result));
    replaceMissingGlyphs(result);
    return &(cache_[_codes] = move(result));
}

void TextShaper::clearCache()
{
    cache_.clear();
    cache2_.clear();

    for ([[maybe_unused]] auto [_, hbf] : hb_fonts_)
        hb_font_destroy(hbf);

    hb_fonts_.clear();
}

bool TextShaper::shape(CodepointSequence const& _codes, Font& _font, reference<GlyphPositionList> _result)
{
    hb_buffer_clear_contents(hb_buf_);

    for (Codepoint const& codepoint : _codes)
        hb_buffer_add(hb_buf_, codepoint.value, codepoint.cluster);

    hb_buffer_set_content_type(hb_buf_, HB_BUFFER_CONTENT_TYPE_UNICODE);
    hb_buffer_set_direction(hb_buf_, HB_DIRECTION_LTR);
    hb_buffer_set_script(hb_buf_, HB_SCRIPT_COMMON);
    hb_buffer_set_language(hb_buf_, hb_language_get_default());
    hb_buffer_guess_segment_properties(hb_buf_);

    hb_font_t* hb_font = nullptr;
    if (auto i = hb_fonts_.find(&_font); i != hb_fonts_.end())
        hb_font = i->second;
    else
    {
        hb_font = hb_ft_font_create_referenced(_font);
        hb_fonts_[&_font] = hb_font;
    }

    hb_shape(hb_font, hb_buf_, nullptr, 0);

    hb_buffer_normalize_glyphs(hb_buf_);

    unsigned const glyphCount = hb_buffer_get_length(hb_buf_);
    hb_glyph_info_t const* info = hb_buffer_get_glyph_infos(hb_buf_, nullptr);
    hb_glyph_position_t const* pos = hb_buffer_get_glyph_positions(hb_buf_, nullptr);

    _result.get().clear();
    _result.get().reserve(glyphCount);

    unsigned int cx = 0;
    unsigned int cy = 0;
    for (unsigned const i : times(glyphCount))
    {
        // TODO: maybe right in here, apply incremented cx/xy only if cluster number has changed?
        _result.get().emplace_back(GlyphPosition{
            _font,
            cx + (pos[i].x_offset >> 6),
            cy + (pos[i].y_offset >> 6),
            info[i].codepoint,
            info[i].cluster
        });

        if (pos[i].x_advance)
            cx += font_.get().maxAdvance();

        cy += pos[i].y_advance >> 6;
    }

    return not any_of(_result.get(), glyphMissing);
}

void TextShaper::replaceMissingGlyphs(GlyphPositionList& _result)
{
    auto constexpr missingGlyphId = 0xFFFDu;
    auto const missingGlyph = FT_Get_Char_Index(font_.get(), missingGlyphId);

    if (missingGlyph)
    {
        for (auto i : times(_result.size()))
            if (glyphMissing(_result[i]))
                _result[i].glyphIndex = missingGlyph;
    }
}

} // end namespace
