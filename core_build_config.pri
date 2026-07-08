# Shared CORE / no_core selection for qmake (libman.pro, tests/tests.pro).
# Probes GitHub access to CommonDB; uses stubs when the private repo is unreachable.

isEmpty(LIBMAN_ROOT) {
    LIBMAN_ROOT = $$dirname(_PRO_FILE_)
}

LIBMAN_CORE_TREE = $$LIBMAN_ROOT/.deps/CommonDB
!isEmpty(LIBMAN_CORE_SOURCE_DIR) {
    LIBMAN_CORE_TREE = $$LIBMAN_CORE_SOURCE_DIR
}

contains(CONFIG, no_core) {
    # explicit disable
} else:contains(CONFIG, core) {
    CONFIG -= no_core
} else {
    LIBMAN_HAS_CORE = 0

    exists($$LIBMAN_CORE_TREE/src/core_paths.h) {
        LIBMAN_HAS_CORE = 1
    } else {
        win32 {
            LIBMAN_CORE_PROBE = $$system(cmd /c $$shell_quote($$LIBMAN_ROOT/scripts/probe_core_access.cmd) $$shell_quote($$shell_path($$LIBMAN_ROOT)))
        } else {
            LIBMAN_CORE_PROBE = $$system(bash $$shell_quote($$LIBMAN_ROOT/scripts/probe_core_access.sh) $$shell_quote($$shell_path($$LIBMAN_ROOT)))
        }
        equals(LIBMAN_CORE_PROBE, 0) {
            LIBMAN_HAS_CORE = 1
        }
    }

    equals(LIBMAN_HAS_CORE, 0) {
        CONFIG += no_core
    }
}

contains(CONFIG, no_core) {
    message("LibMan: building without CORE (CommonDB not available). Set LIBMAN_CORE_GIT_TOKEN or clone .deps/CommonDB for full CORE.")
} else {
    message("LibMan: building with CORE (CommonDB available).")
}
