# Shared Cap'n Proto + lstream schema build (include from libman.pro and tests/tests.pro).
# Set LIBMAN_ROOT to the LibMan repository root before including.

isEmpty(LIBMAN_ROOT) {
    error("capnp_deps.pri: LIBMAN_ROOT must be set to the repository root")
}

CAPNP_ROOT = $$LIBMAN_ROOT/capnp-install
CAPNP_REPO_DIR = $$LIBMAN_ROOT/capnproto
CAPNP_GEN_DIR = $$LIBMAN_ROOT/capnp

INCLUDEPATH += $$CAPNP_GEN_DIR
INCLUDEPATH += $$CAPNP_ROOT/include
win32 {
    # Fallback when install layout differs; headers always live under capnproto/c++/src.
    INCLUDEPATH += $$CAPNP_REPO_DIR/c++/src
}
LIBS += -L$$CAPNP_ROOT/lib
LIBS += -lcapnp -lkj

CAPNP_STAMP = $$CAPNP_ROOT/capnp-built.stamp
# From build/: real file target (qmake cannot emit rules for ../capnp-install/.built).
CAPNP_STAMP_REL = ../capnp-install/capnp-built.stamp
CAPNP_STAMP_REL_ESC = ../capnp-install/capnp-built\\.stamp
LSTREAM_STAMP = $$CAPNP_GEN_DIR/.schemas_built

CAPNP_GIT_URL = https://github.com/capnproto/capnproto.git
CAPNP_VERSION_MODE = branch
CAPNP_GIT_BRANCH = master
CAPNP_GIT_TAG =
CAPNP_GIT_COMMIT =

LSTREAM_GIT_URL = https://codeberg.org/klayoutmatthias/lstream.git
LSTREAM_VERSION_MODE = branch
LSTREAM_GIT_BRANCH = main
LSTREAM_GIT_TAG =
LSTREAM_GIT_COMMIT =
LSTREAM_SCHEMA_REPO_DIR = $$LIBMAN_ROOT/.deps/lstream

win32 {
    # mingw32-make on GHA uses sh: backslashes in paths eat letters (\capnp* -> apnp*).
    # Recipe uses only forward slashes; script names avoid \c and \b after a backslash.
    # After qmake, run scripts/patch_capnp_makefile.sh from build/ so parallel make waits
    # for capnp-built.stamp (qmake .depends on OBJECTS is ignored by MinGW makefiles).
    CAPNP_BUILD_CMD = cd .. && cmd /c scripts/mkcapnp.cmd

    capnp_stamp.target = $$CAPNP_STAMP_REL
    capnp_stamp.commands = $$CAPNP_BUILD_CMD
    QMAKE_EXTRA_TARGETS += capnp_stamp

    LSTREAM_BUILD_CMD = cd .. && cmd /c scripts/mklstream.cmd

    lstreamschemas.target = $$LSTREAM_STAMP
    lstreamschemas.commands = $$LSTREAM_BUILD_CMD
    lstreamschemas.depends = $$CAPNP_STAMP_REL
    QMAKE_EXTRA_TARGETS += lstreamschemas

    PRE_TARGETDEPS += $$CAPNP_STAMP_REL
    !exists($$CAPNP_GEN_DIR/.schemas_built) {
        PRE_TARGETDEPS += $$LSTREAM_STAMP
    }
} else {
    CAPNP_BUILD_CMD = bash $$shell_path($$LIBMAN_ROOT/scripts/build_capnp_linux.sh)
    CAPNP_BUILD_CMD += \"$$CAPNP_GIT_URL\"
    CAPNP_BUILD_CMD += \"$$CAPNP_VERSION_MODE\"
    CAPNP_BUILD_CMD += \"$$CAPNP_GIT_BRANCH\"
    CAPNP_BUILD_CMD += \"$$CAPNP_GIT_TAG\"
    CAPNP_BUILD_CMD += \"$$CAPNP_GIT_COMMIT\"
    CAPNP_BUILD_CMD += \"$$CAPNP_REPO_DIR\"
    CAPNP_BUILD_CMD += \"$$CAPNP_ROOT\"

    capnp_stamp.target = $$CAPNP_STAMP_REL
    capnp_stamp.commands = $$CAPNP_BUILD_CMD
    QMAKE_EXTRA_TARGETS += capnp_stamp

    LSTREAM_BUILD_CMD = bash $$shell_path($$LIBMAN_ROOT/scripts/update_lstream_schemas_linux.sh)
    LSTREAM_BUILD_CMD += \"$$LSTREAM_GIT_URL\"
    LSTREAM_BUILD_CMD += \"$$LSTREAM_VERSION_MODE\"
    LSTREAM_BUILD_CMD += \"$$LSTREAM_GIT_BRANCH\"
    LSTREAM_BUILD_CMD += \"$$LSTREAM_GIT_TAG\"
    LSTREAM_BUILD_CMD += \"$$LSTREAM_GIT_COMMIT\"
    LSTREAM_BUILD_CMD += \"$$LSTREAM_SCHEMA_REPO_DIR\"
    LSTREAM_BUILD_CMD += \"$$CAPNP_GEN_DIR\"
    LSTREAM_BUILD_CMD += \"$$CAPNP_ROOT\"

    lstreamschemas.target = $$LSTREAM_STAMP
    lstreamschemas.commands = $$LSTREAM_BUILD_CMD
    lstreamschemas.depends = $$CAPNP_STAMP_REL
    QMAKE_EXTRA_TARGETS += lstreamschemas

    PRE_TARGETDEPS += $$CAPNP_STAMP_REL
    !exists($$CAPNP_GEN_DIR/.schemas_built) {
        PRE_TARGETDEPS += $$LSTREAM_STAMP
    }
}
