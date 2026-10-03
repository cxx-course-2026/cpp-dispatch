# DSP-7: класс Registry — переходы состояний, ошибки, атомарное сохранение.
[ -f include/dispatch/registry.hpp ] || fail "нет заголовка include/dispatch/registry.hpp"
mkdir -p build-check
unit_tests check_reg7
