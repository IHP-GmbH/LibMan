# Include at the end of libman.pro / tests/tests.pro (after all SOURCES are listed).
# Requires capnp_deps.pri to be included earlier.

isEmpty(CAPNP_BUILD_TARGET) {
    error("capnp_deps_finalize.pri: include capnp_deps.pri before this file")
}

# PRE_TARGETDEPS does not block parallel compilation of .cpp files.
# Dots in names like cell.capnp.cc must be escaped for qmake's depends syntax.
for(_src, SOURCES) {
    _src_key = $$_src
    contains(_src, $$LIBMAN_ROOT): _src_key = $$relative_path($$LIBMAN_ROOT, $$_src)
    _src_key = $$replace(_src_key, \\, /)
    _src_esc = $$replace(_src_key, \\., \\.)
    eval($${_src_esc}.depends += $$CAPNP_BUILD_TARGET)
}
