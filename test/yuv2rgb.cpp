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

cv::Mat do_nv21_to_rgb(char *yuv_data, int width, int height) {
    cv::Mat yuv_mat(height + height / 2, width, CV_8UC1, yuv_data);
    cv::Mat rgb_mat;
    cvtColor(yuv_mat, rgb_mat,  cv::COLOR_YUV2RGB_NV21);
    return rgb_mat;
}

cv::Mat do_planar_to_packed(char *yuv_data, int width, int height) {
    auto count = width * height;
    cv::Mat b_mat(height, width, CV_8UC1, yuv_data);
    cv::Mat g_mat(height, width, CV_8UC1, yuv_data + count);
    cv::Mat r_mat(height, width, CV_8UC1, yuv_data + 2 * count);
    cv::Mat rgb_mat;
    cv::merge(std::vector<cv::Mat>({b_mat, g_mat, r_mat}), rgb_mat);
    return rgb_mat;
}

cv::Mat do_yuyv_to_rgb(char *yuv_data, int width, int height) {
    cv::Mat yuv_mat(height, width, CV_8UC2, yuv_data);
    cv::Mat rgb_mat;
    cvtColor(yuv_mat, rgb_mat,  cv::COLOR_YUV2RGB_YUYV);
    return rgb_mat;
}

cv::Mat left_rotate_90(const cv::Mat &img) {
    cv::Mat tmp = img;
    cv::transpose(tmp, tmp);
    cv::flip(tmp, tmp, 0);
    return tmp;
}

cv::Mat left_rotate_180(const cv::Mat &img) {
    cv::Mat tmp = img;
    cv::flip(tmp, tmp, -1);
    return tmp;
}

cv::Mat right_rotate_90(const cv::Mat &img) {
    cv::Mat tmp = img;
    cv::transpose(tmp, tmp);
    cv::flip(tmp, tmp, 1);
    return tmp;
}

cv::Mat rotate(const cv::Mat &img, int left_rotate) {
    while (left_rotate < 0) left_rotate += 360;
    switch (left_rotate) {
        case 0:
            return img;
        case 90:
            return left_rotate_90(img);
        case 180:
            return left_rotate_180(img);
        case 270:
            return right_rotate_90(img);
        default:
            return cv::Mat();
    }
}

cv::Mat to_rgb(const std::string &yuv, int width, int height, int left_rotate = 0) {
//    static const int height = 1080;
//    static const int width = 1920;
    const int yuv_nv21_bytes = (height + height / 2) * width;
    const int yuv_yuyv_bytes = height * width * 2;
    const int planar_bytes = height * width * 3;
    size_t read;
    auto yuv_data = read_file(yuv, read);
    // std::cout << "read(" << read << "), want(" << yuv_nv21_bytes << "), maybe(" << yuv_yuyv_bytes << ")" << std::endl;
    cv::Mat rgb;
    if (read == yuv_yuyv_bytes) {
        rgb = do_yuyv_to_rgb(yuv_data.get(), width, height);
    }
    if (read == yuv_nv21_bytes) {
        rgb = do_nv21_to_rgb(yuv_data.get(), width, height);
    }
    if (read == planar_bytes) {
        rgb = do_planar_to_packed(yuv_data.get(), width, height);
    }
    if (!rgb.empty()) {
        rgb = rotate(rgb, left_rotate);
    }
    return rgb;
}

int main(int argc, const char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " input_yuv output_rgb [threads=4] [width=1920] [height=1080] [left_rotate=0]" << std::endl;
        return -1;
    }

    std::string yuv_path = argv[1];
    std::string rgb_path = argv[2];
    int threads = 4;
    int rgb_width = 1920;
    int rgb_height = 1080;
    int left_rotate = 0;
    if (argc > 3) {
        threads = atoi(argv[3]);
    }
    if (argc > 4) {
        rgb_width = atoi(argv[4]);
    }
    if (argc > 5) {
        rgb_height = atoi(argv[5]);
    }
    if (argc > 6) {
        left_rotate = atoi(argv[6]);
    }
    if (threads < 0) threads = 0;
    while (left_rotate < 0) left_rotate += 360;

    if ((left_rotate % 90) != 0) {
        std::cerr << "Left rotate must be times of 90, got " << argv[6] << std::endl;
        return 1;
    }

    std::cout << "Using " << threads << " thread(s)." << std::endl;
    std::cout << "Image size " << rgb_width << " x " << rgb_height << std::endl;
    std::cout << "Image left rotate " << (argc > 6 ? argv[6] : "0") << std::endl;

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

                auto rgb = to_rgb(input_path, rgb_width, rgb_height, left_rotate);

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