# DSP-7: команды assign, done, close, cancel — шаги по порядку над рабочей копией файла,
# в конце файл должен совпасть байт в байт с ожидаемым (BOM, CRLF, все изменения).
mkdir -p build-check
cp checks/cli7/data/start.csv build-check/cli7.csv
cp checks/val3/data/bad.csv build-check/cli7-bad.csv
io_tests cli7 dispatch
cmp -s build-check/cli7.csv checks/cli7/data/final.csv ||
  fail "после всех шагов файл не совпадает с ожидаемым checks/cli7/data/final.csv (сравните: cmp -l, или откройте оба)"
ok "файл после изменений совпадает с ожидаемым"
cmp -s build-check/cli7-bad.csv checks/val3/data/bad.csv || fail "файл с ошибочными строками изменён — его менять нельзя"
ok "файл с ошибками не тронут"
