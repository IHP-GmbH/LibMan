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

# Stamp files on disk (scripts). Phony make targets below avoid qmake/DESTDIR path clashes.
CAPNP_STAMP = $$shell_path($$CAPNP_ROOT/capnp_install_stamp)
LSTREAM_STAMP = $$shell_path($$CAPNP_GEN_DIR/schemas_built_stamp)

# Phony targets only — never use a file path here (Qt Creator DESTDIR can collide).
CAPNP_BUILD_PHONY = capnp_install
LSTREAM_BUILD_PHONY = lstream_schemas

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
    _mkcapnp = $$replace($$shell_path($$LIBMAN_ROOT/scripts/mkcapnp.cmd), \\, /)
    _mklstream = $$replace($$shell_path($$LIBMAN_ROOT/scripts/mklstream.cmd), \\, /)
    CAPNP_BUILD_CMD = cmd /c \"$$_mkcapnp\"
    LSTREAM_BUILD_CMD = cmd /c \"$$_mklstream\"

    capnp_install.target = $$CAPNP_BUILD_PHONY
    capnp_install.commands = $$CAPNP_BUILD_CMD
    QMAKE_EXTRA_TARGETS += capnp_install

    lstream_schemas.target = $$LSTREAM_BUILD_PHONY
    lstream_schemas.commands = $$LSTREAM_BUILD_CMD
    lstream_schemas.depends = $$CAPNP_BUILD_PHONY
    QMAKE_EXTRA_TARGETS += lstream_schemas

    !exists($$CAPNP_GEN_DIR/schemas_built_stamp) {
        PRE_TARGETDEPS += $$LSTREAM_BUILD_PHONY
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

    capnp_install.target = $$CAPNP_BUILD_PHONY
    capnp_install.commands = $$CAPNP_BUILD_CMD
    QMAKE_EXTRA_TARGETS += capnp_install

    LSTREAM_BUILD_CMD = bash $$shell_path($$LIBMAN_ROOT/scripts/update_lstream_schemas_linux.sh)
    LSTREAM_BUILD_CMD += \"$$LSTREAM_GIT_URL\"
    LSTREAM_BUILD_CMD += \"$$LSTREAM_VERSION_MODE\"
    LSTREAM_BUILD_CMD += \"$$LSTREAM_GIT_BRANCH\"
    LSTREAM_BUILD_CMD += \"$$LSTREAM_GIT_TAG\"
    LSTREAM_BUILD_CMD += \"$$LSTREAM_GIT_COMMIT\"
    LSTREAM_BUILD_CMD += \"$$LSTREAM_SCHEMA_REPO_DIR\"
    LSTREAM_BUILD_CMD += \"$$CAPNP_GEN_DIR\"
    LSTREAM_BUILD_CMD += \"$$CAPNP_ROOT\"

    lstream_schemas.target = $$LSTREAM_BUILD_PHONY
    lstream_schemas.commands = $$LSTREAM_BUILD_CMD
    lstream_schemas.depends = $$CAPNP_BUILD_PHONY
    QMAKE_EXTRA_TARGETS += lstream_schemas

    !exists($$CAPNP_GEN_DIR/schemas_built_stamp) {
        PRE_TARGETDEPS += $$LSTREAM_BUILD_PHONY
    }
}
