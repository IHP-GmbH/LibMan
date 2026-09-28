# Include at the end of libman.pro / tests/tests.pro (after all SOURCES are listed).
# Requires room_deps.pri to be included earlier.

isEmpty(ROOM_FETCH_PHONY) {
    error("room_deps_finalize.pri: include room_deps.pri before this file")
}

for(_src, SOURCES) {
    _src_key = $$_src
    contains(_src, $$LIBMAN_ROOT): _src_key = $$relative_path($$LIBMAN_ROOT, $$_src)
    _src_key = $$replace(_src_key, \\\\, /)
    _src_esc = $$replace(_src_key, \\\., \\.)
    eval($${_src_esc}.depends += $$ROOM_FETCH_PHONY)
}

for(_obj, OBJECTS) {
    _obj_key = $$replace(_obj, \\\\, /)
    _obj_esc = $$replace(_obj_key, \\\., \\.)
    eval($${_obj_esc}.depends += $$ROOM_FETCH_PHONY)
}
