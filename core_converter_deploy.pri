# Copy CORE converter tools next to libman.exe after linking.
# Include after core_deps.pri when CORE is enabled.

isEmpty(CORE_BUILD_DIR) {
    error("core_converter_deploy.pri: include core_deps.pri first")
}

CONVERTER_DST = $$OUT_PWD
!isEmpty(DESTDIR): CONVERTER_DST = $$OUT_PWD/$$DESTDIR

CORE_CONVERTERS = gds_to_core xschem_to_core qucs_to_core oas_to_core core_to_gds core_to_xschem core_to_qucs

win32 {
  # mingw32-make on GitHub Actions runs recipes in bash; invoke cmd with a helper
  # script so MSYS paths (D:/.../.deps/...) are not parsed as copy /D switches.
    _deploy = $$replace($$shell_path($$LIBMAN_ROOT/scripts/deploy_core_converter_win.cmd), \\, /)
    for(_tool, CORE_CONVERTERS) {
        _src = $$replace($$shell_path($$CORE_BUILD_DIR/$${_tool}.exe), \\, /)
        _dst = $$replace($$shell_path($$CONVERTER_DST/$${_tool}.exe), \\, /)
        QMAKE_POST_LINK += $$quote(cmd //c $$_deploy $$quote($_src) $$quote($_dst))$$escape_expand(\\n\\t)
    }
} else {
    for(_tool, CORE_CONVERTERS) {
        _src = $$shell_path($$CORE_BUILD_DIR/$$_tool)
        _dst = $$shell_path($$CONVERTER_DST/$$_tool)
        QMAKE_POST_LINK += $$quote(cp -f $$_src $$_dst 2>/dev/null || true)$$escape_expand(\\n\\t)
    }
}
