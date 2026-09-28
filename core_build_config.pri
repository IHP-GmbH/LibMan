# Shared ROOM / no_room selection for qmake (libman.pro, tests/tests.pro).
# Probes GitHub access to CommonDB; uses stubs when the private repo is unreachable.

isEmpty(LIBMAN_ROOT) {
    LIBMAN_ROOT = $$dirname(_PRO_FILE_)
}

LIBMAN_ROOM_TREE = $$LIBMAN_ROOT/.deps/Room
!isEmpty(LIBMAN_ROOM_SOURCE_DIR) {
    LIBMAN_ROOM_TREE = $$LIBMAN_ROOM_SOURCE_DIR
}

contains(CONFIG, no_room) {
    # explicit disable
} else:contains(CONFIG, room) {
    CONFIG -= no_room
} else {
    LIBMAN_HAS_ROOM = 0

    exists($$LIBMAN_ROOM_TREE/src/room_paths.h) {
        LIBMAN_HAS_ROOM = 1
    } else {
        win32 {
            LIBMAN_ROOM_PROBE = $$system(cmd /c $$shell_quote($$LIBMAN_ROOT/scripts/probe_core_access.cmd) $$shell_quote($$shell_path($$LIBMAN_ROOT)))
        } else {
            LIBMAN_ROOM_PROBE = $$system(bash $$shell_quote($$LIBMAN_ROOT/scripts/probe_core_access.sh) $$shell_quote($$shell_path($$LIBMAN_ROOT)))
        }
        equals(LIBMAN_ROOM_PROBE, 0) {
            LIBMAN_HAS_ROOM = 1
        }
    }

    equals(LIBMAN_HAS_ROOM, 0) {
        CONFIG += no_room
    }
}

contains(CONFIG, no_room) {
    message("LibMan: building without ROOM (CommonDB not available). Set LIBMAN_CORE_GIT_TOKEN or clone .deps/Room for full ROOM.")
} else {
    message("LibMan: building with ROOM (CommonDB available).")
}
