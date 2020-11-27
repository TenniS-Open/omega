//
// Created by kier on 2020/9/6.
//

#ifndef OMEGA_FILE_SYS_H
#define OMEGA_FILE_SYS_H

#include "format.h"
#include "print.h"
#include "platform.h"
#include "macro.h"

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <regex>

#pragma push_macro("ACCESS")
#pragma push_macro("MKDIR")
#pragma push_macro("GETCWD")
#pragma push_macro("CHDIR")

#if OHM_PLATFORM_OS_WINDOWS

#include <direct.h>
#include <io.h>

#define ACCESS ::_access
#define MKDIR(a) ::_mkdir((a))
#define GETCWD(buffer, length) ::_getcwd((buffer), (length))
#define CHDIR(path) ::_chdir(path)

#include "sys/windows.h"
#include <sys/stat.h>

#elif  OHM_PLATFORM_OS_LINUX || OHM_PLATFORM_OS_MAC || OHM_PLATFORM_OS_IOS

#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <fstream>

#include <string.h>

#define ACCESS ::access
#define MKDIR(a) ::mkdir((a),0755)
#define GETCWD(buffer, length) ::getcwd((buffer), (length))
#define CHDIR(path) ::chdir(path)

#endif

namespace ohm {
    namespace file {
        inline std::string sep() {
#if OHM_PLATFORM_OS_WINDOWS
            return "\\";
#else
            return "/";
#endif
        }
    }

    namespace _ {
        inline bool mkdir_core(const std::string &dir) {
            int miss = ACCESS(dir.c_str(), 0);
            if (miss) {
                int failed = MKDIR(dir.c_str());
                if (failed) {
                    return false;
                }
            }
            return true;
        }
    }

    inline bool mkdir(const std::string &dir) {
        auto path = split(dir, "\\/");
        for (size_t i = 1; i <= path.size(); ++i) {
            if (path[i - 1].empty()) continue;
            auto local_path = join(std::vector<std::string>(path.begin(), path.begin() + i), file::sep());
            if (!_::mkdir_core(local_path)) return false;
        }
        return true;
    }

    inline bool access(const std::string &path) {
        int miss = ACCESS(path.c_str(), 0);
        return !miss;
    }

    inline bool remove(const std::string &filename) {
        return std::remove(filename.c_str()) == 0;
    }

    inline bool rename(const std::string &oldname, const std::string &newname) {
        return std::rename(oldname.c_str(), newname.c_str()) == 0;
    }

    inline bool copy(const std::string &fromfile, const std::string &tofile, bool force) {
#if OHM_PLATFORM_OS_WINDOWS
        return CopyFileA(fromfile.c_str(), tofile.c_str(), !force) != FALSE;
#elif OHM_PLATFORM_OS_LINUX
        return std::system(sprint(force ? "cp -f " : "cp ", fromfile, ' ', tofile).c_str()) == 0;
#else
        std::ifstream input(fromfile, std::ios::binary);
        std::ofstream output(tofile, std::ios::binary);
        output << input.rdbuf();
        return true;
#endif
    }

    inline std::string getcwd() {
        auto pwd = GETCWD(nullptr, 0);
        if (pwd == nullptr) return std::string();
        std::string pwd_str = pwd;
        free(pwd);
        return std::move(pwd_str);
    }

    inline std::string getself() {
#if OHM_PLATFORM_OS_WINDOWS
        char exed[1024];
        auto exed_size = sizeof(exed) / sizeof(exed[0]);
        auto link_size = GetModuleFileNameA(nullptr, exed, DWORD(exed_size));

        if (link_size <= 0) return std::string();

        return std::string(exed, exed + link_size);
#else
        char exed[1024];
        auto exed_size = sizeof(exed) / sizeof(exed[0]);

        auto link_size = readlink("/proc/self/exe", exed, exed_size);

        if (link_size <= 0) return std::string();

        return std::string(exed, exed + link_size);
#endif
    }

    inline std::string cut_path_tail(const std::string &path);

    inline std::string cut_path_tail(const std::string &path, std::string &tail);

    inline std::string cut_name_ext(const std::string &name_ext, std::string &ext);

    inline std::string getexed() {
        auto self = getself();
        return cut_path_tail(self);
    }

    /**
	 * \brief Get current .dll or .so directory
         * \param model_name: will auto add suffix.
         * \ example: windows: module is mytest.dll or mytestd.dll, then model_name="mytest", 'd' will auto add
         * \          linux:   module is libmytest.so or libmytestd.so, then model_name = 'mytest", the "lib' and 'd' will auto add
	 * \return return current module absolute path,
         * \ example: linux module path: "/usr/local/lib/libmytest.so", return "/usr/local/lib"
         * \          window module path: "c:\windows\mytest.dll", return "c:\windows"
	 */
    inline std::string getmodule_root(const std::string &module_name) {
        std::string ret;

        char sLine[2048] = {0};
        std::string model_named = module_name + "d";
#if OHM_PLATFORM_OS_WINDOWS
        HMODULE hmodule = GetModuleHandleA(module_name.c_str());
        if (!hmodule) {
            hmodule = GetModuleHandleA(model_named.c_str());
            if (!hmodule) {
                return "";
            }
        }

        int num = GetModuleFileNameA(hmodule, sLine, sizeof(sLine));
        std::string tmp(sLine, num);
        std::string name;
        ret = cut_path_tail(tmp, name);
        return ret;
#else
        void *pSymbol = (void *) "";
        FILE *fp;
        char *pPath;
        std::string libname = "lib" + module_name;
        std::string libnamed = libname + "d";

        fp = fopen("/proc/self/maps", "r");
        if (fp != NULL) {
            while (!feof(fp)) {
                unsigned long start, end;

                if (!fgets(sLine, sizeof(sLine), fp))
                    continue;
                if (!strstr(sLine, " r-xp ") || !strchr(sLine, '/'))
                    continue;

                sscanf(sLine, "%lx-%lx ", &start, &end);
                if (pSymbol >= (void *) start && pSymbol < (void *) end) {
                    char *tmp;
                    size_t len;

                    pPath = strchr(sLine, '/');

                    tmp = strrchr(pPath, '\n');
                    if (tmp) *tmp = 0;

                    len = strlen(pPath);
                    if (len > 10 && strcmp(pPath + len - 10, " (deleted)") == 0) {
                        tmp = pPath + len - 10;
                        *tmp = 0;
                    }

                    std::string name;
                    std::string ext;
                    ret = cut_path_tail(pPath, name);
                    name = cut_name_ext(name, ext);
                    if (name == module_name || name == libname || name == model_named || name == libnamed) {
                        fclose(fp);
                        return ret;
                    }
                }
            }
            fclose(fp);
        }
        return "";
#endif
    }

    inline bool cd(const std::string &path) {
        return CHDIR(path.c_str()) == 0;
    }

    inline std::string cut_path_tail(const std::string &path) {
        std::string tail;
        return cut_path_tail(path, tail);
    }

    inline std::string cut_path_tail(const std::string &path, std::string &tail) {
        auto win_sep_pos = path.rfind('\\');
        auto unix_sep_pos = path.rfind('/');
        auto sep_pos = win_sep_pos;
        if (sep_pos == std::string::npos) sep_pos = unix_sep_pos;
        else if (unix_sep_pos != std::string::npos && unix_sep_pos > sep_pos) sep_pos = unix_sep_pos;
        if (sep_pos == std::string::npos) {
            tail = path;
            return std::string();
        }
        tail = path.substr(sep_pos + 1);
        return path.substr(0, sep_pos);
    }

    inline std::string cut_name_ext(const std::string &name_ext, std::string &ext) {
        auto dot_pos = name_ext.rfind('.');
        auto sep_pos = dot_pos;
        if (sep_pos == std::string::npos) {
            ext = std::string();
            return name_ext;
        }
        ext = name_ext.substr(sep_pos + 1);
        return name_ext.substr(0, sep_pos);
    }

    inline std::string cut_name_ext(const std::string &name_ext) {
        auto dot_pos = name_ext.rfind('.');
        auto sep_pos = dot_pos;
        if (sep_pos == std::string::npos) {
            return name_ext;
        }
        return name_ext.substr(0, sep_pos);
    }

    inline bool isdir(const std::string &path) {
        struct stat buf;
        if (stat(path.c_str(), &buf)) return false;
        return bool((S_IFDIR & buf.st_mode) != 0);
    }

    inline bool isfile(const std::string &path) {
        struct stat buf;
        if (stat(path.c_str(), &buf)) return false;
        return bool((S_IFREG & buf.st_mode) != 0);
    }

    inline std::string join_path(const std::vector<std::string> &paths) {
        return join(paths, file::sep());
    }

    class stack_cd {
    public:
        using self = stack_cd;

        stack_cd(const stack_cd &) = delete;

        stack_cd &operator=(const stack_cd &) = delete;

        explicit stack_cd(const std::string &path)
                : precwd(getcwd()) {
            cd(path);
        }

        ~stack_cd() {
            cd(precwd);
        }

    private:
        std::string precwd;
    };

    static bool is_absolute(const std::string &path) {
        std::regex abs(R"((^[\\/].*$)|(^[A-Za-z]+:[\\/].*$))");
        return std::regex_match(path, abs);
    }

    static std::string get_absolute(const std::string &root, const std::string &path) {
        if (root.empty() || is_absolute(path)) return path;
        if (root.back() == '\\' || root.back() == '/') return root + path;
        return join_path({root, path});
    }
}

#define ohm_cd(path) ohm::stack_cd ohm_auto_name(__stack_cd_)(path)

#pragma pop_macro("ACCESS")
#pragma pop_macro("MKDIR")
#pragma pop_macro("GETCWD")
#pragma pop_macro("CHDIR")


#endif //OMEGA_FILE_SYS_H
