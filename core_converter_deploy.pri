# Copy ROOM converter tools next to libman.exe after linking.
# Include after room_deps.pri when ROOM is enabled.

isEmpty(ROOM_BUILD_DIR) {
    error("core_converter_deploy.pri: include room_deps.pri first")
}

CONVERTER_DST = $$OUT_PWD
!isEmpty(DESTDIR): CONVERTER_DST = $$OUT_PWD/$$DESTDIR

ROOM_CONVERTERS = gds_to_room xschem_to_room qucs_to_room oas_to_room room_to_gds room_to_xschem room_to_qucs

win32 {
  # mingw32-make on GitHub Actions runs recipes in bash; invoke cmd with a helper
  # script so MSYS paths (D:/.../.deps/...) are not parsed as copy /D switches.
    _deploy = $$replace($$shell_path($$LIBMAN_ROOT/scripts/deploy_core_converter_win.cmd), \\, /)
    for(_tool, ROOM_CONVERTERS) {
        _src = $$replace($$shell_path($$ROOM_BUILD_DIR/$${_tool}.exe), \\, /)
        _dst = $$replace($$shell_path($$CONVERTER_DST/$${_tool}.exe), \\, /)
        QMAKE_POST_LINK += $$quote(cmd //c $$_deploy $$quote($_src) $$quote($_dst))$$escape_expand(\\n\\t)
    }
} else {
    for(_tool, ROOM_CONVERTERS) {
        _src = $$shell_path($$ROOM_BUILD_DIR/$$_tool)
        _dst = $$shell_path($$CONVERTER_DST/$$_tool)
        QMAKE_POST_LINK += $$quote(cp -f $$_src $$_dst 2>/dev/null || true)$$escape_expand(\\n\\t)
    }
}
