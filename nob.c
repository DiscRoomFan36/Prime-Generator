
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "thirdparty/nob.h"

#define SRC_FOLDER          "./src/"
#define BUILD_FOLDER        "./build/"
#define THIRDPARTY_FOLDER   "./thirdparty/"




static bool build_debug(void);
static bool build_release(void);


static Cmd cmd = {0};



#define ARGUMENTS                                               \
    X(help,         "prints this help message and quits")       \
    X(all,          "build all targets. [debug, release]")      \
    X(clean,        "clean up all build artifacts, happens before all other commands, so 'all clean' will clean everything, then build everything.")        \
    X(debug,        "build   debug native version")             \
    X(release,      "build release native version")             \



void print_usage(const char *program_name, bool to_stderr) {
    FILE *out = to_stderr ? stderr : stdout;

    fprintf(out, "USAGE %s: [...ARGUMENTS]\n", program_name);
    fprintf(out, "[ARGUMENTS]:\n");
    #define X(name, description)    fprintf(out, "%11s: %s\n", #name, description);
        ARGUMENTS
    #undef X
}

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    const char *program_name = argv[0];

    // if no arguments provided, print help
    if (argc == 1) {
        print_usage(program_name, false);
        exit(EXIT_SUCCESS);
    }


    // parse arguments

    struct Flags {
        // make flags for all arguments
        #define X(name, ...) bool name;
            ARGUMENTS
        #undef X
    } flags = {};

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        bool valid_flag = false;
        bool duplicate_flag = false;

        // check if the argument is this flag.
        #define X(name, ...)                    \
            if (strcmp(arg, #name) == 0) {      \
                if (flags.name) duplicate_flag = true;     \
                flags.name = true;             \
                valid_flag  = true;             \
            }
            ARGUMENTS
        #undef X

        if (!valid_flag) {
            fprintf(stderr, "ERROR: INVALID ARGUMENT - '%s' in ", arg);

            fprintf(stderr, "[%s", program_name);
            for (int j = 1; j < argc; j++) fprintf(stderr, " %s", argv[j]);
            fprintf(stderr, "]\n");

            print_usage(program_name, true);
            exit(EXIT_FAILURE);
        }
        if (duplicate_flag) {
            fprintf(stderr, "ERROR: DUPLICATE FLAG - '%s' in ", arg);

            fprintf(stderr, "[%s", program_name);
            for (int j = 1; j < argc; j++) fprintf(stderr, " %s", argv[j]);
            fprintf(stderr, "]\n");

            print_usage(program_name, true);
            exit(EXIT_FAILURE);
        }
    }

    if (flags.help) {
        print_usage(program_name, false);
        exit(EXIT_SUCCESS);
    }


    if (flags.all) {
        flags.debug      = true;
        flags.release    = true;
    }

    if (flags.clean) {
        cmd_append(&cmd, "rm", "-fr", BUILD_FOLDER);
        if (!cmd_run(&cmd))         exit(EXIT_FAILURE);

        // its ok to ignore the result, if nob.old did not exist, no reason to delete it.
        delete_file("./nob.old");
    }



    // TODO make this a command line arg
    //
    // this gets the latest version of Bested.h, I know that I might have to
    // refactor stuff when i do this, but its worth it to me.
    #define BESTED_PATH "/home/fletcher/Programming/C-things/Bested.h/Bested.h"
    if (file_exists(BESTED_PATH) == 1) {
        bool result = copy_file(BESTED_PATH, THIRDPARTY_FOLDER"Bested.h");
        if (!result) return 1;
    }


    // dont make a build folder if your not building anything.
    bool building_anything = flags.debug || flags.release;
    if (building_anything) {
        mkdir_if_not_exists(BUILD_FOLDER);
    }

    if (flags.debug) {
        if (!build_debug())         exit(EXIT_FAILURE);
    }
    if (flags.release) {
        if (!build_release())       exit(EXIT_FAILURE);
    }


    exit(EXIT_SUCCESS);
}




void cmd_cc(void) {
    cmd_append(&cmd, "clang");
    cmd_append(&cmd, "-std=gnu11");
}

void cmd_c_flags(void) {
    cmd_append(&cmd, "-Wall", "-Wextra");
    // cmd_append(&cmd, "-Werror");
    cmd_append(&cmd, "-Wno-initializer-overrides");

    cmd_append(&cmd, "-I"THIRDPARTY_FOLDER);
}


bool build_debug(void) {
    cmd_cc();
    cmd_c_flags();
    cmd_append(&cmd, "-ggdb"); // debug flag

    cmd_append(&cmd, "-o", BUILD_FOLDER"main_debug");
    cmd_append(&cmd, SRC_FOLDER"main.c");

    if (!cmd_run(&cmd)) return false;
    return true;
}

bool build_release(void) {
    cmd_cc();
    cmd_c_flags();
    cmd_append(&cmd, "-O2");
    // cmd_append(&cmd, "-march=native");

    cmd_append(&cmd, "-o", BUILD_FOLDER"main_release");
    cmd_append(&cmd, SRC_FOLDER"main.c");

    if (!cmd_run(&cmd)) return false;
    return true;
}
