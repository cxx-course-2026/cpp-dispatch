# DSP-13: весь код оформлен по .clang-format, clang-tidy молчит по библиотеке и командной строке.
[ -f .clang-format ] && [ -f .clang-tidy ] || fail "нет .clang-format или .clang-tidy в корне проекта — выполните make update"
files=$(find src include tests -type f \( -name '*.cpp' -o -name '*.hpp' \) 2>/dev/null | sort)
[ -n "$files" ] || fail "нет исходников в src, include, tests"
# shellcheck disable=SC2086
format_check $files
build dispatch
# shellcheck disable=SC2086
tidy $(find src/core src/cli -name '*.cpp' | sort)
