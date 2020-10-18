//
// Created by kier on 2020/10/10.
//

#include <fstream>
#include <set>

#include "ohm/pipe/pipe.h"
#include "ohm/print.h"
#include "ohm/range.h"
#include "ohm/type_name.h"
#include "ohm/thread/loop_thread.h"
#include "ohm/list_files.h"
#include "ohm/progress_bar.h"

#include "opencv_pipe_profiler.h"


int main() {
    std::string root = R"(F:\tmp\pipe\input)";
    std::string data_file = root + ohm::file::sep() + "images.txt";
    std::set<std::string> filter = {"jpg", "png", "bmp"};

    std::string output = R"(F:\tmp\pipe\output)";

    // step 1: get file list
    uint64_t count = 0;
    std::ofstream tmp_stream(data_file);
    ohm::for_each_file(
            root,
            [&](const std::string &file) {
                std::string ext;
                ohm::cut_name_ext(file, ext);
                if (filter.find(ext) == filter.end()) return;
                tmp_stream << file << std::endl;
                ++count;
            });
    tmp_stream.close();
    tmp_stream.open(data_file + ".info");
    tmp_stream << root << std::endl;
    tmp_stream << count << std::endl;
    tmp_stream.close();

    ohm::println("Found ", count, " file(s).");

    // step 2: build data stream
    // If there is built image list, use it to get all needed information.
    std::ifstream reader;
    reader.open(data_file + ".info");
    reader >> root >> count;
    reader.close();

    // step 2.1 add progress
    ohm::progress_bar bar(count);
    ohm::PipeStatistics stat;
    bar.autostop(false);
    std::string window_name = "Data Pro";

    // step 2.2 build data stream
    reader.open(data_file);
    auto generator = [&]() -> std::string {
        std::string tmp;
        if (!std::getline(reader, tmp)) throw ohm::PipeBreak();
        bar.next();
        stat.input(bar.value(), bar.max(),
                   std::chrono::duration_cast<ohm::time::ms>(bar.used_time()),
                   std::chrono::duration_cast<ohm::time::ms>(bar.left_time()));
        stat.input_dps(bar.dps());
        return std::move(tmp);
    };

    struct ImageWithPath {
        std::string path;
        cv::Mat image;
        std::vector<uchar> buffer;
    };

    auto image_reader = [&](std::string path) -> ImageWithPath {
        ImageWithPath iwp;
        iwp.path = path;
        iwp.image = cv::imread(root + ohm::file::sep() + path);
        if (iwp.image.empty()) throw ohm::PipeLeak();
        return iwp;
    };
    auto image_encoder = [&](ImageWithPath &image) -> void {
        static const std::string ext = ".jpg";
        cv::imencode(ext, image.image, image.buffer);
        image.path = ohm::cut_name_ext(image.path) + ext;
    };

    auto image_saver = [&](ImageWithPath image) -> void {
        auto output_path = ohm::join_path({output, image.path});
        auto output_root = ohm::cut_path_tail(output_path);
        ohm::mkdir(output_root);
        std::ofstream tmp(output_path, std::ios::binary);
        tmp.write(
                reinterpret_cast<char*>(image.buffer.data()),
                image.buffer.size());
    };

    ohm::Tap<std::string> images(generator);

    images.limit(8)
            .profile("read").map(1, image_reader).limit(8)
            .profile("encode").map(0, image_encoder).limit(8)
            .profile("write").seal(5, image_saver);

    // 2.3 setup stat watcher
    ohm::LoopThread log(1, [&]() {
        auto report = images.report();
        stat.report(report);
        stat.update();
        stat.imshow(window_name);
        cv::waitKey(1);
    });

    // 2.4 main loop
    images.loop();
//    images.generate();
    images.join();


    // 2.5 about to be end0
    bar.stop();
    bar.show(std::cout) << std::endl;
    log.dispose();

    cv::destroyAllWindows();
    ohm::println("All jobs done.");

    // wait window close
    auto summary_window_name = window_name + " Finished";
    while (true) {
        stat.report(images.report());
        stat.update();
        stat.imshow(summary_window_name);
        auto key = cv::waitKey(1000);
        auto flag = cv::getWindowProperty(summary_window_name, cv::WND_PROP_AUTOSIZE);
        if (flag < 1) {
            break;
        }
        if (key == 27) {
            break;
        }
    }

    ohm::println("About to close.");

    return 0;
}


