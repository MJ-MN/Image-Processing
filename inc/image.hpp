#pragma once

#define PTHREAD

const int THREAD_COUNT = 4;

const int SHARPEN_KERNEL[3][3] = {
    {0, -1, 0},
    {-1, 5, -1},
    {0, -1, 0}
};

const float SEPIA_MATRIX[3][3] = {
    {0.393f, 0.769f, 0.189f},
    {0.349f, 0.686f, 0.168f},
    {0.272f, 0.534f, 0.131f}
};

const Pixel WHITE_COLOR(255, 255, 255);

#pragma pack(push, 1)
typedef struct {
    uint8_t signature[2];
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} header_t;

typedef struct {
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bpp;
    uint32_t compression;
    uint32_t raw_size;
    int32_t x_ppm;
    int32_t y_ppm;
    uint32_t color_table;
    uint32_t important_colors;
} info_t;
#pragma pack(pop)

#ifdef PTHREAD
class Image;

typedef struct {
    Image *image;
    int start_row;
    int end_row;
} thread_struct_t;
#endif

class Image {
public:
    Image();
    Image(const char *file_name);
    ~Image();
    void read(const char *file_name);
    void apply_horizontal_filter();
    void apply_vertical_filter();
    void apply_sharpen_filter();
    void apply_sepia_filter();
    void apply_x_mark();
    void write(const char *file_name);
private:
    header_t header;
    info_t info;
    uint8_t *ext_info;
    Pixel **pixels;

    void read_header_info(std::ifstream &ifs);
    void read_data(std::ifstream &ifs);
    Pixel apply_kernel(int row, int col, const int kernel[3][3]);
    void write_header_info(std::ofstream &ofs);
    void write_data(std::ofstream &ofs);
    #ifndef PTHREAD
    void apply_horizontal_filter_prl(int start_row, int end_row);
    void apply_vertical_filter_prl(int start_row, int end_row);
    void apply_sharpen_filter_prl(int start_row, int end_row);
    void apply_sepia_filter_prl(int start_row, int end_row);
    void apply_x_mark_prl(int start_row, int end_row);
    #else
    static void *apply_horizontal_filter_prl(void *arg);
    static void *apply_vertical_filter_prl(void *arg);
    static void *apply_sharpen_filter_prl(void *arg);
    static void *apply_sepia_filter_prl(void *arg);
    static void *apply_x_mark_prl(void *arg);
    #endif
};
