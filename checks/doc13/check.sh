# DSP-13: Doxygen по include/dispatch без предупреждений и описание программы по ГОСТ 19.402.
command -v doxygen >/dev/null 2>&1 || fail "не найден doxygen — установите его (памятка курса, неделя 13)"
mkdir -p build-check/doxygen
quiet "Doxygen: публичный интерфейс задокументирован" doxygen checks/doc13/Doxyfile
quiet "описание программы по ГОСТ 19.402" python3 checks/doc13/check_description.py
