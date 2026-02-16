#include <cstring>
#include <fstream>

#include "pixel.hpp"
#include "image.hpp"

#ifndef PTHREAD
#include <thread>
#else
#include <pthread.h>
#endif

using namespace std;

Image::Image() {
    memset(&this->header, 0, sizeof(this->header));
    memset(&this->info, 0, sizeof(this->info));
    this->ext_info = nullptr;
    this->pixels = nullptr;
}

Image::Image(const char *file_name) {
    this->read(file_name);
}

Image::~Image() {
    if (this->pixels != nullptr) {
        for (int i = 0; i < this->info.height; ++i) {
            delete[] this->pixels[i];
        }
        delete[] this->pixels;
    }
    if (this->ext_info != nullptr) {
        delete[] this->ext_info;
    }
}

void Image::read(const char *file_name) {
    ifstream image_file(file_name);
    if (image_file) {
        this->read_header_info(image_file);
        this->read_data(image_file);
    } else {
        throw runtime_error("No such file!");
    }
}

void Image::read_header_info(ifstream &ifs) {
    ifs.read((char *)&this->header, sizeof(header_t));
    ifs.read((char *)&this->info, sizeof(info_t));
    int ext_info_size = this->header.offset - sizeof(header_t) - sizeof(info_t);
    this->ext_info = new uint8_t[ext_info_size];
    ifs.read((char *)this->ext_info, ext_info_size);
}

void Image::read_data(ifstream &ifs) {
    int padding = ((this->info.width * 3) % 4 == 0) ?
                  0 :
                  4 - (this->info.width * 3) % 4;
    this->pixels = new Pixel *[this->info.height];
    for (int i = 0; i < this->info.height; ++i) {
        this->pixels[i] = new Pixel[this->info.width];
        for (int j = 0; j < this->info.width; ++j) {
            this->pixels[i][j].read(ifs);
        }
        ifs.ignore(padding);
    }
}

#ifndef PTHREAD
void Image::apply_horizontal_filter() {
    thread threads[THREAD_COUNT];
    int chunk_size = (this->info.height % THREAD_COUNT > 1) ? 
                     this->info.height / THREAD_COUNT + 1 :
                     this->info.height / THREAD_COUNT;
    int i = 0;
    for (i = 0; i < THREAD_COUNT - 1; ++i) {
        threads[i] = thread(&Image::apply_horizontal_filter_prl, this,
                            i * chunk_size, (i + 1) * chunk_size);
    }
    threads[i] = thread(&Image::apply_horizontal_filter_prl, this,
                        i * chunk_size, this->info.height);
    for (i = 0; i < THREAD_COUNT; ++i) {
        threads[i].join();
    }
}

void Image::apply_horizontal_filter_prl(int start_row, int end_row) {
    Pixel temp;
    for (int i = start_row; i < end_row; ++i) {
        for (int j = 0; j < this->info.width / 2; ++j) {
            temp = this->pixels[i][j];
            this->pixels[i][j] = this->pixels[i][this->info.width - 1 - j];
            this->pixels[i][this->info.width - 1 - j] = temp;
        }
    }
}

void Image::apply_vertical_filter() {
    thread threads[THREAD_COUNT];
    int chunk_size = ((this->info.height / 2) % THREAD_COUNT > 1) ? 
                     this->info.height / 2 / THREAD_COUNT + 1:
                     this->info.height / 2 / THREAD_COUNT;
    int i = 0;
    for (i = 0; i < THREAD_COUNT - 1; ++i) {
        threads[i] = thread(&Image::apply_vertical_filter_prl, this,
                            i * chunk_size, (i + 1) * chunk_size);
    }
    threads[i] = thread(&Image::apply_vertical_filter_prl, this,
                        i * chunk_size, this->info.height / 2);
    for (i = 0; i < THREAD_COUNT; ++i) {
        threads[i].join();
    }
}

void Image::apply_vertical_filter_prl(int start_row, int end_row) {
    Pixel *temp;
    for (int i = start_row; i < end_row; ++i) {
        temp = this->pixels[i];
        this->pixels[i] = this->pixels[this->info.height - 1 - i];
        this->pixels[this->info.height - 1 - i] = temp;
    }
}

void Image::apply_sharpen_filter() {
    thread threads[THREAD_COUNT];
    int chunk_size = (this->info.height % THREAD_COUNT > 1) ? 
                     this->info.height / THREAD_COUNT + 1:
                     this->info.height / THREAD_COUNT;
    int i = 0;
    for (i = 0; i < THREAD_COUNT - 1; ++i) {
        threads[i] = thread(&Image::apply_sharpen_filter_prl, this,
                            i * chunk_size, (i + 1) * chunk_size);
    }
    threads[i] = thread(&Image::apply_sharpen_filter_prl, this,
                        i * chunk_size, this->info.height);
    for (i = 0; i < THREAD_COUNT; ++i) {
        threads[i].join();
    }
}

void Image::apply_sharpen_filter_prl(int start_row, int end_row) {
    Pixel *temp_rows[2];
    temp_rows[0] = new Pixel[this->info.width];
    temp_rows[1] = new Pixel[this->info.width];
    for (int i = start_row; i < end_row; ++i) {
        for (int j = 0; j < this->info.width; ++j) {
            if (i > start_row + 1) {
                this->pixels[i - 2][j] = temp_rows[i % 2][j];
            }
            temp_rows[i % 2][j] = this->apply_kernel(i, j, SHARPEN_KERNEL);
        }
    }
    for (int i = end_row - 2; i < end_row; ++i) {
        for (int j = 0; j < this->info.width; ++j) {
            this->pixels[i][j] = temp_rows[i % 2][j];
        }
    }
    delete[] temp_rows[0];
    delete[] temp_rows[1];
}

void Image::apply_sepia_filter() {
    thread threads[THREAD_COUNT];
    int chunk_size = (this->info.height % THREAD_COUNT > 1) ? 
                     this->info.height / THREAD_COUNT + 1:
                     this->info.height / THREAD_COUNT;
    int i = 0;
    for (i = 0; i < THREAD_COUNT - 1; ++i) {
        threads[i] = thread(&Image::apply_sepia_filter_prl, this,
                            i * chunk_size, (i + 1) * chunk_size);
    }
    threads[i] = thread(&Image::apply_sepia_filter_prl, this,
                        i * chunk_size, this->info.height);
    for (i = 0; i < THREAD_COUNT; ++i) {
        threads[i].join();
    }
}

void Image::apply_sepia_filter_prl(int start_row, int end_row) {
    for (int i = start_row; i < end_row; ++i) {
        for (int j = 0; j < this->info.width; ++j) {
            this->pixels[i][j].apply_matrix(SEPIA_MATRIX);
        }
    }
}

void Image::apply_x_mark() {
    thread threads[THREAD_COUNT];
    int chunk_size = (this->info.height % THREAD_COUNT > 1) ? 
                     this->info.height / THREAD_COUNT + 1:
                     this->info.height / THREAD_COUNT;
    int i = 0;
    for (i = 0; i < THREAD_COUNT - 1; ++i) {
        threads[i] = thread(&Image::apply_x_mark_prl, this,
                            i * chunk_size, (i + 1) * chunk_size);
    }
    threads[i] = thread(&Image::apply_x_mark_prl, this,
                        i * chunk_size, this->info.height);
    for (i = 0; i < THREAD_COUNT; ++i) {
        threads[i].join();
    }
}

void Image::apply_x_mark_prl(int start_row, int end_row) {
    float m = (float)this->info.height / (float)this->info.width;
    for (int i = start_row; i < end_row; ++i) {
        for (int j = 0; j < this->info.width; ++j) {
            if ((m * j - i > -1 && m * j - i < 1) ||
                ((this->info.height - m * j - i > -1) &&
                 (this->info.height - m * j - i < 1))) {
                this->pixels[i][j].set_color(WHITE_COLOR);
            }
        }
    }
}
#else
void Image::apply_horizontal_filter() {
    pthread_t threads[THREAD_COUNT];
    int chunk_size = (this->info.height % THREAD_COUNT > 1) ? 
                     this->info.height / THREAD_COUNT + 1 :
                     this->info.height / THREAD_COUNT;
    int i = 0;
    thread_struct_t *thread_struct = new thread_struct_t[THREAD_COUNT];
    for (i = 0; i < THREAD_COUNT - 1; ++i) {
        thread_struct[i].image = this;
        thread_struct[i].start_row = i * chunk_size;
        thread_struct[i].end_row = (i + 1) * chunk_size;
        pthread_create(&threads[i], nullptr, apply_horizontal_filter_prl,
                       &thread_struct[i]);
    }
    thread_struct[i].image = this;
    thread_struct[i].start_row = i * chunk_size;
    thread_struct[i].end_row = this->info.height;
    pthread_create(&threads[i], nullptr, apply_horizontal_filter_prl,
                   &thread_struct[i]);
    for (i = 0; i < THREAD_COUNT; ++i) {
        pthread_join(threads[i], nullptr);
    }
    delete[] thread_struct;
}

void *Image::apply_horizontal_filter_prl(void *arg) {
    thread_struct_t *thread_struct = static_cast<thread_struct_t *>(arg);
    Image *image = thread_struct->image;
    Pixel temp;
    for (int i = thread_struct->start_row; i < thread_struct->end_row; ++i) {
        for (int j = 0; j < image->info.width / 2; ++j) {
            temp = image->pixels[i][j];
            image->pixels[i][j] = image->pixels[i][image->info.width - 1 - j];
            image->pixels[i][image->info.width - 1 - j] = temp;
        }
    }
    return nullptr;
}

void Image::apply_vertical_filter() {
    pthread_t threads[THREAD_COUNT];
    int chunk_size = ((this->info.height / 2) % THREAD_COUNT > 1) ? 
                     this->info.height / 2 / THREAD_COUNT + 1:
                     this->info.height / 2 / THREAD_COUNT;
    int i = 0;
    thread_struct_t *thread_struct = new thread_struct_t[THREAD_COUNT];
    for (i = 0; i < THREAD_COUNT - 1; ++i) {
        thread_struct[i].image = this;
        thread_struct[i].start_row = i * chunk_size;
        thread_struct[i].end_row = (i + 1) * chunk_size;
        pthread_create(&threads[i], nullptr, apply_vertical_filter_prl,
                       &thread_struct[i]);
    }
    thread_struct[i].image = this;
    thread_struct[i].start_row = i * chunk_size;
    thread_struct[i].end_row = this->info.height / 2;
    pthread_create(&threads[i], nullptr, apply_vertical_filter_prl,
                   &thread_struct[i]);
    for (i = 0; i < THREAD_COUNT; ++i) {
        pthread_join(threads[i], nullptr);
    }
    delete[] thread_struct;
}

void *Image::apply_vertical_filter_prl(void *arg) {
    thread_struct_t *thread_struct = static_cast<thread_struct_t *>(arg);
    Image *image = thread_struct->image;
    Pixel *temp;
    for (int i = thread_struct->start_row; i < thread_struct->end_row; ++i) {
        temp = image->pixels[i];
        image->pixels[i] = image->pixels[image->info.height - 1 - i];
        image->pixels[image->info.height - 1 - i] = temp;
    }
    return nullptr;
}

void Image::apply_sharpen_filter() {
    pthread_t threads[THREAD_COUNT];
    int chunk_size = (this->info.height % THREAD_COUNT > 1) ? 
                     this->info.height / THREAD_COUNT + 1:
                     this->info.height / THREAD_COUNT;
    int i = 0;
    thread_struct_t *thread_struct = new thread_struct_t[THREAD_COUNT];
    for (i = 0; i < THREAD_COUNT - 1; ++i) {
        thread_struct[i].image = this;
        thread_struct[i].start_row = i * chunk_size;
        thread_struct[i].end_row = (i + 1) * chunk_size;
        pthread_create(&threads[i], nullptr, apply_sharpen_filter_prl,
                       &thread_struct[i]);
    }
    thread_struct[i].image = this;
    thread_struct[i].start_row = i * chunk_size;
    thread_struct[i].end_row = this->info.height;
    pthread_create(&threads[i], nullptr, apply_sharpen_filter_prl,
                   &thread_struct[i]);
    for (i = 0; i < THREAD_COUNT; ++i) {
        pthread_join(threads[i], nullptr);
    }
    delete[] thread_struct;
}

void *Image::apply_sharpen_filter_prl(void *arg) {
    thread_struct_t *thread_struct = static_cast<thread_struct_t *>(arg);
    Image *image = thread_struct->image;
    Pixel *temp_rows[2];
    temp_rows[0] = new Pixel[image->info.width];
    temp_rows[1] = new Pixel[image->info.width];
    for (int i = thread_struct->start_row; i < thread_struct->end_row; ++i) {
        for (int j = 0; j < image->info.width; ++j) {
            if (i > thread_struct->start_row + 1) {
                image->pixels[i - 2][j] = temp_rows[i % 2][j];
            }
            temp_rows[i % 2][j] = image->apply_kernel(i, j, SHARPEN_KERNEL);
        }
    }
    for (int i = thread_struct->end_row - 2; i < thread_struct->end_row; ++i) {
        for (int j = 0; j < image->info.width; ++j) {
            image->pixels[i][j] = temp_rows[i % 2][j];
        }
    }
    delete[] temp_rows[0];
    delete[] temp_rows[1];
    return nullptr;
}

void Image::apply_sepia_filter() {
    pthread_t threads[THREAD_COUNT];
    int chunk_size = (this->info.height % THREAD_COUNT > 1) ? 
                     this->info.height / THREAD_COUNT + 1:
                     this->info.height / THREAD_COUNT;
    int i = 0;
    thread_struct_t *thread_struct = new thread_struct_t[THREAD_COUNT];
    for (i = 0; i < THREAD_COUNT - 1; ++i) {
        thread_struct[i].image = this;
        thread_struct[i].start_row = i * chunk_size;
        thread_struct[i].end_row = (i + 1) * chunk_size;
        pthread_create(&threads[i], nullptr, apply_sepia_filter_prl,
                       &thread_struct[i]);
    }
    thread_struct[i].image = this;
    thread_struct[i].start_row = i * chunk_size;
    thread_struct[i].end_row = this->info.height;
    pthread_create(&threads[i], nullptr, apply_sepia_filter_prl,
                   &thread_struct[i]);
    for (i = 0; i < THREAD_COUNT; ++i) {
        pthread_join(threads[i], nullptr);
    }
    delete[] thread_struct;
}

void *Image::apply_sepia_filter_prl(void *arg) {
    thread_struct_t *thread_struct = static_cast<thread_struct_t *>(arg);
    Image *image = thread_struct->image;
    for (int i = thread_struct->start_row; i < thread_struct->end_row; ++i) {
        for (int j = 0; j < image->info.width; ++j) {
            image->pixels[i][j].apply_matrix(SEPIA_MATRIX);
        }
    }
    return nullptr;
}

void Image::apply_x_mark() {
    pthread_t threads[THREAD_COUNT];
    int chunk_size = (this->info.height % THREAD_COUNT > 1) ? 
                     this->info.height / THREAD_COUNT + 1:
                     this->info.height / THREAD_COUNT;
    int i = 0;
    thread_struct_t *thread_struct = new thread_struct_t[THREAD_COUNT];
    for (i = 0; i < THREAD_COUNT - 1; ++i) {
        thread_struct[i].image = this;
        thread_struct[i].start_row = i * chunk_size;
        thread_struct[i].end_row = (i + 1) * chunk_size;
        pthread_create(&threads[i], nullptr, apply_x_mark_prl,
                       &thread_struct[i]);
    }
    thread_struct[i].image = this;
    thread_struct[i].start_row = i * chunk_size;
    thread_struct[i].end_row = this->info.height;
    pthread_create(&threads[i], nullptr, apply_x_mark_prl,
                   &thread_struct[i]);
    for (i = 0; i < THREAD_COUNT; ++i) {
        pthread_join(threads[i], nullptr);
    }
    delete[] thread_struct;
}

void *Image::apply_x_mark_prl(void *arg) {
    thread_struct_t *thread_struct = static_cast<thread_struct_t *>(arg);
    Image *image = thread_struct->image;
    float m = (float)image->info.height / (float)image->info.width;
    for (int i = thread_struct->start_row; i < thread_struct->end_row; ++i) {
        for (int j = 0; j < image->info.width; ++j) {
            if ((m * j - i > -1 && m * j - i < 1) ||
                ((image->info.height - m * j - i > -1) &&
                 (image->info.height - m * j - i < 1))) {
                image->pixels[i][j].set_color(WHITE_COLOR);
            }
        }
    }
    return nullptr;
}
#endif

Pixel Image::apply_kernel(int row, int col, const int kernel[3][3]) {
    Pixel sum;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (row + i - 1 < 0 ||
                col + j - 1 < 0 ||
                row + i > this->info.height ||
                col + j > this->info.width) continue;
            sum += this->pixels[row + i - 1][col + j - 1] * kernel[i][j];
        }
    }
    sum.check_boundaries();
    return sum;
}

void Image::write(const char *file_name) {
    ofstream image_file(file_name);
    if (image_file) {
        this->write_header_info(image_file);
        this->write_data(image_file);
    } else {
        throw runtime_error("Cannot create file!");
    }
}

void Image::write_header_info(ofstream &ofs) {
    ofs.write((char *)&this->header, sizeof(header_t));
    ofs.write((char *)&this->info, sizeof(info_t));
    int ext_info_size = this->header.offset - sizeof(header_t) - sizeof(info_t);
    ofs.write((char *)this->ext_info, ext_info_size);
}

void Image::write_data(ofstream &ofs) {
    int padding = ((this->info.width * 3) % 4 == 0) ?
                  0 :
                  4 - (this->info.width * 3) % 4;
    for (int i = 0; i < this->info.height; ++i) {
        for (int j = 0; j < this->info.width; ++j) {
            this->pixels[i][j].write(ofs);
        }
        for (int p = 0; p < padding; ++p) {
            ofs.write("0", 1);
        }
    }
}
