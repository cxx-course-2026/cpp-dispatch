# DSP-6: релиз 0.1 — документы, релизная сборка без предупреждений, тег v0.1.0.
quiet "журнал изменений и руководство оператора" python3 checks/rel6/check_docs.py
quiet "сборка Release без санитайзеров и без предупреждений" \
  bash -c 'cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DSAN=OFF >/dev/null && cmake --build build-release'
git rev-parse -q --verify refs/tags/v0.1.0 >/dev/null ||
  fail "нет тега v0.1.0 — после слияния PR: git tag -a v0.1.0 -m \"Диспетчер 0.1.0\" && git push origin v0.1.0"
[ "$(git cat-file -t v0.1.0)" = tag ] || fail "тег v0.1.0 не аннотированный — нужен git tag -a"
git merge-base --is-ancestor v0.1.0 HEAD || fail "тег v0.1.0 не в истории текущей ветки"
ok "тег v0.1.0"
