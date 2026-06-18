# Fetch and link CORE (CommonDB) for qmake builds.
# Set LIBMAN_ROOT before including. Requires capnp_deps.pri first.

isEmpty(LIBMAN_ROOT) {
    error("core_deps.pri: LIBMAN_ROOT must be set to the repository root")
}

isEmpty(CAPNP_BUILD_PHONY) {
    error("core_deps.pri: include capnp_deps.pri before this file")
}

CORE_BUILD_DIR = $$LIBMAN_ROOT/.deps/core-build
CORE_SRC_DIR = $$LIBMAN_ROOT/.deps/CommonDB
!isEmpty(LIBMAN_CORE_SOURCE_DIR) {
    CORE_SRC_DIR = $$LIBMAN_CORE_SOURCE_DIR
}

CORE_STAMP = $$shell_path($$CORE_BUILD_DIR/libman_core_built.stamp)
CORE_FETCH_PHONY = core_fetch

INCLUDEPATH += \
    $$CORE_SRC_DIR/src \
    $$CORE_SRC_DIR/utils \
    $$CORE_BUILD_DIR/generated

LIBS += -L$$CORE_BUILD_DIR -lcore_utils -lcore
# CORE archives depend on capnp; repeat for GNU static link order.
LIBS += -lcapnp -lkj

win32 {
    _fetchcore = $$replace($$shell_path($$LIBMAN_ROOT/scripts/fetch_core.cmd), \\, /)
    CORE_FETCH_CMD = cmd /c \"$$_fetchcore\"
    !isEmpty(LIBMAN_CORE_SOURCE_DIR) {
        _coresrc = $$replace($$shell_path($$LIBMAN_CORE_SOURCE_DIR), \\, /)
        CORE_FETCH_CMD = cmd /c \"set LIBMAN_CORE_SOURCE_DIR=$$_coresrc && $$_fetchcore\"
    }
} else {
    CORE_FETCH_CMD = bash $$shell_path($$LIBMAN_ROOT/scripts/fetch_core_linux.sh)
    CORE_FETCH_CMD += \"$$shell_path($$LIBMAN_ROOT)\"
    !isEmpty(LIBMAN_CORE_SOURCE_DIR) {
        CORE_FETCH_CMD = LIBMAN_CORE_SOURCE_DIR=$$shell_path($$LIBMAN_CORE_SOURCE_DIR) $$CORE_FETCH_CMD
    }
}

core_fetch.target = $$CORE_FETCH_PHONY
core_fetch.commands = $$CORE_FETCH_CMD
core_fetch.depends = $$CAPNP_BUILD_PHONY
QMAKE_EXTRA_TARGETS += core_fetch

!exists($$CORE_STAMP) {
    PRE_TARGETDEPS += $$CORE_FETCH_PHONY
}
