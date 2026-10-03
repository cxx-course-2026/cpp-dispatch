# DSP-5: библиотека dispatch_core — модульные проверки курса против вашего интерфейса.
grep -Eq 'add_library\([[:space:]]*dispatch_core' CMakeLists.txt ||
  fail "в CMakeLists.txt нет библиотеки dispatch_core: add_library(dispatch_core STATIC …)"
for h in request store text; do
  [ -f "include/dispatch/$h.hpp" ] || fail "нет заголовка include/dispatch/$h.hpp"
done
ok "библиотека и заголовки на месте"
unit_tests check_lib5
