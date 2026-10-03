# DSP-8: мастера, правила назначения, очередь — модульные проверки.
for h in masters queue; do [ -f "include/dispatch/$h.hpp" ] || fail "нет заголовка include/dispatch/$h.hpp"; done
unit_tests check_pol8
