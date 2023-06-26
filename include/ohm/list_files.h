//
// Created by kier on 2020/9/21.
//

#ifndef OMEGA_LIST_FILES_H
#define OMEGA_LIST_FILES_H

#include <vector>
#include <string>
#include <iostream>
#include <queue>

#include "need.h"
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
            return result;
        }


        inline void for_each_file_or_folder(const std::string &path,
                                            const std::function<void(std::string)> &for_file,
                                            const std::function<void(std::string)> &for_folder) {
            std::vector<std::string> result;
#if OHM_PLATFORM_OS_WINDOWS
            _finddata_t file;
            std::string pattern = path + file::sep() + "*";
            auto handle = _findfirst(pattern.c_str(), &file);
            if (handle == -1L) return;
            ohm_need(_findclose, handle);

            do {
                if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0) continue;
                if (file.attrib & _A_SUBDIR) {
                    if (for_folder) for_folder(file.name);
                } else {
                    if (for_file) for_file(file.name);
                }
            } while (_findnext(handle, &file) == 0);
#else
            struct dirent *file;
            auto handle = opendir(path.c_str());
            if (handle == nullptr) return;
            ohm_need(closedir, handle);

            while ((file = readdir(handle)) != nullptr) {
                if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0) continue;
                if (file->d_type & DT_DIR) {
                    if (for_folder) for_folder(file->d_name);
                } else if (file->d_type & DT_REG) {
                    if (for_file) for_file(file->d_name);
                }
                // DT_LNK // for linkfiles
            }
#endif
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

    inline void for_each_file(const std::string &root,
                       const std::function<void(std::string)> &for_file,
                       int depth = -1) {
        std::queue<std::pair<std::string, int>> work;
        _::for_each_file_or_folder(
                root,
                [&](std::string file){
                    for_file(file);
                },
                [&](std::string folder){
                    work.push(std::make_pair(folder, 1));
                });
        while (!work.empty()) {
            auto local_pair = work.front();
            work.pop();
            auto local_path = local_pair.first;
            auto local_depth = local_pair.second;
            if (depth > 0 && local_depth >= depth) continue;
            _::for_each_file_or_folder(
                    root + file::sep() + local_path,
                    [&](std::string file) {
                        for_file(local_path + file::sep() + file);
                    },
                    [&](std::string folder) {
                        auto next_folder = local_path + file::sep() + folder;
                        auto next_depth = local_depth + 1;
                        if (depth > 0 && next_depth >= depth) return;
                        work.push({next_folder, next_depth});
                    });
        }
    }

    inline void for_each_folder(const std::string &root,
                         const std::function<void(std::string)> &for_folder,
                         int depth = -1) {
        std::queue<std::pair<std::string, int>> work;
        _::for_each_file_or_folder(
                root,
                nullptr,
                [&](std::string folder) {
                    for_folder(folder);
                    work.push(std::make_pair(folder, 1));
                });
        while (!work.empty()) {
            auto local_pair = work.front();
            work.pop();
            auto local_path = local_pair.first;
            auto local_depth = local_pair.second;
            if (depth > 0 && local_depth >= depth) continue;
            _::for_each_file_or_folder(
                    root + file::sep() + local_path,
                    nullptr,
                    [&](std::string folder) {
                        auto next_folder = local_path + file::sep() + folder;
                        auto next_depth = local_depth + 1;
                        for_folder(next_folder);
                        if (depth > 0 && next_depth >= depth) return;
                        work.push({next_folder, next_depth});
                    });
        }
    }
}

#endif //OMEGA_LIST_FILES_H
