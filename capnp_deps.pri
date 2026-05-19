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
LIBS += -L$$CAPNP_ROOT/lib
LIBS += -lcapnp -lkj

CAPNP_STAMP = $$CAPNP_ROOT/.built
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
    CAPNP_BUILD_CMD = cmd /c \"$$shell_path($$LIBMAN_ROOT/scripts/build_capnp_windows.bat)\"
    CAPNP_BUILD_CMD += \"$$CAPNP_GIT_URL\"
    CAPNP_BUILD_CMD += \"$$CAPNP_VERSION_MODE\"
    CAPNP_BUILD_CMD += \"$$CAPNP_GIT_BRANCH\"
    CAPNP_BUILD_CMD += \"$$CAPNP_GIT_TAG\"
    CAPNP_BUILD_CMD += \"$$CAPNP_GIT_COMMIT\"
    CAPNP_BUILD_CMD += \"$$shell_path($$CAPNP_REPO_DIR)\"
    CAPNP_BUILD_CMD += \"$$shell_path($$CAPNP_ROOT)\"

    capnpbuild.target = $$CAPNP_STAMP
    capnpbuild.commands = $$CAPNP_BUILD_CMD
    QMAKE_EXTRA_TARGETS += capnpbuild

    LSTREAM_BUILD_CMD = cmd /c \"$$shell_path($$LIBMAN_ROOT/scripts/update_lstream_schemas_windows.bat)\"
    LSTREAM_BUILD_CMD += \"$$LSTREAM_GIT_URL\"
    LSTREAM_BUILD_CMD += \"$$LSTREAM_VERSION_MODE\"
    LSTREAM_BUILD_CMD += \"$$LSTREAM_GIT_BRANCH\"
    LSTREAM_BUILD_CMD += \"$$LSTREAM_GIT_TAG\"
    LSTREAM_BUILD_CMD += \"$$LSTREAM_GIT_COMMIT\"
    LSTREAM_BUILD_CMD += \"$$shell_path($$LSTREAM_SCHEMA_REPO_DIR)\"
    LSTREAM_BUILD_CMD += \"$$shell_path($$CAPNP_GEN_DIR)\"
    LSTREAM_BUILD_CMD += \"$$shell_path($$CAPNP_ROOT)\"

    lstreamschemas.target = $$LSTREAM_STAMP
    lstreamschemas.commands = $$LSTREAM_BUILD_CMD
    lstreamschemas.depends = $$CAPNP_STAMP
    QMAKE_EXTRA_TARGETS += lstreamschemas

    PRE_TARGETDEPS += $$CAPNP_STAMP
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

    capnpbuild.target = $$CAPNP_STAMP
    capnpbuild.commands = $$CAPNP_BUILD_CMD
    QMAKE_EXTRA_TARGETS += capnpbuild

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
    lstreamschemas.depends = $$CAPNP_STAMP
    QMAKE_EXTRA_TARGETS += lstreamschemas

    PRE_TARGETDEPS += $$CAPNP_STAMP
    !exists($$CAPNP_GEN_DIR/.schemas_built) {
        PRE_TARGETDEPS += $$LSTREAM_STAMP
    }
}
