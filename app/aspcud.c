//
// Copyright (c) 2010, Roland Kaminski <kaminski@cs.uni-potsdam.de>
//
// This file is part of aspcud.
//
// aspcud is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// aspcud is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with aspcud.  If not, see <http://www.gnu.org/licenses/>.
//

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <cudf/version.h>

char *aspcud_tmpdir      = NULL;
char *aspcud_gringo_enc  = ASPCUD_DEFAULT_ENCODING;
char *aspcud_cudf2lp_bin = ASPCUD_CUDF2LP_BIN;
char *aspcud_gringo_bin  = ASPCUD_GRINGO_BIN;
char *aspcud_clasp_bin   = ASPCUD_CLASP_BIN;

char *aspcud_clasp_args_default[] = {
    "--opt-heu=1",
    "--sat-prepro",
    "--restarts=L,128",
    "--heuristic=VSIDS",
    "--opt-hierarch=1",
    "--local-restarts",
    "--del-max=200000,250",
    "--save-progress=0",
    NULL
};

int aspcud_debug = 0;

volatile pid_t aspcud_current_pid     = 0;
volatile pid_t aspcud_interrupted_pid = 0;

void aspcud_set_tmpdir() {
    char *tmpdir = getenv("TMPDIR");
    if (!tmpdir) {
        tmpdir = P_tmpdir;
    }
    aspcud_tmpdir = malloc((sizeof(char))*(strlen(tmpdir)+2));
    if (!aspcud_tmpdir) {
        fprintf(stderr, "error: out of memory\n");
        exit(1);
    }
    sprintf(aspcud_tmpdir, "%s/", tmpdir);
}

void aspcud_interrupt(int signal) {
    if (aspcud_current_pid > 0) { 
        kill(aspcud_current_pid, signal);
        aspcud_interrupted_pid = aspcud_current_pid;
    }
    else { aspcud_interrupted_pid = -1; }
}

int aspcud_exec(char *const args[], char const *out_path, char const *err_path) {
    if (aspcud_interrupted_pid != 0) { return 1; }
    aspcud_current_pid = fork();
    if (aspcud_current_pid == -1) {
        fprintf(stderr, "error: could not fork %s (%s)\n", args[0], strerror(errno));
        exit(1);
    }
    if (!aspcud_current_pid) {
        if (aspcud_debug) {
            fprintf(stderr, "debug: starting process");
            char *const *arg;
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
        close(STDOUT_FILENO);
        if (dup2(out_fd, STDOUT_FILENO) == -1) {
            fprintf(stderr, "error: could not duplicate stdout (%s)\n", strerror(errno));
            exit(1);
        }
        close(STDERR_FILENO);
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
    int status = 0;
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
    return WEXITSTATUS(status); 
}

char **aspcud_args_new() {
    char **opts = malloc(sizeof(char const *));
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
    *data = realloc(*data, sizeof(char const *) * (size + 2));
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
    if (!aspcud_debug) {
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
    if (!f) {
        fprintf(stderr, "error: could not open %s (%s)\n", name, strerror(errno));
        exit(1);
    }
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
        "  -s SOL : path to solver (clasp or unclasp)\n"
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
        "Copyright (C) Roland Kaminski\n"
        "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n"
        "aspcud is free software: you are free to change and redistribute it.\n"
        "There is NO WARRANTY, to the extent permitted by law.\n");
}

void aspcud_checkarg(int i, int argc, char *argv[]) {
    if (i >= argc) {
        fprintf(stderr, "error: argument expected\n");
        aspcud_print_usage(argv[0]);
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    signal(SIGTERM, &aspcud_interrupt);
    signal(SIGUSR1, &aspcud_interrupt);
    signal(SIGINT,  &aspcud_interrupt);

    aspcud_set_tmpdir();

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
        if (a == 0 && strncmp(argv[i], "-", 1) == 0) {
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
    while ((read = getline(&line, &line_length, fclasp_out)) != -1) {
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

