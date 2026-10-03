# DSP-9: dispatch stats на 500 000 заявок — сборка Release, не дольше 3 секунд.
mkdir -p build-check
[ -s build-check/big500k.csv ] || quiet "сгенерирован файл на 500 000 заявок" \
  bash -c 'python3 checks/gen_requests.py 500000 20 > build-check/big500k.csv'
quiet "сборка Release без санитайзеров" \
  bash -c 'cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DSAN=OFF >/dev/null && cmake --build build-release --target dispatch'
prog=build-release/dispatch
[ -x "$prog" ] || prog=$(find build-release -type f -name dispatch -perm -u+x | head -n 1)
result=$(python3 checks/perf9/timeit.py 3 "$prog" stats build-check/big500k.csv); rc=$?
[ $rc -eq 0 ] || fail "dispatch stats на 500 000 заявок: $result"
ok "dispatch stats на 500 000 заявок: $result"
