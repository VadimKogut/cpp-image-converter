#include "ppm_image.h"

#include <array>
#include <fstream>
#include <string_view>
#include <vector>

using namespace std;

namespace img_lib {

static constexpr string_view PPM_SIG = "P6"sv;
static constexpr int PPM_MAX = 255;

bool SavePPM(const Path& file, const Image& image) {
    ofstream out(file, ios::binary);
    if (!out) return false;

    const int w = image.GetWidth();
    const int h = image.GetHeight();
    
    // Записываем заголовок одним вызовом
    out << PPM_SIG << '\n' << w << ' ' << h << '\n' << PPM_MAX << '\n';
    if (!out) return false;

    // Используем буфер размером со всю строку
    vector<char> buff(w * 3);

    for (int y = 0; y < h; ++y) {
        const Color* line = image.GetLine(y);
        for (int x = 0; x < w; ++x) {
            buff[x * 3 + 0] = static_cast<char>(line[x].r);
            buff[x * 3 + 1] = static_cast<char>(line[x].g);
            buff[x * 3 + 2] = static_cast<char>(line[x].b);
        }
        out.write(buff.data(), buff.size());
        if (!out) return false;
    }

    return true;
}

Image LoadPPM(const Path& file) {
    ifstream ifs(file, ios::binary);
    if (!ifs) return {};

    string sign;
    int w, h, color_max;

    // Читаем заголовок
    if (!(ifs >> sign >> w >> h >> color_max)) return {};
    
    // Проверяем формат
    if (sign != PPM_SIG || color_max != PPM_MAX || w <= 0 || h <= 0) {
        return {};
    }

    // Пропускаем оставшиеся байты до начала данных
    ifs.ignore(numeric_limits<streamsize>::max(), '\n');
    if (ifs.gcount() != 1) return {};

    Image result(w, h, Color::Black());
    vector<char> buff(w * 3);

    for (int y = 0; y < h; ++y) {
        Color* line = result.GetLine(y);
        ifs.read(buff.data(), buff.size());
        if (ifs.gcount() != static_cast<streamsize>(buff.size())) {
            return {};
        }

        for (int x = 0; x < w; ++x) {
            line[x].r = static_cast<byte>(buff[x * 3 + 0]);
            line[x].g = static_cast<byte>(buff[x * 3 + 1]);
            line[x].b = static_cast<byte>(buff[x * 3 + 2]);
        }
    }

    return result;
}

}  // namespace img_lib
