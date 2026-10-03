# DSP-1: каркас CMake и dispatch --version — версия записана только в CMakeLists.txt.
[ -f CMakeLists.txt ] || fail "нет CMakeLists.txt в корне проекта — его пишете вы, см. тикет"
version=$(sed -n -E 's/.*project\(dispatch[[:space:]]+VERSION[[:space:]]+([0-9]+\.[0-9]+\.[0-9]+).*/\1/p' CMakeLists.txt | head -n 1)
[ -n "$version" ] || fail "в CMakeLists.txt нет project(dispatch VERSION X.Y.Z …)"
grep -q 'cmake/flags.cmake' CMakeLists.txt || fail "CMakeLists.txt не подключает cmake/flags.cmake"
if grep -rn -F "$version" src include 2>/dev/null | grep -v '\.in:' >&3; then
  fail "версия $version записана прямо в коде — её нужно брать из CMake (configure_file), см. тикет"
fi
ok "версия $version берётся из CMakeLists.txt"
io_tests cli1 dispatch
got=$("$(exe dispatch)" --version 2>&1)
[ "$got" = "dispatch $version" ] || fail "dispatch --version печатает «$got», а должна «dispatch $version»"
ok "dispatch --version: $got"
