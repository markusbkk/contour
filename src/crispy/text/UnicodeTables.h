/**
 * This file is part of the "libterminal" project
 *   Copyright (c) 2020 Christian Parpart <christian@parpart.family>
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

#include <crispy/text/Unicode.h>
#include <array>

namespace crispy::text::tables {

#if 0
constexpr auto emojiPresentation = std::array{
    Interval{0x231A, 0x231B},
    Interval{0x23E9, 0x23EC},
    Interval{0x23F0, 0x23F0},
    Interval{0x23F3, 0x23F3},
    Interval{0x25FD, 0x25FE},
    Interval{0x2614, 0x2615},
    Interval{0x2648, 0x2653},
    Interval{0x267F, 0x267F},
    Interval{0x2693, 0x2693},
    Interval{0x26A1, 0x26A1},
    Interval{0x26AA, 0x26AB},
    Interval{0x26BD, 0x26BE},
    Interval{0x26C4, 0x26C5},
    Interval{0x26CE, 0x26CE},
    Interval{0x26D4, 0x26D4},
    Interval{0x26EA, 0x26EA},
    Interval{0x26F2, 0x26F3},
    Interval{0x26F5, 0x26F5},
    Interval{0x26FA, 0x26FA},
    Interval{0x26FD, 0x26FD},
    Interval{0x2705, 0x2705},
    Interval{0x270A, 0x270B},
    Interval{0x2728, 0x2728},
    Interval{0x274C, 0x274C},
    Interval{0x274E, 0x274E},
    Interval{0x2753, 0x2755},
    Interval{0x2757, 0x2757},
    Interval{0x2795, 0x2797},
    Interval{0x27B0, 0x27B0},
    Interval{0x27BF, 0x27BF},
    Interval{0x2B1B, 0x2B1C},
    Interval{0x2B50, 0x2B50},
    Interval{0x2B55, 0x2B55},
    Interval{0x1F004, 0x1F004},
    Interval{0x1F0CF, 0x1F0CF},
    Interval{0x1F18E, 0x1F18E},
    Interval{0x1F191, 0x1F19A},
    Interval{0x1F1E6, 0x1F1FF},
    Interval{0x1F201, 0x1F201},
    Interval{0x1F21A, 0x1F21A},
    Interval{0x1F22F, 0x1F22F},
    Interval{0x1F232, 0x1F236},
    Interval{0x1F238, 0x1F23A},
    Interval{0x1F250, 0x1F251},
    Interval{0x1F300, 0x1F320},
    Interval{0x1F32D, 0x1F335},
    Interval{0x1F337, 0x1F37C},
    Interval{0x1F37E, 0x1F393},
    Interval{0x1F3A0, 0x1F3CA},
    Interval{0x1F3CF, 0x1F3D3},
    Interval{0x1F3E0, 0x1F3F0},
    Interval{0x1F3F4, 0x1F3F4},
    Interval{0x1F3F8, 0x1F43E},
    Interval{0x1F440, 0x1F440},
    Interval{0x1F442, 0x1F4FC},
    Interval{0x1F4FF, 0x1F53D},
    Interval{0x1F54B, 0x1F54E},
    Interval{0x1F550, 0x1F567},
    Interval{0x1F57A, 0x1F57A},
    Interval{0x1F595, 0x1F596},
    Interval{0x1F5A4, 0x1F5A4},
    Interval{0x1F5FB, 0x1F64F},
    Interval{0x1F680, 0x1F6C5},
    Interval{0x1F6CC, 0x1F6CC},
    Interval{0x1F6D0, 0x1F6D2},
    Interval{0x1F6D5, 0x1F6D7},
    Interval{0x1F6EB, 0x1F6EC},
    Interval{0x1F6F4, 0x1F6FC},
    Interval{0x1F7E0, 0x1F7EB},
    Interval{0x1F90C, 0x1F93A},
    Interval{0x1F93C, 0x1F945},
    Interval{0x1F947, 0x1F978},
    Interval{0x1F97A, 0x1F9CB},
    Interval{0x1F9CD, 0x1F9FF},
    Interval{0x1FA70, 0x1FA74},
    Interval{0x1FA78, 0x1FA7A},
    Interval{0x1FA80, 0x1FA86},
    Interval{0x1FA90, 0x1FAA8},
    Interval{0x1FAB0, 0x1FAB6},
    Interval{0x1FAC0, 0x1FAC2},
    Interval{0x1FAD0, 0x1FAD6},
};

constexpr auto emojiExtendedPictographic = std::array{
    Interval{0x00A9, 0x00A9},
    Interval{0x00AE, 0x00AE},
    Interval{0x203C, 0x203C},
    Interval{0x2049, 0x2049},
    Interval{0x2122, 0x2122},
    Interval{0x2139, 0x2139},
    Interval{0x2194, 0x2199},
    Interval{0x21A9, 0x21AA},
    Interval{0x231A, 0x231B},
    Interval{0x2328, 0x2328},
    Interval{0x2388, 0x2388},
    Interval{0x23CF, 0x23CF},
    Interval{0x23E9, 0x23F3},
    Interval{0x23F8, 0x23FA},
    Interval{0x24C2, 0x24C2},
    Interval{0x25AA, 0x25AB},
    Interval{0x25B6, 0x25B6},
    Interval{0x25C0, 0x25C0},
    Interval{0x25FB, 0x25FE},
    Interval{0x2600, 0x2605},
    Interval{0x2607, 0x2612},
    Interval{0x2614, 0x2685},
    Interval{0x2690, 0x2705},
    Interval{0x2708, 0x2712},
    Interval{0x2714, 0x2714},
    Interval{0x2716, 0x2716},
    Interval{0x271D, 0x271D},
    Interval{0x2721, 0x2721},
    Interval{0x2728, 0x2728},
    Interval{0x2733, 0x2734},
    Interval{0x2744, 0x2744},
    Interval{0x2747, 0x2747},
    Interval{0x274C, 0x274C},
    Interval{0x274E, 0x274E},
    Interval{0x2753, 0x2755},
    Interval{0x2757, 0x2757},
    Interval{0x2763, 0x2767},
    Interval{0x2795, 0x2797},
    Interval{0x27A1, 0x27A1},
    Interval{0x27B0, 0x27B0},
    Interval{0x27BF, 0x27BF},
    Interval{0x2934, 0x2935},
    Interval{0x2B05, 0x2B07},
    Interval{0x2B1B, 0x2B1C},
    Interval{0x2B50, 0x2B50},
    Interval{0x2B55, 0x2B55},
    Interval{0x3030, 0x3030},
    Interval{0x303D, 0x303D},
    Interval{0x3297, 0x3297},
    Interval{0x3299, 0x3299},
    Interval{0x1F000, 0x1F0FF},
    Interval{0x1F10D, 0x1F10F},
    Interval{0x1F12F, 0x1F12F},
    Interval{0x1F16C, 0x1F171},
    Interval{0x1F17E, 0x1F17F},
    Interval{0x1F18E, 0x1F18E},
    Interval{0x1F191, 0x1F19A},
    Interval{0x1F1AD, 0x1F1E5},
    Interval{0x1F201, 0x1F20F},
    Interval{0x1F21A, 0x1F21A},
    Interval{0x1F22F, 0x1F22F},
    Interval{0x1F232, 0x1F23A},
    Interval{0x1F23C, 0x1F23F},
    Interval{0x1F249, 0x1F3FA},
    Interval{0x1F400, 0x1F53D},
    Interval{0x1F546, 0x1F64F},
    Interval{0x1F680, 0x1F6FF},
    Interval{0x1F774, 0x1F77F},
    Interval{0x1F7D5, 0x1F7FF},
    Interval{0x1F80C, 0x1F80F},
    Interval{0x1F848, 0x1F84F},
    Interval{0x1F85A, 0x1F85F},
    Interval{0x1F888, 0x1F88F},
    Interval{0x1F8AE, 0x1F8FF},
    Interval{0x1F90C, 0x1F93A},
    Interval{0x1F93C, 0x1F945},
    Interval{0x1F947, 0x1FAFF},
    Interval{0x1FC00, 0x1FFFD},
};

constexpr auto emojiModifierBase = std::array{
    Interval{0x261D, 0x261D},
    Interval{0x26F9, 0x26F9},
    Interval{0x270A, 0x270D},
    Interval{0x1F385, 0x1F385},
    Interval{0x1F3C2, 0x1F3C4},
    Interval{0x1F3C7, 0x1F3C7},
    Interval{0x1F3CA, 0x1F3CC},
    Interval{0x1F442, 0x1F443},
    Interval{0x1F446, 0x1F450},
    Interval{0x1F466, 0x1F478},
    Interval{0x1F47C, 0x1F47C},
    Interval{0x1F481, 0x1F483},
    Interval{0x1F485, 0x1F487},
    Interval{0x1F48F, 0x1F48F},
    Interval{0x1F491, 0x1F491},
    Interval{0x1F4AA, 0x1F4AA},
    Interval{0x1F574, 0x1F575},
    Interval{0x1F57A, 0x1F57A},
    Interval{0x1F590, 0x1F590},
    Interval{0x1F595, 0x1F596},
    Interval{0x1F645, 0x1F647},
    Interval{0x1F64B, 0x1F64F},
    Interval{0x1F6A3, 0x1F6A3},
    Interval{0x1F6B4, 0x1F6B6},
    Interval{0x1F6C0, 0x1F6C0},
    Interval{0x1F6CC, 0x1F6CC},
    Interval{0x1F90C, 0x1F90C},
    Interval{0x1F90F, 0x1F90F},
    Interval{0x1F918, 0x1F91F},
    Interval{0x1F926, 0x1F926},
    Interval{0x1F930, 0x1F939},
    Interval{0x1F93C, 0x1F93E},
    Interval{0x1F977, 0x1F977},
    Interval{0x1F9B5, 0x1F9B6},
    Interval{0x1F9B8, 0x1F9B9},
    Interval{0x1F9BB, 0x1F9BB},
    Interval{0x1F9CD, 0x1F9CF},
    Interval{0x1F9D1, 0x1F9DD},
};

constexpr auto emojiModifier = std::array{
    Interval{0x1F3FB , 0x1F3FF},
};

constexpr auto regionalIndicator = std::array{
    Interval{0x1F1E6 , 0x1F1FF},
};

constexpr auto emojiKeycapBase = std::array{
    Interval{'0', '9'},
    Interval{'#', '#'},
    Interval{'*', '*'},
};

constexpr auto emoji = std::array{
    Interval{0x0023, 0x0023},
    Interval{0x002A, 0x002A},
    Interval{0x0030, 0x0039},
    Interval{0x00A9, 0x00A9},
    Interval{0x00AE, 0x00AE},
    Interval{0x203C, 0x203C},
    Interval{0x2049, 0x2049},
    Interval{0x2122, 0x2122},
    Interval{0x2139, 0x2139},
    Interval{0x2194, 0x2199},
    Interval{0x21A9, 0x21AA},
    Interval{0x231A, 0x231B},
    Interval{0x2328, 0x2328},
    Interval{0x23CF, 0x23CF},
    Interval{0x23E9, 0x23F3},
    Interval{0x23F8, 0x23FA},
    Interval{0x24C2, 0x24C2},
    Interval{0x25AA, 0x25AB},
    Interval{0x25B6, 0x25B6},
    Interval{0x25C0, 0x25C0},
    Interval{0x25FB, 0x25FE},
    Interval{0x2600, 0x2604},
    Interval{0x260E, 0x260E},
    Interval{0x2611, 0x2611},
    Interval{0x2614, 0x2615},
    Interval{0x2618, 0x2618},
    Interval{0x261D, 0x261D},
    Interval{0x2620, 0x2620},
    Interval{0x2622, 0x2623},
    Interval{0x2626, 0x2626},
    Interval{0x262A, 0x262A},
    Interval{0x262E, 0x262F},
    Interval{0x2638, 0x263A},
    Interval{0x2640, 0x2640},
    Interval{0x2642, 0x2642},
    Interval{0x2648, 0x2653},
    Interval{0x265F, 0x2660},
    Interval{0x2663, 0x2663},
    Interval{0x2665, 0x2666},
    Interval{0x2668, 0x2668},
    Interval{0x267B, 0x267B},
    Interval{0x267E, 0x267F},
    Interval{0x2692, 0x2697},
    Interval{0x2699, 0x2699},
    Interval{0x269B, 0x269C},
    Interval{0x26A0, 0x26A1},
    Interval{0x26A7, 0x26A7},
    Interval{0x26AA, 0x26AB},
    Interval{0x26B0, 0x26B1},
    Interval{0x26BD, 0x26BE},
    Interval{0x26C4, 0x26C5},
    Interval{0x26C8, 0x26C8},
    Interval{0x26CE, 0x26CF},
    Interval{0x26D1, 0x26D1},
    Interval{0x26D3, 0x26D4},
    Interval{0x26E9, 0x26EA},
    Interval{0x26F0, 0x26F5},
    Interval{0x26F7, 0x26FA},
    Interval{0x26FD, 0x26FD},
    Interval{0x2702, 0x2702},
    Interval{0x2705, 0x2705},
    Interval{0x2708, 0x270D},
    Interval{0x270F, 0x270F},
    Interval{0x2712, 0x2712},
    Interval{0x2714, 0x2714},
    Interval{0x2716, 0x2716},
    Interval{0x271D, 0x271D},
    Interval{0x2721, 0x2721},
    Interval{0x2728, 0x2728},
    Interval{0x2733, 0x2734},
    Interval{0x2744, 0x2744},
    Interval{0x2747, 0x2747},
    Interval{0x274C, 0x274C},
    Interval{0x274E, 0x274E},
    Interval{0x2753, 0x2755},
    Interval{0x2757, 0x2757},
    Interval{0x2763, 0x2764},
    Interval{0x2795, 0x2797},
    Interval{0x27A1, 0x27A1},
    Interval{0x27B0, 0x27B0},
    Interval{0x27BF, 0x27BF},
    Interval{0x2934, 0x2935},
    Interval{0x2B05, 0x2B07},
    Interval{0x2B1B, 0x2B1C},
    Interval{0x2B50, 0x2B50},
    Interval{0x2B55, 0x2B55},
    Interval{0x3030, 0x3030},
    Interval{0x303D, 0x303D},
    Interval{0x3297, 0x3297},
    Interval{0x3299, 0x3299},
    Interval{0x1F004, 0x1F004},
    Interval{0x1F0CF, 0x1F0CF},
    Interval{0x1F170, 0x1F171},
    Interval{0x1F17E, 0x1F17F},
    Interval{0x1F18E, 0x1F18E},
    Interval{0x1F191, 0x1F19A},
    Interval{0x1F1E6, 0x1F1FF},
    Interval{0x1F201, 0x1F202},
    Interval{0x1F21A, 0x1F21A},
    Interval{0x1F22F, 0x1F22F},
    Interval{0x1F232, 0x1F23A},
    Interval{0x1F250, 0x1F251},
    Interval{0x1F300, 0x1F321},
    Interval{0x1F324, 0x1F393},
    Interval{0x1F396, 0x1F397},
    Interval{0x1F399, 0x1F39B},
    Interval{0x1F39E, 0x1F3F0},
    Interval{0x1F3F3, 0x1F3F5},
    Interval{0x1F3F7, 0x1F4FD},
    Interval{0x1F4FF, 0x1F53D},
    Interval{0x1F549, 0x1F54E},
    Interval{0x1F550, 0x1F567},
    Interval{0x1F56F, 0x1F570},
    Interval{0x1F573, 0x1F57A},
    Interval{0x1F587, 0x1F587},
    Interval{0x1F58A, 0x1F58D},
    Interval{0x1F590, 0x1F590},
    Interval{0x1F595, 0x1F596},
    Interval{0x1F5A4, 0x1F5A5},
    Interval{0x1F5A8, 0x1F5A8},
    Interval{0x1F5B1, 0x1F5B2},
    Interval{0x1F5BC, 0x1F5BC},
    Interval{0x1F5C2, 0x1F5C4},
    Interval{0x1F5D1, 0x1F5D3},
    Interval{0x1F5DC, 0x1F5DE},
    Interval{0x1F5E1, 0x1F5E1},
    Interval{0x1F5E3, 0x1F5E3},
    Interval{0x1F5E8, 0x1F5E8},
    Interval{0x1F5EF, 0x1F5EF},
    Interval{0x1F5F3, 0x1F5F3},
    Interval{0x1F5FA, 0x1F64F},
    Interval{0x1F680, 0x1F6C5},
    Interval{0x1F6CB, 0x1F6D2},
    Interval{0x1F6D5, 0x1F6D7},
    Interval{0x1F6E0, 0x1F6E5},
    Interval{0x1F6E9, 0x1F6E9},
    Interval{0x1F6EB, 0x1F6EC},
    Interval{0x1F6F0, 0x1F6F0},
    Interval{0x1F6F3, 0x1F6FC},
    Interval{0x1F7E0, 0x1F7EB},
    Interval{0x1F90C, 0x1F93A},
    Interval{0x1F93C, 0x1F945},
    Interval{0x1F947, 0x1F978},
    Interval{0x1F97A, 0x1F9CB},
    Interval{0x1F9CD, 0x1F9FF},
    Interval{0x1FA70, 0x1FA74},
    Interval{0x1FA78, 0x1FA7A},
    Interval{0x1FA80, 0x1FA86},
    Interval{0x1FA90, 0x1FAA8},
    Interval{0x1FAB0, 0x1FAB6},
    Interval{0x1FAC0, 0x1FAC2},
    Interval{0x1FAD0, 0x1FAD6},
};
#endif

} // end namespace
