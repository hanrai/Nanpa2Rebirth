

#pragma once

#include <cstdint>

struct RGBQUAD {
    std::uint8_t rgbBlue;
    std::uint8_t rgbGreen;
    std::uint8_t rgbRed;
    std::uint8_t rgbReserved;
};

struct BITMAPINFOHEADER {
    std::uint32_t biSize;
    std::int32_t  biWidth;
    std::int32_t  biHeight;
    std::uint16_t biPlanes;
    std::uint16_t biBitCount;
    std::uint32_t biCompression;
    std::uint32_t biSizeImage;
    std::int32_t  biXPelsPerMeter;
    std::int32_t  biYPelsPerMeter;
    std::uint32_t biClrUsed;
    std::uint32_t biClrImportant;
};

struct BITMAPINFO {
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD bmiColors[1];
};

struct PALETTEENTRY {
    std::uint8_t peRed;
    std::uint8_t peGreen;
    std::uint8_t peBlue;
    std::uint8_t peFlags;
};

struct LOGPALETTE {
    std::uint16_t palVersion;
    std::uint16_t palNumEntries;
    PALETTEENTRY palPalEntry[1];
};

struct LOGFONT {
    std::int32_t lfHeight;
    std::int32_t lfWidth;
    std::int32_t lfEscapement;
    std::int32_t lfOrientation;
    std::int32_t lfWeight;
    std::uint8_t lfItalic;
    std::uint8_t lfUnderline;
    std::uint8_t lfStrikeOut;
    std::uint8_t lfCharSet;
    std::uint8_t lfOutPrecision;
    std::uint8_t lfClipPrecision;
    std::uint8_t lfQuality;
    std::uint8_t lfPitchAndFamily;
    char lfFaceName[32];
};

static_assert(sizeof(RGBQUAD) == 4);
static_assert(sizeof(PALETTEENTRY) == 4);
static_assert(sizeof(BITMAPINFOHEADER) == 40);
static_assert(sizeof(LOGFONT) == 60);
