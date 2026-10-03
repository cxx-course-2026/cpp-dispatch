# DSP-9: сроки по правилам инспекции — deadline, overdue — и команда stats.
[ -f include/dispatch/sla.hpp ] || fail "нет заголовка include/dispatch/sla.hpp"
unit_tests check_sla9
io_tests sla9 dispatch
