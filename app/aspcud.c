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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#ifdef __WIN32__
#   define NOMINMAX
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#else
#   include <sys/wait.h>
#   include <libgen.h>
#endif
#ifdef __APPLE__
#   include <mach-o/dyld.h>
#endif
#include <sys/stat.h>
#include <cudf/version.h>

#define ASPCUD_MIN_CHUNK 64
ssize_t aspcud_getline (char **lineptr, size_t *n, FILE *stream) {
    assert (lineptr && n && stream);

    if (!*lineptr) {
        *n = ASPCUD_MIN_CHUNK;
        *lineptr = (char*)malloc(*n);
        if (!*lineptr) {
            errno = ENOMEM;
            return -1;
        }
    }

    ssize_t m;
    char *p = *lineptr;
    for (m = *n; ; --m) {
        int c = getc(stream);
        int old_errno = errno;

        if (m < 2) {
            *n = *n >= ASPCUD_MIN_CHUNK ? *n * 2 : ASPCUD_MIN_CHUNK;

            *lineptr = (char*)realloc(*lineptr, *n);
            if (!*lineptr) {
                errno = ENOMEM;
                return -1;
            }
            m = *n + *lineptr - p;
            p = *n - m + *lineptr;
        }

        if (ferror(stream)) {
            errno = old_errno;
            return -1;
        }

        if (c == EOF) {
            if (p == *lineptr) { return -1; }
            break;
        }

        *p++ = c;

        if (c == '\n') { break; }
    }

    *p = '\0';

    return p - *lineptr;
}

//////////////////// Windows ////////////////////////// {{{1

#ifdef __WIN32__

int mkstemp(char *template) {
    template = _mktemp(template);
    int fd = open (template, O_RDWR | O_CREAT | O_EXCL, _S_IREAD | _S_IWRITE);
    return fd;
}

// See http://blogs.msdn.com/b/twistylittlepassagesallalike/archive/2011/04/23/everyone-quotes-arguments-the-wrong-way.aspx
static void aspcud_reserve(int len, char **cmdline, int *offset, int *length) {
    if (!cmdline) {
        *length  = ASPCUD_MIN_CHUNK;
        *cmdline = malloc(*length);
        if (!*cmdline) {
            fprintf(stderr, "error: out of memory\n");
            exit(1);
        }
    }
    if (*offset + len >= *length) {
        *length = ASPCUD_MIN_CHUNK + *length + len;
        *cmdline = realloc(*cmdline, *length);
        if (!*cmdline) {
            fprintf(stderr, "error: out of memory\n");
            exit(1);
        }
    }
}

static void aspcud_append_char(char arg, char **cmdline, int *offset, int *length) {
    aspcud_reserve(1, cmdline, offset, length);
    *(*cmdline + (*offset)++) = arg;
    *(*cmdline + *offset) = '\0';
}

static void aspcud_append_string(char *arg, char **cmdline, int *offset, int *length) {
    int len = strlen(arg);
    aspcud_reserve(len, cmdline, offset, length);
    strcpy(*cmdline + *offset, arg);
    *offset += len;
}

static void aspcud_append_quoted(char *arg, char **cmdline, int *offset, int *length) {
    if (*arg && !strpbrk(arg, " \t\n\v\"")) { aspcud_append_string(arg, cmdline, offset, length); }
    else {
        aspcud_append_char('\"', cmdline, offset, length);
        char *it;
        for (it = arg; *it; ++it) {
            int slashes = 0;
            for (; *it == '\\'; ++it, ++slashes) { aspcud_append_char('\\', cmdline, offset, length); }
            if (!*it || *it == '\"') {
                int i;
                for (; slashes > 0; --i) { aspcud_append_char('\\', cmdline, offset, length); }
                if (*it == '\"') { aspcud_append_char('\\', cmdline, offset, length); }
                else             { break; }
            }
            aspcud_append_char(*it, cmdline, offset, length);
        }
        aspcud_append_char('\"', cmdline, offset, length);
    }
}

static char *BuildCommandLine(char **argv) {
    char *cmdline = 0;
    int   offset  = 0;
    int   length  = 0;
    char **it;
    for (it = argv; *it; ++it) {
        aspcud_append_quoted(*it, &cmdline, &offset, &length);
        if (*(it + 1) != 0) { aspcud_append_char(' ', &cmdline, &offset, &length); }
    }
    if (!cmdline) { aspcud_append_string("", &cmdline, &offset, &length); }
    return cmdline;
}

#endif

//////////////////// aspcud /////////////////////////// {{{1

char *aspcud_tmpdir      = NULL;
char *aspcud_gringo_enc  = ASPCUD_DEFAULT_ENCODING;
char *aspcud_cudf2lp_bin = ASPCUD_CUDF2LP_BIN;
char *aspcud_gringo_bin  = ASPCUD_GRINGO_BIN;
char *aspcud_clasp_bin   = ASPCUD_CLASP_BIN;

char *aspcud_expand_path(char *path, char *module_path) {
    char const *prefix = "<module_path>";
    if (strncmp(path, prefix, strlen(prefix)) == 0) {
#ifdef __WIN32__
        char buf1[MAX_PATH+1];
        size_t length = GetModuleFileName(0, buf1, MAX_PATH+1);
        if (!length || length >= MAX_PATH) {
            fprintf(stderr, "error: module file path too long\n");
            exit(1);
        }
        char *ptr;
        char *buf = malloc(sizeof(char)*(MAX_PATH+1+strlen(path+strlen(prefix))));
        if (!buf) {
            fprintf(stderr, "error: out of memory\n");
            exit(1);
        }
        length = GetFullPathName(buf1, MAX_PATH, buf, &ptr);
        if (!length || length >= MAX_PATH) {
            fprintf(stderr, "error: module file path too long\n");
            exit(1);
        }
        strcpy(buf + (ptr - buf), path + strlen(prefix));
        return buf;
#else
#   ifdef __linux__
        module_path = "/proc/self/exe";
#   endif
#   ifdef __APPLE__
        uint32_t length = 0;
        _NSGetExecutablePath(NULL, &length);
        if (!length) {
            fprintf(stderr, "error: could not get executable path\n");
            exit(1);
        }
        module_path = malloc(length + 1);
        if (!module_path) {
            fprintf(stderr, "error: out of memory\n");
            exit(1);
        }
        if (_NSGetExecutablePath(module_path, &length) < 0) {
            fprintf(stderr, "error: could not get executable path\n");
            exit(1);
        }
#   endif // freebsd, openbsd, ...
        struct stat sb;

        if (lstat(module_path, &sb) == -1) {
            fprintf(stderr, "error: could not lstat file\n");
            exit(1);
        }
        if (S_ISLNK(sb.st_mode)) {
            if (sb.st_size == 0) { sb.st_size = 1024; }
            char *linkname = malloc(sb.st_size + 1);
            if (!linkname) {
                fprintf(stderr, "error: out of memory\n");
                exit(1);
            }
            ssize_t r = readlink(module_path, linkname, sb.st_size + 1);
            if (r < 0) {
                fprintf(stderr, "error: could not read link\n");
                exit(1);
            }
            if (r > sb.st_size) {
                fprintf(stderr, "error: could not read link\n");
                exit(1);
            }
            linkname[sb.st_size] = '\0';
            if (linkname[0] != '/') {
                module_path = dirname(strdup(module_path));
                char *buf = malloc(sizeof(char)*(strlen(module_path) + strlen(linkname) + 2));
                if (!buf) {
                    fprintf(stderr, "error: out of memory\n");
                    exit(1);
                }
                strcpy(buf, module_path);
                *(buf + strlen(module_path)) = '/';
                strcpy(buf + strlen(module_path) + 1, linkname);
                module_path = dirname(buf);
            }
            else {
                module_path = dirname(linkname);
            }
        }
        else {
            module_path = dirname(strdup(module_path));
        }
        char *buf = (char *)malloc(sizeof(char)*(strlen(module_path) + strlen(path+strlen(prefix))+2));
        strcpy(buf, module_path);
        *(buf + strlen(module_path)) = '/';
        strcpy(buf + strlen(module_path) + 1, path+strlen(prefix));
        return buf;
#endif
    }
    return path;
}

char *aspcud_clasp_args_default[] = {
    "--opt-strategy=5",
    NULL
};

int aspcud_debug = 0;

void aspcud_set_tmpdir() {
#ifdef __WIN32__
    unsigned length = GetTempPath(0, 0);
    if (!length) {
        fprintf(stderr, "error: could not get TEMP\n");
        exit(1);
    }
    aspcud_tmpdir = malloc(sizeof(char)*length);
    if (!aspcud_tmpdir) {
        fprintf(stderr, "error: out of memory\n");
        exit(1);
    }
    unsigned size = GetTempPath(length, aspcud_tmpdir);
    if (!size || size >= length) {
        fprintf(stderr, "error: could not get TEMP\n");
        exit(1);
    }
#else
    char *tmpdir = getenv("TMPDIR");
    if (!tmpdir) { tmpdir = P_tmpdir; }
    aspcud_tmpdir = (char*)malloc((sizeof(char))*(strlen(tmpdir)+2));
    if (!aspcud_tmpdir) {
        fprintf(stderr, "error: out of memory\n");
        exit(1);
    }
    sprintf(aspcud_tmpdir, "%s/", tmpdir);
#endif
}

#ifdef __WIN32__

volatile int aspcud_interrupted               = 0;
volatile unsigned long aspcud_current_pid     = 0;
volatile unsigned long aspcud_interrupted_pid = 0;

void aspcud_setmain() { }

int aspcud_ismain() { return 1; }

void aspcud_interrupt(int signal) {
    if (!aspcud_interrupted) {
        //AttachConsole(aspcud_current_pid);
        //SetConsoleCtrlHandler(0, 1);
        GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
        aspcud_interrupted_pid = aspcud_current_pid;
    }
    aspcud_interrupted = 1;
}

int aspcud_exec(char **args, char *out_path, char *err_path) {
    if (aspcud_interrupted) { return 1; }
    if (aspcud_debug) {
        fprintf(stderr, "debug: starting process");
        char **arg;
        for (arg = args; *arg; ++arg) {
            fprintf(stderr, " %s", *arg);
        }
        fprintf(stderr, "\n");
    }
    // file descriptors returned by open are inheritable
    // http://msdn.microsoft.com/en-us/library/z0kc8e3z.aspx
    mode_t mode = S_IRUSR | S_IWUSR;
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

    char *cmdline = BuildCommandLine(args);
    if (!CreateProcess(args[0], cmdline, 0, 0, 1, 0, 0, 0, &si, &pi)) {
        fprintf(stderr, "error: could not execute %s\n", cmdline);
        exit(1);
    }

    aspcud_current_pid = pi.dwProcessId;
    if (aspcud_interrupted && aspcud_interrupted_pid != aspcud_current_pid) {
        //AttachConsole(aspcud_current_pid);
        //SetConsoleCtrlHandler(0, 1);
        GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    unsigned long exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    close(out_fd);
    close(err_fd);
    return aspcud_interrupted ? 1 : exitCode;
}

#else

volatile pid_t aspcud_pid             = 0;
volatile pid_t aspcud_current_pid     = 0;
volatile pid_t aspcud_interrupted_pid = 0;

void aspcud_setmain() {
    aspcud_pid = getpid();
}

int aspcud_ismain() {
    return aspcud_pid == getpid();
}

void aspcud_interrupt(int signal) {
    if (aspcud_current_pid > 0) {
        kill(aspcud_current_pid, signal);
        aspcud_interrupted_pid = aspcud_current_pid;
    }
    else { aspcud_interrupted_pid = -1; }
}

int aspcud_exec(char **args, char *out_path, char *err_path) {
    if (aspcud_interrupted_pid != 0) { return 1; }
    aspcud_current_pid = fork();
    if (aspcud_current_pid == -1) {
        fprintf(stderr, "error: could not fork %s (%s)\n", args[0], strerror(errno));
        exit(1);
    }
    if (!aspcud_current_pid) {
        if (aspcud_debug) {
            fprintf(stderr, "debug: starting process");
            char **arg;
            for (arg = args; *arg; ++arg) {
                fprintf(stderr, " %s", *arg);
            }
            fprintf(stderr, "\n");
        }
        mode_t mode = S_IRUSR | S_IWUSR;
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
        if (dup2(out_fd, STDOUT_FILENO) == -1) {
            fprintf(stderr, "error: could not duplicate stdout (%s)\n", strerror(errno));
            exit(1);
        }
        if (dup2(err_fd, STDERR_FILENO) == -1) {
            fprintf(stderr, "error: could not duplicate stderr (%s)\n", strerror(errno));
            exit(1);
        }
        execvp(args[0], args);
        fprintf(stderr, "error: executing %s failed (%s)\n", args[0], strerror(errno));
        exit(1);
    }
    // NOTE: the child has been started and a signal handler might have been executed unnoticed
    if (aspcud_interrupted_pid < 0) {
        kill(aspcud_current_pid, SIGTERM);
        aspcud_interrupted_pid = aspcud_current_pid;
    }
    int status = 1;
    while (1) {
        int ret = waitpid(aspcud_current_pid, &status, 0);
        if (ret == -1) {
            if (errno == EINTR) { continue; }
            else {
                fprintf(stderr, "error: executing %s failed (%s)\n", args[0], strerror(errno));
                exit(1);
            }
        }
        else { break; }
    }
    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}
#endif

char **aspcud_args_new() {
    char **opts = malloc(sizeof(char *));
    if (!opts) {
        fprintf(stderr, "error: out of memory\n");
        exit(1);
    }
    opts[0] = NULL;
    return opts;
}

char **aspcud_args_push(char ***data, char *val) {
    int size;
    for (size = 0; (*data)[size]; size++) { }
    *data = realloc(*data, sizeof(char *) * (size + 2));
    if (!data) {
        fprintf(stderr, "error: out of memory\n");
        exit(1);
    }
    (*data)[size + 0] = val;
    (*data)[size + 1] = NULL;
    return *data;
}

char *aspcud_cudf2lp_out = NULL;
char *aspcud_cudf2lp_err = NULL;
char *aspcud_gringo_out  = NULL;
char *aspcud_gringo_err  = NULL;
char *aspcud_clasp_out   = NULL;
char *aspcud_clasp_err   = NULL;

void aspcud_atexit() {
    if (!aspcud_debug && aspcud_ismain()) {
        if (aspcud_cudf2lp_out) { unlink(aspcud_cudf2lp_out); }
        if (aspcud_cudf2lp_err) { unlink(aspcud_cudf2lp_err); }
        if (aspcud_gringo_out)  { unlink(aspcud_gringo_out); }
        if (aspcud_gringo_err)  { unlink(aspcud_gringo_err); }
        if (aspcud_clasp_out)   { unlink(aspcud_clasp_out); }
        if (aspcud_clasp_err)   { unlink(aspcud_clasp_err); }
    }
}

void aspcud_tempfile(char **pname, char *template) {
    char *name = malloc(sizeof(char)*(strlen(aspcud_tmpdir) + strlen(template) + 1));
    if (!name) {
        fprintf(stderr, "error: out of memory\n");
        exit(1);
    }
    sprintf(name, "%s%s", aspcud_tmpdir, template);
    int fd = mkstemp(name);
    if (aspcud_debug) {
        fprintf(stderr, "debug: created temporary file %s\n", name);
    }
    if (fd == -1) {
        fprintf(stderr, "error: could not create %s (%s)\n", name, strerror(errno));
        exit(1);
    }
    *pname = name;
    close(fd);
}

void aspcud_ecat(char *name) {
    FILE *f  = fopen(name, "r");
    if (!f) { return; }
    char buf[4096];
    size_t read;
    while ((read = fread(buf, 1, 4096, f)) > 0) {
        fwrite(buf, 1, read, stderr);
    }
    fclose(f);
}

void aspcud_print_usage(char *name) {
    printf(
        "Usage: %s [OPTION]... CUDFIN CUDFOUT [CRITERIA]\n"
        "Solves a package configuration problem given in CUDF format.\n"
        "\n"
        "Options:\n"
        "  -h     : print this help\n"
        "  -v     : print version/license information\n"
        "  -c OPT : append clasp option OPT\n"
        "  -e ENC : append encoding ENC\n"
        "  -p OPT : append cudf2lp option OPT\n"
        "  -s SOL : path to solver (clasp)\n"
        "  -g GRD : path to grounder (gringo)\n"
        "  -l PRE : path to cudf preprocessor (cudf2lp)\n"
        "  -d     : print debug info/do not cleanup temporary files\n"
        "\n"
        "If no criteria is given, then the paranoid optimization criteria is chosen.\n"
        "To get a list of supported criteria, call:\n"
        "  %s --help\n"
        "\n"
        "Default search options for clasp (overwrite with -c):\n", name, aspcud_cudf2lp_bin);
    char **arg;
    for (arg = aspcud_clasp_args_default; *arg; ++arg) {
        printf("  %s\n", *arg);
    }
    printf(
        "\n"
        "Default paths (overwrite with -e, -s, -g, -l, -t):\n"
        "  encoding : %s\n"
        "  clasp    : %s\n"
        "  gringo   : %s\n"
        "  cudf2lp  : %s\n"
        "  TMPDIR   : %s\n"
        "\n"
        "aspcud is part of Potassco : http://potassco.sourceforge.net/#aspcud\n"
        "Get help/report bugs via   : http://sourceforge.net/projects/potassco/support\n", aspcud_gringo_enc, aspcud_clasp_bin, aspcud_gringo_bin, aspcud_cudf2lp_bin, aspcud_tmpdir);
}

void aspcud_print_version() {
    printf(
        "aspcud version " CUDF_VERSION "\n"
        "\n"
        "License: The MIT License <https://opensource.org/licenses/MIT>\n");
}

void aspcud_checkarg(int i, int argc, char *argv[]) {
    if (i >= argc) {
        fprintf(stderr, "error: argument expected\n");
        aspcud_print_usage(argv[0]);
        exit(1);
    }
}

//////////////////// main ///////////////////////////// {{{1

int main(int argc, char *argv[]) {
    signal(SIGTERM, &aspcud_interrupt);
#ifndef __WIN32__
    signal(SIGUSR1, &aspcud_interrupt);
#endif
    signal(SIGINT,  &aspcud_interrupt);

    aspcud_set_tmpdir();
    aspcud_gringo_enc  = aspcud_expand_path(aspcud_gringo_enc, argv[0]);
    aspcud_cudf2lp_bin = aspcud_expand_path(aspcud_cudf2lp_bin, argv[0]);
    aspcud_gringo_bin  = aspcud_expand_path(aspcud_gringo_bin, argv[0]);
    aspcud_clasp_bin   = aspcud_expand_path(aspcud_clasp_bin, argv[0]);

    char **cudf2lp_args = aspcud_args_new();
    aspcud_args_push(&cudf2lp_args, aspcud_cudf2lp_bin);

    char **clasp_args = aspcud_args_new();
    aspcud_args_push(&clasp_args, aspcud_clasp_bin);
    aspcud_args_push(&clasp_args, "-q1,2");
    aspcud_args_push(&clasp_args, "--stats=2");

    char **gringo_args = aspcud_args_new();
    aspcud_args_push(&gringo_args, aspcud_gringo_bin);
    aspcud_args_push(&gringo_args, "-Wno-atom-undefined");

    char *aspcud_in   = NULL;
    char *aspcud_out  = NULL;
    char *aspcud_crit = NULL;

    int clasp_args_add_default = 1;
    int gringo_enc_add_default = 1;
    int i;
    int a = 0;
    for (i = 1; i < argc; ++i) {
        if (a == 0 && strncmp(argv[i], "-", 1) == 0 && strcmp(argv[i], "-") != 0) {
            if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
                aspcud_print_usage(argv[0]);
                exit(0);
            }
            else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
                aspcud_print_version();
                exit(0);
            }
            else if (strcmp(argv[i], "-d") == 0) {
                aspcud_debug = 1;
            }
            else if (strcmp(argv[i], "-c") == 0) {
                aspcud_checkarg(++i, argc, argv);
                aspcud_args_push(&clasp_args, argv[i]);
                clasp_args_add_default = 0;
            }
            else if (strcmp(argv[i], "-e") == 0) {
                aspcud_checkarg(++i, argc, argv);
                aspcud_args_push(&gringo_args, argv[i]);
                gringo_enc_add_default = 0;
            }
            else if (strcmp(argv[i], "-p") == 0) {
                aspcud_checkarg(++i, argc, argv);
                aspcud_args_push(&cudf2lp_args, argv[i]);
            }
            else if (strcmp(argv[i], "-s") == 0) {
                aspcud_checkarg(++i, argc, argv);
                clasp_args[0] = argv[i];
            }
            else if (strcmp(argv[i], "-g") == 0) {
                aspcud_checkarg(++i, argc, argv);
                gringo_args[0] = argv[i];
            }
            else if (strcmp(argv[i], "-l") == 0) {
                aspcud_checkarg(++i, argc, argv);
                cudf2lp_args[0] = argv[i];
            }
            else if (strcmp(argv[i], "-t") == 0) {
                aspcud_checkarg(++i, argc, argv);
                aspcud_tmpdir = argv[i];
            }
            else {
                fprintf(stderr, "error: unknown option %s\n", argv[i]);
                aspcud_print_usage(argv[0]);
                exit(1);
            }
        }
        else {
            if (a == 0)      { aspcud_in   = argv[i]; }
            else if (a == 1) { aspcud_out  = argv[i]; }
            else if (a == 2) { aspcud_crit = argv[i]; }
            else {
                fprintf(stderr, "error: at most three arguments expected %s\n", argv[i]);
                aspcud_print_usage(argv[0]);
                exit(1);
            }
            ++a;
        }
    }
    if (a < 2) {
        fprintf(stderr, "error: at least two arguments expected\n");
        aspcud_print_usage(argv[0]);
        exit(1);
    }

    char **arg;
    for (arg = aspcud_clasp_args_default; clasp_args_add_default && *arg; ++arg) {
        aspcud_args_push(&clasp_args, *arg);
    }

    if (gringo_enc_add_default) {
        aspcud_args_push(&gringo_args, "-f");
        aspcud_args_push(&gringo_args, aspcud_gringo_enc);
    }

    aspcud_setmain();
    if (atexit(&aspcud_atexit) != 0) {
        fprintf(stderr, "error: could not set exithandler\n");
        exit(1);
    }

    aspcud_tempfile(&aspcud_cudf2lp_out, "cudf2lp.outXXXXXX");
    aspcud_tempfile(&aspcud_cudf2lp_err, "cudf2lp.errXXXXXX");
    aspcud_tempfile(&aspcud_gringo_out,  "gringo.outXXXXXX");
    aspcud_tempfile(&aspcud_gringo_err,  "gringo.errXXXXXX");
    aspcud_tempfile(&aspcud_clasp_out,   "clasp.outXXXXXX");
    aspcud_tempfile(&aspcud_clasp_err,   "clasp.errXXXXXX");

    // run cudf2lp
    aspcud_args_push(&cudf2lp_args, "-f");
    aspcud_args_push(&cudf2lp_args, aspcud_in);
    if (aspcud_crit) {
        aspcud_args_push(&cudf2lp_args, "-c");
        aspcud_args_push(&cudf2lp_args, aspcud_crit);
    }
    else {
        aspcud_args_push(&cudf2lp_args, "-c");
        aspcud_args_push(&cudf2lp_args, "paranoid");
    }
    int cudf2lp_status = aspcud_exec(cudf2lp_args, aspcud_cudf2lp_out, aspcud_cudf2lp_err);
    aspcud_ecat(aspcud_cudf2lp_err);
    if (cudf2lp_status != 0) {
        fprintf(stderr, "error: cudf2lp returned with non-zero exit status\n");
        exit(1);
    }

    // run gringo
    aspcud_args_push(&gringo_args, "-f");
    aspcud_args_push(&gringo_args, aspcud_cudf2lp_out);
    int gringo_status = aspcud_exec(gringo_args, aspcud_gringo_out, aspcud_gringo_err);
    aspcud_ecat(aspcud_gringo_err);
    if (gringo_status != 0) {
        fprintf(stderr, "error: gringo returned with non-zero exit status\n");
        exit(1);
    }

    // run clasp
    aspcud_args_push(&clasp_args, "-f");
    aspcud_args_push(&clasp_args, aspcud_gringo_out);
    int clasp_status = aspcud_exec(clasp_args, aspcud_clasp_out, aspcud_clasp_err);
    // TODO: is it possible to do something with the exit status of clasp???
    (void)clasp_status;
    aspcud_ecat(aspcud_clasp_err);

    // find answer set
    FILE   *fclasp_out  = fopen(aspcud_clasp_out, "r");
    if (!fclasp_out) {
        fprintf(stderr, "error: could not open %s (%s)\n", aspcud_clasp_out, strerror(errno));
        exit(1);
    }
    size_t  line_length;
    size_t  solution_length;
    char   *line = NULL;
    char   *solution = NULL;
    ssize_t read;
    int     next = 0;
    while ((read = aspcud_getline(&line, &line_length, fclasp_out)) != -1) {
        if (read > 0 && line[read-1] == '\n') {
            line[read-1] = '\0';
        }
        printf("%.80s\n", line);
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
        fprintf(stderr, "error: reading %s failed (%s)\n", aspcud_clasp_out, strerror(errno));
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

    return 0;
}

