# Copy CORE converter tools next to libman.exe after linking.
# Include after core_deps.pri when CORE is enabled.

isEmpty(CORE_BUILD_DIR) {
    error("core_converter_deploy.pri: include core_deps.pri first")
}

CONVERTER_DST = $$OUT_PWD
contains(CONFIG, release): CONVERTER_DST = $$OUT_PWD/release
contains(CONFIG, debug): CONVERTER_DST = $$OUT_PWD/debug

CORE_CONVERTERS = gds_to_core xschem_to_core qucs_to_core oas_to_core core_to_gds core_to_xschem core_to_qucs

win32 {
    for(_tool, CORE_CONVERTERS) {
        _src = $$shell_path($$CORE_BUILD_DIR/$${_tool}.exe)
        _dst = $$shell_path($$CONVERTER_DST/$${_tool}.exe)
        QMAKE_POST_LINK += $$quote(if exist $$_src copy /Y $$_src $$_dst)$$escape_expand(\\n\\t)
    }
} else {
    for(_tool, CORE_CONVERTERS) {
        _src = $$shell_path($$CORE_BUILD_DIR/$$_tool)
        _dst = $$shell_path($$CONVERTER_DST/$$_tool)
        QMAKE_POST_LINK += $$quote(cp -f $$_src $$_dst 2>/dev/null || true)$$escape_expand(\\n\\t)
    }
}
