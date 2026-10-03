# DSP-10: окно диспетчера — таблица заявок в порядке очереди, подсветка аварийных.
grep -Eq 'add_library\([[:space:]]*dispatch_gui_lib' CMakeLists.txt ||
  fail "в CMakeLists.txt нет библиотеки dispatch_gui_lib — окно должно быть в ней, см. тикет"
[ -f src/gui/main_window.hpp ] || fail "нет src/gui/main_window.hpp"
qt_tests check_gui10
