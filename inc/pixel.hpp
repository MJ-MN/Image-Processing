#pragma once

class Pixel {
public:
    Pixel();
    Pixel(int _r, int _g, int _b);
    Pixel(const Pixel &pixel) = default;
    Pixel &operator=(const Pixel &pixel) = default;
    Pixel operator*(const int opr);
    Pixel &operator+=(const Pixel &pixel);
    ~Pixel();
    void read(std::ifstream &ifs);
    void write(std::ofstream &ofs);
    void check_boundaries();
    void apply_matrix(const float matrix[3][3]);
    void set_color(const Pixel &color);
private:
    int r;
    int g;
    int b;
};
