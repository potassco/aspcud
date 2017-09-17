// {{{ MIT License

// Copyright 2017 Roland Kaminski

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

// }}}

//////////////////// Preamble ///////////////////////// {{{1

#ifndef _MSC_VER
#   include <unistd.h>
#endif
#include "options.hh"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <csignal>
#include <cassert>
#include <fcntl.h>
#ifdef _WIN32
#   define NOMINMAX
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
/*
#   ifdef _MSC_VER
#       include <io.h>
// why _W64???
typedef _W64 SSIZE_T ssize_t;
typedef int mode_t;
#       define open _open
#   endif
*/
#else
#   include <sys/wait.h>
#   include <libgen.h>
#endif
#ifdef __APPLE__
#   include <mach-o/dyld.h>
#endif
#include <sys/stat.h>
#include <cudf/version.hh>

#ifndef ASPCUD_DEFAULT_ENCODING
#define ASPCUD_DEFAULT_ENCODING "specification.lp"
#endif
#ifndef ASPCUD_CUDF2LP_BIN
#define ASPCUD_CUDF2LP_BIN "cudf2lp"
#endif
#ifndef ASPCUD_GRINGO_BIN
#define ASPCUD_GRINGO_BIN "gringo"
#endif
#ifndef ASPCUD_CLASP_BIN
#define ASPCUD_CLASP_BIN "clasp"
#endif

//////////////////// aspcud /////////////////////////// {{{1

class Aspcud {
private:
    std::string tempdir_;
    std::string encoding_    = ASPCUD_DEFAULT_ENCODING;
    std::string cudf2lp_bin_ = ASPCUD_CUDF2LP_BIN;
    std::string gringo_bin_  = ASPCUD_GRINGO_BIN;
    std::string clasp_bin_   = ASPCUD_CLASP_BIN;

    std::string cudf2lp_out_;
    std::string cudf2lp_err_;
    std::string gringo_out_;
    std::string gringo_err_;
    std::string clasp_out_;
    std::string clasp_err_;

    bool debug_ = false;

#ifdef _WIN32
    int interrupted_               = 0;
    unsigned long current_pid_     = 0;
    unsigned long interrupted_pid_ = 0;
#else
    pid_t pid_             = 0;
    pid_t current_pid_     = 0;
    pid_t interrupted_pid_ = 0;
#endif

public:
    static Aspcud *app() {
        static Aspcud app;
        return &app;
    }

int run(int argc, char *argv[]) {
    signal(SIGTERM, &interrupt_);
#ifndef _WIN32
    signal(SIGUSR1, &interrupt_);
#endif
    signal(SIGINT,  &interrupt_);

    set_tempdir_();
    expand_path_(encoding_, argv[0]);
    expand_path_(cudf2lp_bin_, argv[0]);
    expand_path_(gringo_bin_, argv[0]);
    expand_path_(clasp_bin_, argv[0]);

    bool help = false;
    bool version = false;

    std::string criteria = "paranoid";
    std::vector<std::string> inputs;
    std::vector<std::string> cudf2lp_args{cudf2lp_bin_};
    std::vector<std::string> clasp_args{clasp_bin_, "-q1,2", "--stats=2"};
    std::vector<std::string> gringo_args{gringo_bin_, "-Wnone"};
    std::vector<std::string> gringo_encodings;
    auto clasp_args_size = clasp_args.size();

    Options options;

    std::vector<std::string> clasp_args_default{"--opt-strategy=5"};
    std::string clasp_default_args_str;
    for (auto &arg : clasp_args_default) {
        if (!clasp_default_args_str.empty()) {
            clasp_default_args_str.push_back(' ');
        }
        clasp_default_args_str += arg;
    }

    options.group("Aspcud Options");
    options.add(inputs, "argument", "positional arguments", "arg", 3, 0, true);
    options.add(criteria, "c,criteria", "optimization criteria");

    options.add(clasp_args, "s,solver-option", "append argument for solver", clasp_default_args_str.c_str(), "arg", 0);
    options.add(gringo_args, "g,grounder-option", "append argument for grounder", "arg", 0);
    options.add(gringo_encodings, "e,encoding", "append encoding for grounder", "enc", 0);
    options.add(cudf2lp_args, "p,preprocessor-option", "append argument for preprocessor", "arg", 0);

    options.add(clasp_bin_, "S,solver", "path to solver");
    options.add(gringo_bin_, "G,grounder", "path to grounder");
    options.add(cudf2lp_bin_, "P,preprocessor", "path to preprocessor");

    options.group("Basic Options");
    options.add(help, "h,help", "print help information");
    options.add(version, "v,version", "print version information");

    options.parse(argc, argv);

    if (help) {
        print_usage(argv[0]);
        std::cout << options.description() <<
            "The optimization criteria can be passed as third argument or via option\n"
            "--criteria. To get a list of supported criteria, call:\n"
            "  " << cudf2lp_bin_ << " --help\n"
            "\n"
            "If argument CUDFOUT is not given, the solution is printed to stdout. If\n"
            "argument CUDFIN is not given, input is read from stdin.\n"
            "\n"
            "TEMPDIR: " << tempdir_ << std::endl;
        return 0;
    }
    else if (version) {
        std::cout << "aspcud" << " version " << CUDF_VERSION << "\n\n";
        std::cout << "License: The MIT License <https://opensource.org/licenses/MIT>" << std::endl;
        return 0;
    }
    else {
        if (options.assigned("criteria") > 0 && inputs.size() >= 3) {
            throw OptionsException("multiple values for criteria");
        }
        else if (inputs.size() >= 3) {
            criteria = inputs[2];
        }
    }

    if (clasp_args_size == clasp_args.size()) {
        clasp_args.insert(clasp_args.end(), clasp_args_default.begin(), clasp_args_default.end());
    }

    if (gringo_encodings.empty()) {
        gringo_args.emplace_back("-f");
        gringo_args.emplace_back(encoding_);
    }

    setmain_();
    if (atexit(&atexit_) != 0) {
        throw std::runtime_error("could not set exithandler\n");
    }

    cudf2lp_out_ = tempfile_("cudf2lp.outXXXXXX");
    cudf2lp_err_ = tempfile_("cudf2lp.errXXXXXX");
    gringo_out_ = tempfile_("gringo.outXXXXXX");
    gringo_err_ = tempfile_("gringo.errXXXXXX");
    clasp_out_ = tempfile_("clasp.outXXXXXX");
    clasp_err_ = tempfile_("clasp.errXXXXXX");

    cudf2lp_args.emplace_back("-f");
    cudf2lp_args.emplace_back(inputs.size() > 0 ? inputs[0] : "-");
    cudf2lp_args.emplace_back("-c");
    cudf2lp_args.emplace_back(criteria);
    int cudf2lp_status = exec_(cudf2lp_args, cudf2lp_out_, cudf2lp_err_);
    aspcud_ecat(cudf2lp_err_);
    if (cudf2lp_status != 0) {
        throw std::runtime_error("preprocessor returned with non-zero exit status\n");
    }

    // run gringo
    gringo_args.emplace_back("-f");
    gringo_args.emplace_back(cudf2lp_out_);
    int gringo_status = exec_(gringo_args, gringo_out_, gringo_err_);
    aspcud_ecat(gringo_err_);
    if (gringo_status != 0) {
        throw std::runtime_error("grounder returned with non-zero exit status\n");
    }

    // run clasp
    clasp_args.emplace_back("-f");
    clasp_args.emplace_back(gringo_out_);
    int clasp_status = exec_(clasp_args, clasp_out_, clasp_err_);
    // TODO: is it possible to do something with the exit status of clasp???
    (void)clasp_status;
    aspcud_ecat(clasp_err_);

    // find answer set
    FILE *fclasp_out  = fopen(clasp_out_, "r");
    if (!fclasp_out) {
        fprintf(stderr, "error: could not open %s (%s)\n", clasp_out_.c_str(), strerror(errno));
        exit(1);
    }
    size_t  line_length = 0;
    size_t  solution_length = 0;
    char   *line = NULL;
    char   *solution = NULL;
    ssize_t read;
    int     next = 0;
    while ((read = aspcud_getline(&line, &line_length, fclasp_out)) != -1) {
        if (read > 0 && line[read-1] == '\n') {
            line[read-1] = '\0';
        }
        printf("%.80s\n", line); fflush(stdout);
        if (next == 1) {
            char *tmp = solution;
            solution  = line;
            line      = tmp;
            next      = 0;
            size_t tmp_length = solution_length;
            solution_length   = line_length;
            line_length       = tmp_length;
        }
        else if (strncmp("Answer:", line, 7) == 0) {
            next = 1;
        }
    }
    if (read == -1 && !feof(fclasp_out)) {
        fprintf(stderr, "error: reading %s failed (%s)\n", clasp_out_, strerror(errno));
        exit(1);
    }
    fclose(fclasp_out);

    // rewrite the solution
    FILE *faspcud_out = fopen(aspcud_out, "w");
    if (!faspcud_out) {
        fprintf(stderr, "error: could not open %s (%s)\n", aspcud_out, strerror(errno));
        exit(1);
    }
    if (solution) {
        while ((solution = strstr(solution, "in(\""))) {
            solution+= 4;
            char *comma = strchr(solution, ',');
            if (!comma) {
                fprintf(stderr, "error: unexpected output\n");
                exit(1);
            }
            *(comma++-1) = '\0';
            char *paren = strchr(comma, ')');
            if (!paren) {
                fprintf(stderr, "error: unexpected output\n");
                exit(1);
            }
            *paren++ = '\0';
            if (fprintf(faspcud_out, "package: %s\nversion: %s\ninstalled: true\n\n", solution, comma) < 0) {
                fprintf(stderr, "error: could not write to %s (%s)\n", aspcud_out, strerror(errno));
                exit(1);
            }
            solution = paren;
        }
    }
    else {
        if (fprintf(faspcud_out, "FAIL\n") < 0) {
            fprintf(stderr, "error: could not write to %s (%s)\n", aspcud_out, strerror(errno));
            exit(1);
        }
    }
    if (fclose(faspcud_out)) {
        fprintf(stderr, "error: could not close %s (%s)\n", aspcud_out, strerror(errno));
        exit(1);
    }

    return EXIT_SUCCESS;
}

private:
    // if path starts with <module_prefix> replace this prefix with the
    // directory name of the aspcud executable
    // the function receives argv[0] in parameter module_path as a hint
    // which is used if no better method is available on a platform
    void expand_path_(std::string &path, std::string module_path) {
        char const *prefix = "<module_path>";
        if (strncmp(path.c_str(), prefix, strlen(prefix)) == 0) {
            path.erase(path.begin(), path.begin() + strlen(prefix));
#if defined(_WIN32)
            char module_filename[MAX_PATH+1];
            size_t length = GetModuleFileName(nullptr, module_filename, MAX_PATH+1);
            if (!length || length >= MAX_PATH) {
                throw std::runtime_error("module file path too long");
            }
            std::vector<char> buf(MAX_PATH+1);
            length = GetFullPathName(module_filename, buf.size(), buf.data(), nullptr);
            if (!length || length >= MAX_PATH) {
                throw std::runtime_error("module file path too long");
            }
            module_path.assign(buf.begin(), buf.end());
#else
#   if defined(__linux__)
            module_path = "/proc/self/exe";
#   elif defined(__APPLE__)
            uint32_t length = 0;
            _NSGetExecutablePath(NULL, &length);
            if (!length) {
                throw std::runtime_error("could not get executable path");
            }
            std::vector<char> buf(length + 1);
            if (_NSGetExecutablePath(buf.data(), &length) < 0) {
                throw std::runtime_error("could not get executable path");
            }
            module_path.assign(buf.begin(), buf.end());
#endif // freebsd, openbsd, ...
            struct stat sb;
            if (lstat(module_path.c_str(), &sb) == -1) {
                throw std::runtime_error("could not lstat file");
            }
            std::vector<char> linkname;
            if (S_ISLNK(sb.st_mode)) {
                if (sb.st_size == 0) { sb.st_size = 1024; }
                linkname.resize(sb.st_size + 1);
                ssize_t r = readlink(module_path.c_str(), linkname.data(), linkname.size());
                if (r < 0 || r > sb.st_size) {
                    throw std::runtime_error("could not read link");
                }
                linkname[sb.st_size] = '\0';
            }
            else {
                linkname.assign(module_path.begin(), module_path.end());
                linkname.push_back('\0');
            }
            module_path = dirname(linkname.data());
#endif
            path.insert(path.begin(), module_path.begin(), module_path.end());
        }
    }

    void set_tempdir_() {
#ifdef _WIN32
        char buf[MAX_PATH+1];
        unsigned length = GetTempPath(MAX_PATH+1, buf);
        if (!length || length >= MAX_PATH) {
            throw std::runtime_error("could not get TEMP\n");
        }
        tempdir_ = buf;
#else
        char const *tmpdir = std::getenv("TMPDIR");
        if (!tmpdir) { tmpdir = P_tmpdir; }
        tempdir_ = tmpdir;
        tempdir_.push_back('/');
#endif
    }

    void setmain_() {
#ifndef _WIN32
        pid_ = getpid();
#endif
    }

    bool ismain_() {
#ifdef _WIN32
        return true;
#else
        return pid_ == getpid();
#endif
    }

    static void interrupt_(int signal) {
#ifdef _WIN32
        if (!app()->interrupted_) {
            //AttachConsole(current_pid_);
            //SetConsoleCtrlHandler(0, 1);
            GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
            app()->interrupted_pid_ = current_pid_;
        }
        app()->interrupted_ = 1;
#else
        if (app()->current_pid_ > 0) {
            kill(app()->current_pid_, signal);
            app()->interrupted_pid_ = app()->current_pid_;
        }
        else { app()->interrupted_pid_ = -1; }
#endif
    }

#ifdef _WIN32
    // See http://blogs.msdn.com/b/twistylittlepassagesallalike/archive/2011/04/23/everyone-quotes-arguments-the-wrong-way.aspx
    void append_quoted_(std::string const &arg, std::string &cmdline) {
        if (!strpbrk(arg.c_str(), " \t\n\v\"")) { cmdline+= arg; }
        else {
            cmdline.push_back('\"');
            for (auto it = arg.c_str(); *it; ++it) {
                int slashes = 0;
                for (; *it == '\\'; ++it, ++slashes) { cmdline.push_back('\\'); }
                if (!*it || *it == '"') {
                    for (; slashes > 0; --slashes) { cmdline.push_back('\\'); }
                    if (*it == '"') { cmdline.push_back('\\'); }
                    else            { break; }
                }
                cmdline.push_back(*it);
            }
            cmdline.push_back('\"');
        }
    }

    std::string build_commandline_(std::vector<std::string> const &args) {
        std::string cmdline;
        for (auto &x : args) {
            if (!cmdline.empty()) {
                cmdline.push_back(' ');
            }
            append_quoted_(x, cmdline);
        }
        return cmdline;
    }
#endif

    int exec_(std::vector<std::string> const &args, std::string const &out_path, std::string const &err_path) {
        if (debug_) {
            std::cerr << "debug: starting process";
            for (auto &arg : args) {
                std::cerr << " " << arg;
            }
            std::cerr << std::endl;
        }
#ifdef _WIN32
        if (interrupted_) { return 1; }
        // file descriptors returned by open are inheritable
        // http://msdn.microsoft.com/en-us/library/z0kc8e3z.aspx
        mode_t mode = _S_IREAD | _S_IWRITE;
        int out_fd = open(out_path, O_WRONLY | O_CREAT | O_TRUNC, mode);
        if (out_fd == -1) {
            fprintf(stderr, "error: could not open %s (%s)\n", out_path, strerror(errno));
            exit(1);
        }
        int err_fd = open(err_path, O_RDWR | O_CREAT | O_TRUNC, mode);
        if (err_fd == -1) {
            fprintf(stderr, "error: could not open %s (%s)\n", err_path, strerror(errno));
            exit(1);
        }

        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

        STARTUPINFO si;
        ZeroMemory(&si, sizeof(STARTUPINFO));
        si.cb         = sizeof(STARTUPINFO);
        si.dwFlags    = STARTF_USESTDHANDLES;
        si.hStdOutput = (HANDLE*)_get_osfhandle(out_fd);
        si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
        si.hStdError  = (HANDLE*)_get_osfhandle(err_fd);

        std::string cmdline = build_commandline_(args);
        if (!CreateProcess(args[0], cmdline.c_str(), 0, 0, 1, 0, 0, 0, &si, &pi)) {
            fprintf(stderr, "error: could not execute %s\n", cmdline.c_str());
            exit(1);
        }

        current_pid_ = pi.dwProcessId;
        if (interrupted_ && interrupted_pid_ != current_pid_) {
            //AttachConsole(current_pid_);
            //SetConsoleCtrlHandler(0, 1);
            GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
        }
        WaitForSingleObject(pi.hProcess, INFINITE);
        unsigned long exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);

        close(out_fd);
        close(err_fd);
        return interrupted_ ? 1 : exitCode;
#else
        if (interrupted_pid_ != 0) { return 1; }
        current_pid_ = fork();
        if (current_pid_ == -1) {
            fprintf(stderr, "error: could not fork %s (%s)\n", args.front().c_str(), strerror(errno));
            exit(1);
        }
        if (!current_pid_) {
            mode_t mode = S_IRUSR | S_IWUSR;
            int out_fd = open(out_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, mode);
            if (out_fd == -1) {
                fprintf(stderr, "error: could not open %s (%s)\n", out_path.c_str(), strerror(errno));
                exit(1);
            }
            int err_fd = open(err_path.c_str(), O_RDWR | O_CREAT | O_TRUNC, mode);
            if (err_fd == -1) {
                fprintf(stderr, "error: could not open %s (%s)\n", err_path.c_str(), strerror(errno));
                exit(1);
            }
            if (dup2(out_fd, STDOUT_FILENO) == -1) {
                fprintf(stderr, "error: could not duplicate stdout (%s)\n", strerror(errno));
                exit(1);
            }
            if (dup2(err_fd, STDERR_FILENO) == -1) {
                fprintf(stderr, "error: could not duplicate stderr (%s)\n", strerror(errno));
                exit(1);
            }
            std::vector<std::unique_ptr<char[]>> uargs;
            std::vector<char *> sargs;
            for (auto &arg : args) {
                uargs.emplace_back(std::make_unique<char[]>(arg.size() + 1));
                std::strcpy(uargs.back().get(), arg.c_str());
                sargs.emplace_back(uargs.back().get());
            }
            sargs.emplace_back(nullptr);
            execvp(sargs[0], sargs.data());
            fprintf(stderr, "error: executing %s failed (%s)\n", args.front().c_str(), strerror(errno));
            exit(1);
        }
        // NOTE: the child has been started and a signal handler might have been executed unnoticed
        if (interrupted_pid_ < 0) {
            kill(current_pid_, SIGTERM);
            interrupted_pid_ = current_pid_;
        }
        int status = 1;
        while (1) {
            int ret = waitpid(current_pid_, &status, 0);
            if (ret == -1) {
                if (errno == EINTR) { continue; }
                else {
                    fprintf(stderr, "error: executing %s failed (%s)\n", args.front().c_str(), strerror(errno));
                    exit(1);
                }
            }
            else { break; }
        }
        return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
#endif
    }

    static void atexit_() {
        if (!app()->debug_ && app()->ismain_()) {
            if (!app()->cudf2lp_out_.empty()) { unlink(app()->cudf2lp_out_.c_str()); }
            if (!app()->cudf2lp_err_.empty()) { unlink(app()->cudf2lp_err_.c_str()); }
            if (!app()->gringo_out_.empty())  { unlink(app()->gringo_out_.c_str()); }
            if (!app()->gringo_err_.empty())  { unlink(app()->gringo_err_.c_str()); }
            if (!app()->clasp_out_.empty())   { unlink(app()->clasp_out_.c_str()); }
            if (!app()->clasp_err_.empty())   { unlink(app()->clasp_err_.c_str()); }
        }
    }

    std::string tempfile_(char const *pattern) {
        std::vector<char> name;
        name.resize(tempdir_.size() + strlen(pattern) + 1);
        name.insert(name.end(), tempdir_.begin(), tempdir_.end());
        name.insert(name.end(), pattern, pattern + strlen(pattern) + 1);
#ifdef _WIN32
        char *p = _mktemp(name.data());
        int fd = open (p, O_RDWR | O_CREAT | O_EXCL, _S_IREAD | _S_IWRITE);
#else
        int fd = mkstemp(name.data());
#endif
        if (debug_) {
            std::cerr << "debug: created temporary file " << name.data() << std::endl;
        }
        if (fd == -1) {
            fprintf(stderr, "error: could not create %s (%s)\n", name.data(), strerror(errno));
            exit(1);
        }
        close(fd);
        return {name.begin(), name.end() - 1};
    }

    void aspcud_ecat(std::string const &name ) {
        std::cerr << std::ifstream(name).rdbuf();
    }

    void print_usage(char *name) {
        std::cout << "Usage: " << name << " [options]... [cudfin] [cudfout] [criteria]" << std::endl;
    }

    void print_version() {
        std::cout <<
            "aspcud version " CUDF_VERSION "\n"
            "\n"
            "License: The MIT License <https://opensource.org/licenses/MIT>" << std::endl;
    }

};

//////////////////// main ///////////////////////////// {{{1

int main(int argc, char *argv[]) {
    try {
        return Aspcud::app()->run(argc, argv);
    }
    catch (OptionsException const &e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        std::cerr << "INFO : " << "try '--help' for usage information" << std::endl;
        return EXIT_FAILURE;
    }
    catch (std::exception const &e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
