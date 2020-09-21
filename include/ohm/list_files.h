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
        inline std::vector<std::string> list_files(const std::string &path, std::vector<std::string> *folders = nullptr) {
            std::vector<std::string> result;
            if (folders) folders->clear();
#if OHM_PLATFORM_OS_WINDOWS
            _finddata_t file;
            std::string pattern = path + file::sep() + "*";
            auto handle = _findfirst(pattern.c_str(), &file);

            if (handle == -1L) return result;
            do {
                if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0) continue;
                if (file.attrib & _A_SUBDIR) {
                    if (folders) folders->push_back(file.name);
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
                    if (folders) folders->push_back(file->d_name);
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

    /**
     * @brief list files in `path`.
     * @param path where to list files.
     * @return vector of listed files.
     */
    inline std::vector<std::string> list_files(const std::string &path) {
        return _::list_files(path);
    }

    /**
     * @brief list files and folders in `path`.
     * @param path where to list files and folders.
     * @param [out] folders return vector of listed folders.
     * @return vector of listed files.
     */
    inline std::vector<std::string> list_files(const std::string &path, std::vector<std::string> &folders) {
        return _::list_files(path, &folders);
    }

    /**
     * @brief glob files in path with depth limit.
     * @param path where to list files
     * @param depth glob depth limit, -1 for no limits.
     * @return vector of listed files.
     */
    inline std::vector<std::string> glob_files(const std::string &path, int depth = -1) {
        std::vector<std::string> result;
        std::queue<std::pair<std::string, int> > work;
        std::vector<std::string> folders;
        std::vector<std::string> files = list_files(path, folders);
        result.insert(result.end(), files.begin(), files.end());
        for (auto &folder : folders) work.push({folder, 1});
        while (!work.empty()) {
            auto local_pair = work.front();
            work.pop();
            auto local_path = local_pair.first;
            auto local_depth = local_pair.second;
            if (depth > 0 && local_depth >= depth) continue;
            files = list_files(path + file::sep() + local_path, folders);
            for (auto &file : files) result.push_back(local_path + file::sep() + file);
            for (auto &folder : folders) work.push({local_path + file::sep() + folder, local_depth + 1});
        }
        return result;
    }

    /**
     * @brief glob folders in path with depth limit.
     * @param path where to list folders
     * @param depth glob depth limit, -1 for no limits.
     * @return vector of listed folders.
     */
    inline std::vector<std::string> glob_folders(const std::string &path, int depth = -1) {
        std::vector<std::string> result;
        std::queue<std::pair<std::string, int> > work;
        std::vector<std::string> folders;
        std::vector<std::string> files = list_files(path, folders);
        result.insert(result.end(), folders.begin(), folders.end());
        for (auto &folder : folders) work.push({folder, 1});
        while (!work.empty()) {
            auto local_pair = work.front();
            work.pop();
            auto local_path = local_pair.first;
            auto local_depth = local_pair.second;
            if (depth > 0 && local_depth >= depth) continue;
            files = list_files(path + file::sep() + local_path, folders);
            for (auto &folder : folders) result.push_back(local_path + file::sep() + folder);
            for (auto &folder : folders) work.push({local_path + file::sep() + folder, local_depth + 1});
        }
        return result;
    }
}

#endif //OMEGA_LIST_FILES_H
