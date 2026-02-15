#include <fstream>

#include "pixel.hpp"

using namespace std;

Pixel::Pixel() : r(0), g(0), b(0) {}

Pixel::Pixel(int _r, int _g, int _b) :
    r(_r), g(_g), b(_b) {}

Pixel Pixel::operator*(const int opr) {
    return Pixel(this->r * opr, this->g * opr, this->b * opr);
}

Pixel &Pixel::operator+=(const Pixel &pixel) {
    this->r += pixel.r;
    this->g += pixel.g;
    this->b += pixel.b;
    return *this;
}

Pixel::~Pixel() {}

void Pixel::read(ifstream &ifs) {
    ifs.read((char *)&this->b, 1);
    ifs.read((char *)&this->g, 1);
    ifs.read((char *)&this->r, 1);
}

void Pixel::write(ofstream &ofs) {
    ofs.write((char *)&this->b, 1);
    ofs.write((char *)&this->g, 1);
    ofs.write((char *)&this->r, 1);
}

void Pixel::check_boundaries() {
    this->r = max(0, min(this->r, 255));
    this->g = max(0, min(this->g, 255));
    this->b = max(0, min(this->b, 255));
}

void Pixel::apply_matrix(const float matrix[3][3]) {
    Pixel temp_pixel(*this);
    this->r = matrix[0][0] * temp_pixel.r +
              matrix[0][1] * temp_pixel.g +
              matrix[0][2] * temp_pixel.b;
    this->g = matrix[1][0] * temp_pixel.r +
              matrix[1][1] * temp_pixel.g +
              matrix[1][2] * temp_pixel.b;
    this->b = matrix[2][0] * temp_pixel.r +
              matrix[2][1] * temp_pixel.g +
              matrix[2][2] * temp_pixel.b;
    this->check_boundaries();
}

void Pixel::set_color(const Pixel &color) {
    this->r = color.r;
    this->g = color.g;
    this->b = color.b;
}
