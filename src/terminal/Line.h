/**
 * This file is part of the "libterminal" project
 *   Copyright (c) 2019-2020 Christian Parpart <christian@parpart.family>
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
#pragma once

#include <terminal/CellUtil.h>
#include <terminal/GraphicsAttributes.h>
#include <terminal/Hyperlink.h>
#include <terminal/primitives.h>

#include <crispy/BufferObject.h>
#include <crispy/Comparison.h>
#include <crispy/assert.h>

#include <unicode/convert.h>

#include <gsl/span>
#include <gsl/span_ext>

#include <iterator>
#include <sstream>
#include <string>
#include <vector>

namespace terminal
{

enum class LineFlags : uint8_t
{
    None = 0x0000,
    Wrappable = 0x0001,
    Wrapped = 0x0002,
    Marked = 0x0004,
    // TODO: DoubleWidth  = 0x0010,
    // TODO: DoubleHeight = 0x0020,
};

// clang-format off
template <typename, bool> struct OptionalProperty;
template <typename T> struct OptionalProperty<T, false> {};
template <typename T> struct OptionalProperty<T, true> { T value; };
// clang-format on

/**
 * Line storage with call columns sharing the same SGR attributes.
 */
struct TrivialLineBuffer
{
    ColumnCount displayWidth;
    GraphicsAttributes textAttributes;
    GraphicsAttributes fillAttributes = textAttributes;
    HyperlinkId hyperlink {};

    ColumnCount usedColumns {};
    crispy::BufferFragment text {};

    void reset(GraphicsAttributes _attributes) noexcept
    {
        textAttributes = _attributes;
        fillAttributes = _attributes;
        hyperlink = {};
        usedColumns = {};
        text.reset();
    }
};

template <typename Cell>
using InflatedLineBuffer = std::vector<Cell>;

/// Unpacks a TrivialLineBuffer into an InflatedLineBuffer<Cell>.
template <typename Cell>
InflatedLineBuffer<Cell> inflate(TrivialLineBuffer const& input);

template <typename Cell>
using LineStorage = std::variant<TrivialLineBuffer, InflatedLineBuffer<Cell>>;

/**
 * Line<Cell> API.
 *
 * TODO: Use custom allocator for ensuring cache locality of Cells to sibling lines.
 * TODO: Make the line optimization work.
 */
template <typename Cell>
class Line
{
  public:
    Line() = default;
    Line(Line const&) = default;
    Line(Line&&) noexcept = default;
    Line& operator=(Line const&) = default;
    Line& operator=(Line&&) noexcept = default;

    using TrivialBuffer = TrivialLineBuffer;
    using InflatedBuffer = InflatedLineBuffer<Cell>;
    using Storage = LineStorage<Cell>;
    using value_type = Cell;
    using iterator = typename InflatedBuffer::iterator;
    using reverse_iterator = typename InflatedBuffer::reverse_iterator;
    using const_iterator = typename InflatedBuffer::const_iterator;

    Line(LineFlags _flags, TrivialBuffer _buffer):
        storage_ { std::move(_buffer) }, flags_ { static_cast<unsigned>(_flags) }
    {
    }

    Line(LineFlags _flags, InflatedBuffer _buffer):
        storage_ { std::move(_buffer) }, flags_ { static_cast<unsigned>(_flags) }
    {
    }

    void reset(LineFlags _flags, GraphicsAttributes _attributes) noexcept
    {
        flags_ = static_cast<unsigned>(_flags);
        if (isTrivialBuffer())
            trivialBuffer().reset(_attributes);
        else
            setBuffer(TrivialBuffer { ColumnCount::cast_from(inflatedBuffer().size()), _attributes });
    }

    void reset(LineFlags _flags, GraphicsAttributes _attributes, ColumnCount count) noexcept
    {
        flags_ = static_cast<unsigned>(_flags);
        setBuffer(TrivialBuffer { count, _attributes });
    }

    void fill(LineFlags _flags,
              GraphicsAttributes const& _attributes,
              char32_t _codepoint,
              uint8_t _width) noexcept
    {
        if (_codepoint == 0)
            reset(_flags, _attributes);
        else
        {
            flags_ = static_cast<unsigned>(_flags);
            for (Cell& cell: inflatedBuffer())
            {
                cell.reset();
                cell.write(_attributes, _codepoint, _width);
            }
        }
    }

    /// Tests if all cells are empty.
    [[nodiscard]] bool empty() const noexcept
    {
        if (isTrivialBuffer())
            return trivialBuffer().text.empty();

        for (auto const& cell: inflatedBuffer())
            if (!cell.empty())
                return false;
        return true;
    }

    /**
     * Fills this line with the given content.
     *
     * @p _start offset into this line of the first charater
     * @p _sgr graphics rendition for the line starting at @c _start until the end
     * @p _ascii the US-ASCII characters to fill with
     */
    void fill(ColumnOffset _start, GraphicsAttributes const& _sgr, std::string_view _ascii)
    {
        auto& buffer = inflatedBuffer();

        assert(unbox<size_t>(_start) + _ascii.size() <= buffer.size());

        auto constexpr ASCII_Width = 1;
        auto const* s = _ascii.data();

        Cell* i = &buffer[unbox<size_t>(_start)];
        Cell* e = i + _ascii.size();
        while (i != e)
            (i++)->write(_sgr, static_cast<char32_t>(*s++), ASCII_Width);

        auto const e2 = buffer.data() + buffer.size();
        while (i != e2)
            (i++)->reset();
    }

    [[nodiscard]] ColumnCount size() const noexcept
    {
        if (isTrivialBuffer())
            return trivialBuffer().displayWidth;
        else
            return ColumnCount::cast_from(inflatedBuffer().size());
    }

    void resize(ColumnCount _count);

    gsl::span<Cell const> trim_blank_right() const noexcept;

    gsl::span<Cell const> cells() const noexcept { return inflatedBuffer(); }

    gsl::span<Cell> useRange(ColumnOffset _start, ColumnCount _count) noexcept
    {
#if defined(__clang__) && __clang_major__ <= 11
        auto const bufferSpan = gsl::span(inflatedBuffer());
        return bufferSpan.subspan(unbox<size_t>(_start), unbox<size_t>(_count));
#else
        // Clang <= 11 cannot deal with this (e.g. FreeBSD 13 defaults to Clang 11).
        return gsl::span(inflatedBuffer()).subspan(unbox<size_t>(_start), unbox<size_t>(_count));
#endif
    }

    [[nodiscard]] Cell& useCellAt(ColumnOffset _column) noexcept
    {
        Require(ColumnOffset(0) <= _column);
        Require(_column <= ColumnOffset::cast_from(size())); // Allow off-by-one for sentinel.
        return inflatedBuffer()[unbox<size_t>(_column)];
    }

    [[nodiscard]] uint8_t cellEmptyAt(ColumnOffset column) const noexcept
    {
        if (isTrivialBuffer())
        {
            Require(ColumnOffset(0) <= column);
            Require(column < ColumnOffset::cast_from(size()));
            return unbox<size_t>(column) >= trivialBuffer().text.size()
                   || trivialBuffer().text[column.as<size_t>()] == 0x20;
        }
        return inflatedBuffer().at(unbox<size_t>(column)).empty();
    }

    [[nodiscard]] uint8_t cellWidthAt(ColumnOffset column) const noexcept
    {
        if (isTrivialBuffer())
        {
            Require(ColumnOffset(0) <= column);
            Require(column < ColumnOffset::cast_from(size()));
            return 1; // TODO: When trivial line is to support Unicode, this should be adapted here.
        }
        return inflatedBuffer().at(unbox<size_t>(column)).width();
    }

    [[nodiscard]] LineFlags flags() const noexcept
    {
        return static_cast<LineFlags>(flags_);
    }

    [[nodiscard]] bool marked() const noexcept
    {
        return isFlagEnabled(LineFlags::Marked);
    }
    void setMarked(bool _enable)
    {
        setFlag(LineFlags::Marked, _enable);
    }

    [[nodiscard]] bool wrapped() const noexcept
    {
        return isFlagEnabled(LineFlags::Wrapped);
    }
    void setWrapped(bool _enable)
    {
        setFlag(LineFlags::Wrapped, _enable);
    }

    [[nodiscard]] bool wrappable() const noexcept
    {
        return isFlagEnabled(LineFlags::Wrappable);
    }
    void setWrappable(bool _enable)
    {
        setFlag(LineFlags::Wrappable, _enable);
    }

    [[nodiscard]] LineFlags wrappableFlag() const noexcept
    {
        return wrappable() ? LineFlags::Wrappable : LineFlags::None;
    }
    [[nodiscard]] LineFlags wrappedFlag() const noexcept
    {
        return marked() ? LineFlags::Wrapped : LineFlags::None;
    }
    [[nodiscard]] LineFlags markedFlag() const noexcept
    {
        return marked() ? LineFlags::Marked : LineFlags::None;
    }

    [[nodiscard]] LineFlags inheritableFlags() const noexcept
    {
        auto constexpr Inheritables = unsigned(LineFlags::Wrappable) | unsigned(LineFlags::Marked);
        return static_cast<LineFlags>(flags_ & Inheritables);
    }

    void setFlag(LineFlags _flag, bool _enable) noexcept
    {
        if (_enable)
            flags_ |= static_cast<unsigned>(_flag);
        else
            flags_ &= ~static_cast<unsigned>(_flag);
    }

    [[nodiscard]] bool isFlagEnabled(LineFlags _flag) const noexcept
    {
        return (flags_ & static_cast<unsigned>(_flag)) != 0;
    }

    [[nodiscard]] InflatedBuffer reflow(ColumnCount _newColumnCount);
    [[nodiscard]] std::string toUtf8() const;
    [[nodiscard]] std::string toUtf8Trimmed() const;

    // Returns a reference to this mutable grid-line buffer.
    //
    // If this line has been stored in an optimized state, then
    // the line will be first unpacked into a vector of grid cells.
    InflatedBuffer& inflatedBuffer();
    InflatedBuffer const& inflatedBuffer() const;

    [[nodiscard]] TrivialBuffer& trivialBuffer() noexcept
    {
        return std::get<TrivialBuffer>(storage_);
    }
    [[nodiscard]] TrivialBuffer const& trivialBuffer() const noexcept
    {
        return std::get<TrivialBuffer>(storage_);
    }

    [[nodiscard]] bool isTrivialBuffer() const noexcept
    {
        return std::holds_alternative<TrivialBuffer>(storage_);
    }
    [[nodiscard]] bool isInflatedBuffer() const noexcept
    {
        return !std::holds_alternative<TrivialBuffer>(storage_);
    }

    void setBuffer(Storage buffer) noexcept
    {
        storage_ = std::move(buffer);
    }

    // Tests if the given text can be matched in this line at the exact given start column.
    [[nodiscard]] bool matchTextAt(std::u32string_view text, ColumnOffset startColumn) const noexcept
    {
        if (isTrivialBuffer())
        {
            auto const u8Text = unicode::convert_to<char>(text);
            TrivialBuffer const& buffer = trivialBuffer();
            if (!buffer.usedColumns)
                return false;
            auto const column = std::min(startColumn, boxed_cast<ColumnOffset>(buffer.usedColumns - 1));
            if (text.size() > column.value - buffer.usedColumns.value)
                return false;
            auto const resultIndex = buffer.text.view()
                                         .substr(unbox<size_t>(column))
                                         .find(std::string_view(u8Text), unbox<size_t>(column));
            return resultIndex == 0;
        }
        else
        {
            auto const u8Text = unicode::convert_to<char>(text);
            InflatedBuffer const& cells = inflatedBuffer();
            if (text.size() > size().value - startColumn.value)
                return false;
            auto const baseColumn = unbox<size_t>(startColumn);
            size_t i = 0;
            while (i < text.size())
            {
                if (!CellUtil::beginsWith(text.substr(i), cells[baseColumn + i]))
                    return false;
                ++i;
            }
            return i == text.size();
        }
    }

    // Searches for the given text in the line at a specific column.
    // Returns SearchResult with column set to location of beginning of found location or std::nullopt and
    // remainingText set to 0
    // If a partial match is found at the end of line then returns SearchResult::column
    // set to std::nullopt and remainingText set to number of character which were not matched.
    [[nodiscard]] SearchResult search(std::u32string_view text, ColumnOffset startColumn) const noexcept
    {
        if (isTrivialBuffer())
        {
            auto const u8Text = unicode::convert_to<char>(text);
            TrivialBuffer const& buffer = trivialBuffer();
            if (!buffer.usedColumns)
                return SearchResult {};
            auto const column = std::min(startColumn, boxed_cast<ColumnOffset>(buffer.usedColumns - 1));
            auto const resultIndex = buffer.text.view().find(std::string_view(u8Text), unbox<size_t>(column));
            if (resultIndex != std::string_view::npos)
                return SearchResult { ColumnOffset::cast_from(resultIndex) };
            else
                return SearchResult {}; // Not found, so stay with initial column as result.
        }
        else
        {
            InflatedBuffer const& buffer = inflatedBuffer();
            if (buffer.size() < text.size())
                return SearchResult {}; // not found: line is smaller than search term

            auto baseColumn = startColumn;
            auto rightMostSearchPosition = ColumnOffset::cast_from(buffer.size());
            auto const originalSize = text.size();
            while (baseColumn < rightMostSearchPosition)
            {
                if (buffer.size() - baseColumn.value < text.size())
                {
                    text.remove_suffix(text.size() - (size().value - baseColumn.value));
                    if (matchTextAt(text, baseColumn))
                        return SearchResult { std::nullopt,
                                              int(originalSize - (buffer.size() - baseColumn.value)) };
                }
                else if (matchTextAt(text, baseColumn))
                    return SearchResult { baseColumn };
                baseColumn++;
            }

            return SearchResult {}; // Not found, so stay with initial column as result.
        }
    }

    // Searches for the given text in the line at a specific column from right to left.
    // Returns SearchResult with column set to location of beginning of found location or std::nullopt and
    // remainingText set to 0
    // If a partial match is found at the start of line then returns SearchResult::column
    // set to std::nullopt and remainingText set to number of character which were not matched.
    [[nodiscard]] SearchResult searchReverse(std::u32string_view text,
                                             ColumnOffset startColumn) const noexcept
    {
        if (isTrivialBuffer())
        {
            auto const u8Text = unicode::convert_to<char>(text);
            TrivialBuffer const& buffer = trivialBuffer();
            if (!buffer.usedColumns)
                return SearchResult {};
            auto const column = std::min(startColumn, boxed_cast<ColumnOffset>(buffer.usedColumns - 1));
            auto const resultIndex =
                buffer.text.view().rfind(std::string_view(u8Text), unbox<size_t>(column));
            if (resultIndex != std::string_view::npos)
                return SearchResult { ColumnOffset::cast_from(resultIndex) };
            else
                return SearchResult {}; // Not found, so stay with initial column as result.
        }
        else
        {
            InflatedBuffer const& buffer = inflatedBuffer();
            if (buffer.size() < text.size())
                return SearchResult {}; // not found: line is smaller than search term

            // reverse search from right@column to left until match is complete.
            auto baseColumn = std::min(startColumn, ColumnOffset::cast_from(buffer.size() - text.size()));
            auto const originalSize = text.size();
            while (baseColumn >= ColumnOffset(0))
            {
                if (matchTextAt(text, baseColumn))
                    return SearchResult { baseColumn };
                baseColumn--;
            }
            baseColumn = ColumnOffset::cast_from(text.size() - 1);
            while (baseColumn >= ColumnOffset(0))
            {
                text.remove_prefix(text.size() - baseColumn.value);
                if (matchTextAt(text, ColumnOffset(0)))
                    return SearchResult { std::nullopt, int(originalSize - baseColumn.value) };
                baseColumn--;
            }

            return SearchResult {}; // Not found, so stay with initial column as result.
        }
    }

  private:
    Storage storage_;
    unsigned flags_ = 0;
};

constexpr LineFlags operator|(LineFlags a, LineFlags b) noexcept
{
    return LineFlags(unsigned(a) | unsigned(b));
}

constexpr LineFlags operator~(LineFlags a) noexcept
{
    return LineFlags(~unsigned(a));
}

constexpr LineFlags operator&(LineFlags a, LineFlags b) noexcept
{
    return LineFlags(unsigned(a) & unsigned(b));
}

template <typename Cell>
inline typename Line<Cell>::InflatedBuffer& Line<Cell>::inflatedBuffer()
{
    if (std::holds_alternative<TrivialBuffer>(storage_))
        storage_ = inflate<Cell>(std::get<TrivialBuffer>(storage_));
    return std::get<InflatedBuffer>(storage_);
}

template <typename Cell>
inline typename Line<Cell>::InflatedBuffer const& Line<Cell>::inflatedBuffer() const
{
    return const_cast<Line<Cell>*>(this)->inflatedBuffer();
}

} // namespace terminal

namespace fmt // {{{
{
template <>
struct formatter<terminal::LineFlags>
{
    template <typename ParseContext>
    auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const terminal::LineFlags _flags, FormatContext& ctx) const
    {
        static const std::array<std::pair<terminal::LineFlags, std::string_view>, 3> nameMap = {
            std::pair { terminal::LineFlags::Wrappable, std::string_view("Wrappable") },
            std::pair { terminal::LineFlags::Wrapped, std::string_view("Wrapped") },
            std::pair { terminal::LineFlags::Marked, std::string_view("Marked") },
        };
        std::string s;
        for (auto const& mapping: nameMap)
        {
            if ((mapping.first & _flags) != terminal::LineFlags::None)
            {
                if (!s.empty())
                    s += ",";
                s += mapping.second;
            }
        }
        return fmt::format_to(ctx.out(), "{}", s);
    }
};
} // namespace fmt
