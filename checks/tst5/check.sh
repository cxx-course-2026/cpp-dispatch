# DSP-5: ваши тесты — программа dispatch_tests, не меньше 8 тестов, все проходят.
ls tests/test_*.cpp >/dev/null 2>&1 || fail "нет файлов tests/test_*.cpp"
n=$(cat tests/test_*.cpp | grep -c 'TEST(')
[ "$n" -ge 8 ] || fail "тестов $n, нужно не меньше 8 — TEST(\"…\") в tests/test_*.cpp"
ok "тестов: $n"
unit_tests dispatch_tests
