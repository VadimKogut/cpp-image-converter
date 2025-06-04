//Простите пожалуйста, замотался и не то прикрепил, теперь всё должно быть верно

#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>
#include <vector>

using namespace std;

namespace img_lib {

PACKED_STRUCT_BEGIN BitmapFileHeader {
    uint16_t bfType = 0x4D42;  // 'BM'
    uint32_t bfSize = 0;
    uint16_t bfReserved1 = 0;
    uint16_t bfReserved2 = 0;
    uint32_t bfOffBits = 14 + 40;
}
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN BitmapInfoHeader {
    uint32_t biSize = 40;
    int32_t  biWidth = 0;
    int32_t  biHeight = 0;
    uint16_t biPlanes = 1;
    uint16_t biBitCount = 24;
    uint32_t biCompression = 0;
    uint32_t biSizeImage = 0;
    int32_t  biXPelsPerMeter = 11811;     
    int32_t  biYPelsPerMeter = 11811;
    uint32_t biClrUsed = 0;
    uint32_t biClrImportant = 0x1000000;  
}
PACKED_STRUCT_END

static int GetBMPStride(int width) {
    return 4 * ((width * 3 + 3) / 4);
}

bool SaveBMP(const Path& file, const Image& image) {
    ofstream out(file, ios::binary);
    if (!out) {
        return false;
    }

    const int width = image.GetWidth();
    const int height = image.GetHeight();
    const int stride = GetBMPStride(width);

    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;

    info_header.biWidth = width;
    info_header.biHeight = height;
    info_header.biSizeImage = stride * height;
    file_header.bfSize = file_header.bfOffBits + info_header.biSizeImage;

    if (!out.write(reinterpret_cast<const char*>(&file_header), 14)) {
        return false;
    }
    if (!out.write(reinterpret_cast<const char*>(&info_header), 40)) {
        return false;
    }

    vector<char> buffer(stride);
    for (int y = height - 1; y >= 0; --y) {
        const Color* line = image.GetLine(y);
        for (int x = 0; x < width; ++x) {
            buffer[x * 3 + 0] = static_cast<char>(line[x].b);
            buffer[x * 3 + 1] = static_cast<char>(line[x].g);
            buffer[x * 3 + 2] = static_cast<char>(line[x].r);
        }
        if (!out.write(buffer.data(), stride)) {
            return false;
        }
    }

    return true;
}

Image LoadBMP(const Path& file) {
    ifstream in(file, ios::binary);
    if (!in) {
        return {};
    }

    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;

    if (!in.read(reinterpret_cast<char*>(&file_header), 14) || 
        !in.read(reinterpret_cast<char*>(&info_header), 40)) {
        return {};
    }

    if (file_header.bfType != 0x4D42 ||
        info_header.biBitCount != 24 ||
        info_header.biCompression != 0) {
        return {};
    }

    const int width = info_header.biWidth;
    int height = info_header.biHeight;

    if (width <= 0 || height == 0) return {};

    bool top_down = false;
    if (height < 0) {
        top_down = true;
        height = -height;
    }

    const int stride = GetBMPStride(width);
    Image image(width, height, Color::Black());
    vector<char> row(stride);

    for (int y = 0; y < height; ++y) {
        int row_index = top_down ? y : height - 1 - y;

        if (in.read(row.data(), stride).gcount() != stride) {
            return {};
        }

        Color* line = image.GetLine(row_index);
        for (int x = 0; x < width; ++x) {
            line[x].b = static_cast<std::byte>(row[x * 3 + 0]);
            line[x].g = static_cast<std::byte>(row[x * 3 + 1]);
            line[x].r = static_cast<std::byte>(row[x * 3 + 2]);
            line[x].a = std::byte{255};
        }
    }

    return image;
}

}  // namespace img_lib
