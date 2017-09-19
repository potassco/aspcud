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

#include "options.hh"
#include <cudf/version.hh>

#ifdef _WIN32
#   define NOMINMAX
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   include <io.h>
#else
#   include <unistd.h>
#   include <sys/wait.h>
#   include <libgen.h>
#endif
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
#include <iterator>
#include <fcntl.h>
#ifdef __APPLE__
#   include <mach-o/dyld.h>
#endif
#include <sys/stat.h>

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
    int verbosity_ = 0;

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
    std::vector<std::string> clasp_args{clasp_bin_};
    std::vector<std::string> gringo_args{gringo_bin_};
    std::vector<std::string> gringo_encodings;

    Options options{"positional", 0, 79, true};

    std::vector<std::string> clasp_args_default{"--opt-strategy=5"};
    std::string clasp_default_args_str;
    for (auto &arg : clasp_args_default) {
        if (!clasp_default_args_str.empty()) {
            clasp_default_args_str.push_back(' ');
        }
        clasp_default_args_str += arg;
    }

    options.group("Aspcud Options");
    options.add(inputs, "positional", "positional arguments", "arg", 3, 0, true);
    options.add(criteria, "c,criterion", "optimization criterion");

    options.add(clasp_args, "s,solver-option", "append argument for solver", clasp_default_args_str.c_str(), "arg", 0);
    options.add(gringo_args, "g,grounder-option", "append argument for grounder", "arg", nullptr, 0);
    options.add(gringo_encodings, "e,encoding", "append encoding for grounder", encoding_.c_str(), "enc", 0);
    options.add(cudf2lp_args, "p,preprocessor-option", "append argument for preprocessor", "arg", nullptr, 0);

    options.add(clasp_bin_, "S,solver", "path to solver", "path", 1);
    options.add(gringo_bin_, "G,grounder", "path to grounder", "path", 1);
    options.add(cudf2lp_bin_, "P,preprocessor", "path to preprocessor", "path", 1);

    options.group("Basic Options", ":", 21);
    options.add(help, "h,help", "print help information");
    options.add(version, "v,version", "print version information");
    options.add(verbosity_, "V,verbosity", "set verbosity level", "n", 1);
    options.add(debug_, "d,debug", "do not delete intermediate files");

    options.parse(argc, argv);

    if (help) {
        print_usage(argv[0]);
        std::cout << options.description() <<
            "\n"
            "The optimization criterion can be passed as third argument or via option\n"
            "--criterion. To get a list of supported criteria, call:\n"
            "  " << cudf2lp_bin_ << " --help\n"
            "\n"
            "If argument cudfout is not given, the solution is printed to stdout. If\n"
            "argument cudfin is not given, input is read from stdin.\n"
            "\n"
            "TEMPDIR: " << tempdir_ << std::endl;
        return 0;
    }
    else if (version) {
        std::cout << "aspcud" << " version " << CUDF_VERSION << "\n\n";
        std::cout << "License: The MIT License <https://opensource.org/licenses/MIT>" << std::endl;
        return 0;
    }

    cudf2lp_args[0] = cudf2lp_bin_;

    clasp_args[0] = clasp_bin_;
    if (clasp_args.size() == 1) {
        clasp_args.insert(clasp_args.end(), clasp_args_default.begin(), clasp_args_default.end());
    }
    clasp_args.emplace_back("-q1,2");
    clasp_args.emplace_back("--stats=2");

    if (gringo_encodings.empty()) {
        gringo_encodings.emplace_back(encoding_);
    }
    gringo_args[0] = gringo_bin_;
    for (auto &encoding : gringo_encodings) {
        gringo_args.emplace_back("-f");
        gringo_args.emplace_back(encoding);
    }

    if (inputs.size() == 0) { inputs.emplace_back("-"); }
    if (inputs.size() == 1) { inputs.emplace_back("-"); }
    if (inputs.size() == 2) { inputs.emplace_back(std::move(criteria)); }
    else if (options.assigned("criteria")) {
        throw OptionsException("multiple values for criteria");
    }

    setmain_();
    if (atexit(&atexit_) != 0) {
        throw std::runtime_error("could not set exithandler");
    }

    cudf2lp_out_ = tempfile_("cudf2lp.outXXXXXX");
    cudf2lp_err_ = tempfile_("cudf2lp.errXXXXXX");
    gringo_out_ = tempfile_("gringo.outXXXXXX");
    gringo_err_ = tempfile_("gringo.errXXXXXX");
    clasp_out_ = tempfile_("clasp.outXXXXXX");
    clasp_err_ = tempfile_("clasp.errXXXXXX");

    cudf2lp_args.emplace_back("-f");
    cudf2lp_args.emplace_back(inputs[0]);
    cudf2lp_args.emplace_back("-c");
    cudf2lp_args.emplace_back(inputs[2]);
    int cudf2lp_status = exec_(cudf2lp_args, cudf2lp_out_, cudf2lp_err_);
    aspcud_ecat(cudf2lp_err_);
    if (cudf2lp_status != 0) {
        throw std::runtime_error("preprocessor returned with non-zero exit status");
    }

    // run gringo
    gringo_args.emplace_back("-f");
    gringo_args.emplace_back(cudf2lp_out_);
    int gringo_status = exec_(gringo_args, gringo_out_, gringo_err_);
    aspcud_ecat(gringo_err_);
    if (gringo_status != 0) {
        throw std::runtime_error("grounder returned with non-zero exit status");
    }

    // run clasp
    clasp_args.emplace_back("-f");
    clasp_args.emplace_back(gringo_out_);
    int clasp_status = exec_(clasp_args, clasp_out_, clasp_err_);
    // TODO: is it possible to do something with the exit status of clasp???
    (void)clasp_status;
    aspcud_ecat(clasp_err_);

    // find answer set
    std::string solution;
    bool solution_found = false;
    std::ifstream fclasp_out;
    fclasp_out.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        fclasp_out.open(clasp_out_);
        int next = 0;
        for (std::string line; std::getline(fclasp_out, line); ) {
            if (verbosity_ >= 2) {
                if (line.length() < 80) {
                    std::cerr << line << std::endl;
                }
                else {
                    std::copy(line.begin(), line.begin() + 75, std::ostreambuf_iterator<char>(std::cerr));
                    std::cerr << " ..." << std::endl;
                }
            }
            if (next == 1) {
                solution = std::move(line);
                solution_found = true;
                next = 0;
            }
            else if (std::strncmp("Answer:", line.c_str(), 7) == 0) {
                next = 1;
            }
        }
    }
    catch (std::ios_base::failure const &e) {
        if (!fclasp_out.eof()) { throw e; }
    }
    fclasp_out.close();

    // rewrite the solution
    std::ofstream aspcud_out_file;
    aspcud_out_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    std::ostream &faspcud_out = inputs[1] == "-" ? std::cout : aspcud_out_file;
    auto old = faspcud_out.exceptions();
    faspcud_out.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    if (inputs[1] != "-") { aspcud_out_file.open(inputs[1], std::ios_base::out | std::ios_base::trunc); }
    if (solution_found) {
        char const *it = solution.c_str();
        while ((it = std::strstr(it, "in(\""))) {
            it+= 4;
            char const *comma = strchr(it, ',');
            if (!comma) {
                throw std::runtime_error("unexpected output");
            }
            char const *paren = strchr(comma, ')');
            if (!paren) {
                throw std::runtime_error("unexpected output");
            }
            faspcud_out << "package: ";
            std::copy(it, comma-1, std::ostreambuf_iterator<char>(faspcud_out));
            faspcud_out << "\nversion: ";
            std::copy(comma+1, paren, std::ostreambuf_iterator<char>(faspcud_out));
            faspcud_out << "\ninstalled: true\n\n";
            it = paren + 1;
        }
    }
    else {
        faspcud_out << "FAIL\n";
    }
    if (&faspcud_out == &aspcud_out_file) {
        aspcud_out_file.close();
    }
    faspcud_out.exceptions(old);
    return EXIT_SUCCESS;
}

private:
    // if path starts with <module_path> replace this prefix with the
    // directory name of the aspcud executable
    // the function receives argv[0] in parameter module_path as a hint
    // which is used if no better method is available on a platform
    void expand_path_(std::string &path, std::string module_path) {
        char const *prefix = "<module_path>";
        if (strncmp(path.c_str(), prefix, strlen(prefix)) == 0) {
            path.erase(path.begin(), path.begin() + strlen(prefix));
#if defined(_WIN32)
            char module_filename[MAX_PATH+1];
            auto length = GetModuleFileName(nullptr, module_filename, MAX_PATH+1);
            if (!length || length >= MAX_PATH) {
                throw std::runtime_error("module file path too long");
            }
            char *file;
            std::vector<char> buf(MAX_PATH+1);
            length = GetFullPathName(module_filename, buf.size(), buf.data(), &file);
            if (!file) {
                throw std::runtime_error("module file is a directory");
            }
            if (!length || length >= MAX_PATH) {
                throw std::runtime_error("module file path too long");
            }
            module_path.assign(buf.data(), file-1);
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
                auto r = readlink(module_path.c_str(), linkname.data(), linkname.size());
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
            throw std::runtime_error("could not get TEMP");
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
            app()->interrupted_pid_ = app()->current_pid_;
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
        if (verbosity_ >= 1) {
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
        auto mode = _S_IREAD | _S_IWRITE;
        int out_fd = _open(out_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, mode);
        if (out_fd == -1) {
            throw std::runtime_error("could not open " + out_path + " (" + strerror(errno) + ")");
        }
        int err_fd = _open(err_path.c_str(), O_RDWR | O_CREAT | O_TRUNC, mode);
        if (err_fd == -1) {
            throw std::runtime_error("could not open " + err_path + " (" + strerror(errno) + ")");
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
        std::vector<char> buf{cmdline.c_str(), cmdline.c_str() + cmdline.length() + 1};

        if (!CreateProcess(args.front().c_str(), buf.data(), 0, 0, 1, 0, 0, 0, &si, &pi)) {
            throw std::runtime_error("could not execute " + cmdline);
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
            throw std::runtime_error("could not fork " + args.front() + " (" + strerror(errno) + ")");
        }
        if (!current_pid_) {
            auto mode = S_IRUSR | S_IWUSR;
            int out_fd = open(out_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, mode);
            if (out_fd == -1) {
                throw std::runtime_error("could not open " + out_path + " (" + strerror(errno) + ")");
            }
            int err_fd = open(err_path.c_str(), O_RDWR | O_CREAT | O_TRUNC, mode);
            if (err_fd == -1) {
                throw std::runtime_error("could not open " + err_path + " (" + strerror(errno) + ")");
            }
            if (dup2(out_fd, STDOUT_FILENO) == -1) {
                throw std::runtime_error("could not duplicate stdout (" + std::string(strerror(errno)) + ")");
            }
            if (dup2(err_fd, STDERR_FILENO) == -1) {
                throw std::runtime_error("could not duplicate stderr (" + std::string(strerror(errno)) + ")");
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
            throw std::runtime_error("executing " + args.front() + " failed (" + strerror(errno) + ")");
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
                    throw std::runtime_error("executing " + args.front() + " failed (" + strerror(errno) + ")");
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
        name.insert(name.end(), tempdir_.begin(), tempdir_.end());
        name.insert(name.end(), pattern, pattern + strlen(pattern) + 1);
#ifdef _WIN32
        if (!_mktemp(name.data())) {
            throw std::runtime_error("could not create temporary file");
        }
        int fd = _open (name.data(), O_RDWR | O_CREAT | O_EXCL, _S_IREAD | _S_IWRITE);
#else
        int fd = mkstemp(name.data());
#endif
        std::string ret = name.data();
        if (verbosity_ >= 1 || debug_) {
            std::cerr << "debug: created temporary file " << ret << std::endl;
        }
        if (fd == -1) {
            throw std::runtime_error("could not create " + ret + " (" + strerror(errno) + ")");
        }
        close(fd);
        return ret;
    }

    void aspcud_ecat(std::string const &name ) {
        if (verbosity_ >= 2) {
            std::ifstream in{name};
            std::copy(std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}, std::ostreambuf_iterator<char>{std::cerr});
        }
    }

    void print_usage(char *name) {
        std::cout << "Usage: " << name << " [option]... [cudfin] [cudfout] [criteria]" << std::endl;
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
