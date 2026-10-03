# DSP-12: заявки по дням (per_day) и график на QPainter.
[ -f include/dispatch/stats.hpp ] || fail "нет заголовка include/dispatch/stats.hpp с per_day"
qt_tests check_day12
