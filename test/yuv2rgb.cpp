//
// Created by Acer on 2020/11/16.
//

#include <iostream>
#include "ohm/list_files.h"
#include "ohm/thread/shotgun.h"
#include "ohm/progress_bar.h"

#include <fstream>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace ohm;

std::shared_ptr<char> read_file(const std::string &filename, size_t &read) {
    std::shared_ptr<char> bin;
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) return bin;
    in.seekg(0, std::ios::end);
    auto size = in.tellg();
    bin.reset(new char[size], std::default_delete<char[]>());
    in.seekg(0, std::ios::beg);
    in.read(bin.get(), size);
    in.close();
    read = size;
    return bin;
}


void YV12ToBGR24_OpenCV(unsigned char *pYUV, cv::Mat &rgbImg, int width, int height) {
    if (width < 1 || height < 1 || pYUV == NULL)
        return;
    cv::Mat src(height + height / 2, width, CV_8UC1, pYUV);
    cvtColor(src, rgbImg,  cv::COLOR_YUV2RGB_NV21);
}

int do_img_conv_impl(unsigned char *img_data, int w, int h,
                                    const std::string &fn) {
    cv::Mat mat_dst;
    YV12ToBGR24_OpenCV(img_data, mat_dst, w, h);

    // write image

    return 0;
}


cv::Mat to_rgb(const std::string &yuv) {
    static const int height = 1080;
    static const int width = 1920;
    static const int yuv_bytes = (height + height / 2) * width;
    size_t read;
    auto yuv_data = read_file(yuv, read);
    if (read != yuv_bytes) return cv::Mat();
    cv::Mat yuv_mat(height + height / 2, width, CV_8UC1, yuv_data.get());
    cv::Mat rgb_mat;
    cvtColor(yuv_mat, rgb_mat,  cv::COLOR_YUV2RGB_NV21);
    return rgb_mat;
}

int main(int argc, const char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " input_yuv output_rgb [threads=4]" << std::endl;
        return -1;
    }

    std::string yuv_path = argv[1];
    std::string rgb_path = argv[2];
    int threads = 4;
    if (argc > 3) {
        threads = atoi(argv[3]);
    }
    if (threads < 0) threads = 0;

    std::cout << "Using " << threads << " thread(s)." << std::endl;

    ohm::Shotgun gun(threads);

    auto all_yuv = glob_files(yuv_path);


    std::cout << "Found " << all_yuv.size() << " files(s)." << std::endl;

    ohm::progress_bar bar(all_yuv.size());
    bar.autostop(false);
    bar.start();

    for (auto &yuv : all_yuv) {
        bar.next();
        bar.wait_show(1000, std::cout);

        gun.fire([&, yuv](int) {
            try {
                std::string ext;
                cut_name_ext(yuv, ext);
                if (ext != "yuv") {
                    std::ostringstream oss;
                    oss << "Skip unknown file: " << yuv << std::endl;
                    std::cout << oss.str();
                    return;
                }
                auto input_path = join_path({yuv_path, yuv});
                auto output_path = join_path({rgb_path, yuv});
                output_path = cut_name_ext(output_path);
                output_path += ".png";

                auto rgb = to_rgb(input_path);

                if (rgb.empty()) {
                    std::ostringstream oss;
                    oss << "Skip broken file: " << yuv << std::endl;
                    std::cout << oss.str();
                    return;
                }

                ohm::mkdir(ohm::cut_path_tail(output_path));
                cv::imwrite(output_path, rgb);
            } catch (const std::exception &e) {
                std::ostringstream oss;
                oss << "Catch exception on file: " << yuv << ". " << e.what() << std::endl;
                std::cout << oss.str();
            }
        });

    }

    gun.join();

    bar.stop();
    bar.show(std::cout) << std::endl;

    std::cout << "All done." << std::endl;

    return 0;
}