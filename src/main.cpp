#include <chrono>
#include <fstream>
#include <iostream>

#include "pixel.hpp"
#include "image.hpp"

using namespace std;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "Usage: bin/ImageProcessor.out <bitmap_file>" << endl;
        return EXIT_FAILURE;
    }
    auto start_time = chrono::high_resolution_clock::now();
    Image image;
    try {
        image.read(argv[1]);
    } catch (runtime_error &ex) {
        cerr << "Runtime error: " << ex.what() << endl;
        return EXIT_FAILURE;
    } catch (...) {
        cerr << "An error ocuured in reading file!" << endl;
        return EXIT_FAILURE;
    }
    image.apply_horizontal_filter();
    image.apply_vertical_filter();
    image.apply_sharpen_filter();
    image.apply_sepia_filter();
    image.apply_x_mark();
    image.write("output.bmp");
    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
    cout << "Execution time: " << duration.count()  << "ms" << endl;
    return EXIT_SUCCESS;
}
