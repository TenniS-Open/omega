//
// Created by kier on 2020/9/21.
//

#ifndef OMEGA_LIST_FILES_H
#define OMEGA_LIST_FILES_H

#include <vector>
#include <string>
#include <iostream>
#include <queue>

#include "filesys.h"
#include "platform.h"

#if OHM_PLATFORM_OS_WINDOWS

#include <io.h>

#else
#include <dirent.h>
#include <cstring>
#endif

namespace ohm {
    namespace _ {
        inline std::vector<std::string> list_files(const std::string &path, std::vector<std::string> *dirs = nullptr) {
            std::vector<std::string> result;
            if (dirs) dirs->clear();
#if OHM_PLATFORM_OS_WINDOWS
            _finddata_t file;
            std::string pattern = path + file::sep() + "*";
            auto handle = _findfirst(pattern.c_str(), &file);

            if (handle == -1L) return result;
            do {
                if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0) continue;
                if (file.attrib & _A_SUBDIR) {
                    if (dirs) dirs->push_back(file.name);
                } else {
                    result.push_back(file.name);
                }
            } while (_findnext(handle, &file) == 0);

            _findclose(handle);
#else
            struct dirent *file;

            auto handle = opendir(path.c_str());

            if (handle == nullptr) return result;

            while ((file = readdir(handle)) != nullptr)
            {
                if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0) continue;
                if (file->d_type & DT_DIR)
                {
                    if (dirs) dirs->push_back(file->d_name);
                }
                else if (file->d_type & DT_REG)
                {
                    result.push_back(file->d_name);
                }
                // DT_LNK // for linkfiles
            }

            closedir(handle);
#endif
            return std::move(result);
        }
    }

    inline std::vector<std::string> list_files(const std::string &path) {
        return _::list_files(path);
    }

    inline std::vector<std::string> list_files(const std::string &path, std::vector<std::string> &dirs) {
        return _::list_files(path, &dirs);
    }

    inline std::vector<std::string> glob_files(const std::string &path, int depth = -1) {
        std::vector<std::string> result;
        std::queue<std::pair<std::string, int> > work;
        std::vector<std::string> dirs;
        std::vector<std::string> files = list_files(path, dirs);
        result.insert(result.end(), files.begin(), files.end());
        for (auto &dir : dirs) work.push({dir, 1});
        while (!work.empty()) {
            auto local_pair = work.front();
            work.pop();
            auto local_path = local_pair.first;
            auto local_depth = local_pair.second;
            if (depth > 0 && local_depth >= depth) continue;
            files = list_files(path + file::sep() + local_path, dirs);
            for (auto &file : files) result.push_back(local_path + file::sep() + file);
            for (auto &dir : dirs) work.push({local_path + file::sep() + dir, local_depth + 1});
        }
        return result;
    }

    inline std::vector<std::string> glob_folders(const std::string &path, int depth = -1) {
        std::vector<std::string> result;
        std::queue<std::pair<std::string, int> > work;
        std::vector<std::string> dirs;
        std::vector<std::string> files = list_files(path, dirs);
        result.insert(result.end(), dirs.begin(), dirs.end());
        for (auto &dir : dirs) work.push({dir, 1});
        while (!work.empty()) {
            auto local_pair = work.front();
            work.pop();
            auto local_path = local_pair.first;
            auto local_depth = local_pair.second;
            if (depth > 0 && local_depth >= depth) continue;
            files = list_files(path + file::sep() + local_path, dirs);
            for (auto &dir : dirs) result.push_back(local_path + file::sep() + dir);
            for (auto &dir : dirs) work.push({local_path + file::sep() + dir, local_depth + 1});
        }
        return result;
    }
}

#endif //OMEGA_LIST_FILES_H
