# DSP-14: выпуск 1.0.0 — версия, журнал изменений, сборка Release с окном, тег v1.0.0.
grep -Eq 'project\(dispatch[[:space:]]+VERSION[[:space:]]+1\.0\.0' CMakeLists.txt || fail "в CMakeLists.txt версия не 1.0.0"
build dispatch
got=$("$(exe dispatch)" --version 2>&1)
[ "$got" = "dispatch 1.0.0" ] || fail "dispatch --version печатает «$got», а должна «dispatch 1.0.0»"
ok "dispatch --version: $got"
quiet "журнал изменений 1.0.0" python3 checks/fin14/check_changelog.py
quiet "сборка Release всего проекта без санитайзеров и без предупреждений" \
  bash -c 'cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DSAN=OFF >/dev/null && cmake --build build-release'
git rev-parse -q --verify refs/tags/v1.0.0 >/dev/null ||
  fail "нет тега v1.0.0 — после слияния PR: git tag -a v1.0.0 -m \"Диспетчер 1.0.0\" && git push origin v1.0.0"
[ "$(git cat-file -t v1.0.0)" = tag ] || fail "тег v1.0.0 не аннотированный — нужен git tag -a"
git merge-base --is-ancestor v1.0.0 HEAD || fail "тег v1.0.0 не в истории текущей ветки"
ok "тег v1.0.0"
