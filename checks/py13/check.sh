# DSP-13: импорт выгрузки старой программы на Python — результат байт в байт, ошибки строк, dispatch check.
[ -f tools/import_legacy.py ] || fail "нет tools/import_legacy.py"
mkdir -p build-check
rm -f build-check/imported.csv
python3 tools/import_legacy.py checks/py13/data/legacy.csv build-check/imported.csv >"$TMP/py.out" 2>"$TMP/py.err"
code=$?
[ "$code" = "$(cat checks/py13/data/expected.code)" ] || fail "код возврата $code, а должен быть $(cat checks/py13/data/expected.code)"
if ! diff -u -L ожидалось -L получено checks/py13/data/expected.err "$TMP/py.err" >"$TMP/diff"; then
  say "  ✗ сообщения об ошибках в stderr не те:"; show "$TMP/diff" 30; exit 1
fi
ok "ошибки строк — те, что нужно"
grep -q '^перенесено заявок: 34, пропущено строк: 6$' "$TMP/py.out" || fail "нет итога «перенесено заявок: 34, пропущено строк: 6» в stdout"
cmp -s build-check/imported.csv checks/py13/data/expected.csv ||
  fail "build-check/imported.csv не совпадает с checks/py13/data/expected.csv (сравните: diff их по строкам, cmp -l — по байтам)"
ok "результат совпадает с ожидаемым байт в байт"
build dispatch
quiet "dispatch check принимает результат" "$(exe dispatch)" check build-check/imported.csv
python3 tools/import_legacy.py checks/py13/data/utf8.csv build-check/x.csv >/dev/null 2>&1
[ $? -eq 2 ] || fail "файл не в Windows-1251 (checks/py13/data/utf8.csv) — нужен код 2"
ok "файл не в Windows-1251 — код 2"
