#!/usr/bin/env bash
# Проверки проекта «Диспетчер» курса «C++ и Qt».
#
#   bash checks/run.sh           — проверить все тикеты
#   bash checks/run.sh tz1       — проверить один тикет
#   bash checks/run.sh --score   — только итог: «заработано максимум»
#   bash checks/run.sh --report  — для GitHub Actions: подробный вывод, сводка и баллы
#
# То же самое делают `make check` и `make check-tz1`.
#
# Проверка каждого задания — небольшой скрипт checks/<тикет>/check.sh: его можно
# и нужно читать, он вызывает обычные инструменты языка (тесты, vet, линтер).
# Не изменяйте этот файл и каталог checks/: работа с изменёнными тестами
# не засчитывается.
#
# Файл собран tools/sync.py платформы курсов из общей части и части для языка курса.

set -u
cd "$(dirname "$0")/.." || exit 2
export CHECK_SRC=checks
export IN_CI=${GITHUB_ACTIONS:-}

if [ -t 1 ]; then
  RED=$(printf '\033[31m'); GREEN=$(printf '\033[32m')
  BOLD=$(printf '\033[1m'); DIM=$(printf '\033[2m'); OFF=$(printf '\033[0m')
else
  RED=; GREEN=; BOLD=; DIM=; OFF=
fi

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
export TMP

# ---- Функции для checks/<тикет>/check.sh ---------------------------------------
# Скрипт проверки пишет пояснения в поток 3 (say/fail); вывод команд
# попадает в отчёт только при неудаче.

say() { printf '%s\n' "$*" >&3; }                      # пояснение в отчёт
fail() { say "  ✗ $*"; exit 1; }                        # проверка не пройдена
note() { say "  · $*"; }                                # заметка без вердикта
ok() { say "  ✓ $*"; }

quiet() { # название команда… -> выполнить; при неудаче показать команду, хвост вывода и провалить проверку
  local title=$1 log; shift
  log=$(mktemp "$TMP/cmd.XXXXXX")
  if "$@" >"$log" 2>&1; then ok "$title"; return 0; fi
  say "  ✗ $title"
  say "    \$ $*"
  tail -n 40 "$log" | sed 's/^/    /' >&3
  exit 1
}
answer() { # файл ключ -> значение строки «ключ: значение»
  tr -d '\r' <"$1" | sed -n "s/^$2:[[:space:]]*//p" | head -n1 | sed 's/[[:space:]`]*$//; s/^`//'
}
sha256() { # stdin -> sha256 (shasum в macOS, sha256sum в Linux)
  if command -v sha256sum >/dev/null 2>&1; then sha256sum | cut -d' ' -f1; else shasum -a 256 | cut -d' ' -f1; fi
}
sha() { printf '%s' "$1" | sha256; }
answer_is() { # файл ключ sha256-правильного-ответа -> ответ совпал (регистр и пробелы не важны)
  local a; a=$(answer "$1" "$2" | tr 'A-Z' 'a-z' | tr -d ' ')
  [ -n "$a" ] || { say "  ✗ в $1 не заполнен ответ «$2»"; return 1; }
  [ "$(sha "$a")" = "$3" ] || { say "  ✗ ответ «$2» неверный"; return 1; }
  say "  ✓ ответ «$2»"
}
unchanged() { # файл sha256 -> файл не изменён (для файлов, которые трогать нельзя)
  [ "$(sha256 <"$1")" = "$2" ] || fail "файл $1 изменён — верните его: git checkout -- $1"
}
export -f say fail note ok quiet answer sha256 sha answer_is unchanged

# ---- Язык курса --------------------------------------------------------------
# C++: CMake и компилятор из системы. Задание собирается в build-check/ (отдельно от вашего
# build/, чтобы не спорить с генератором IDE) один раз за прогон, дальше — только нужные цели.
command -v cmake >/dev/null 2>&1 || { echo "Не найден cmake: установите CMake 3.25 или новее — https://cmake.org/download/"; exit 2; }

TIME_LIMIT=${TIME_LIMIT:-5}
BUILD=${BUILD_DIR:-build-check}
# «Фальшивый стек» ASan (detect_stack_use_after_return) в GCC 13 включён по умолчанию и замедляет
# программы с множеством вызовов в десятки раз; задания, которым он нужен, включают его сами.
ASAN_OPTIONS=${ASAN_OPTIONS:-detect_stack_use_after_return=0}
export TIME_LIMIT BUILD ASAN_OPTIONS

limited() { perl -e 'alarm shift @ARGV; exec @ARGV or exit 127' "$TIME_LIMIT" "$@"; }  # команда с пределом времени

show() { # файл [строк] -> содержимое с отступом в отчёт
  if [ -s "$1" ]; then head -n "${2:-15}" "$1" | sed 's/^/      /' >&3; else say "      (пусто)"; fi
}

configure() { # один раз за прогон: cmake -S . -B build-check (Debug, с санитайзерами); у проекта -S checks
  [ -f "$TMP/configured" ] && return 0
  local gen=() log="$TMP/configure.log"
  command -v ninja >/dev/null 2>&1 && [ ! -f "$BUILD/CMakeCache.txt" ] && gen=(-G Ninja)
  if ! cmake -S "${CHECK_SRC:-.}" -B "$BUILD" ${gen[@]+"${gen[@]}"} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_ARGS:-} >"$log" 2>&1; then
    say "  ✗ cmake не настроил сборку:"
    grep -v '^--' "$log" | head -n 30 | sed 's/^/    /' >&3
    exit 1
  fi
  : >"$TMP/configured"
}

build() { # цель… -> собрать; при ошибке показать первые ошибки компилятора и провалить проверку
  configure
  local log="$TMP/build.log"
  if cmake --build "$BUILD" --target "$@" >"$log" 2>&1; then ok "собирается: $*"; return 0; fi
  say "  ✗ не собирается: $* — ошибки или предупреждения компилятора (-Werror):"
  { grep -E -A3 'error|ошибка' "$log" || cat "$log"; } | grep -v '^ninja: build stopped\|^\[' | head -n 40 | sed 's/^/    /' >&3
  exit 1
}

exe() { # цель -> путь к собранной программе
  local p="$BUILD/$1"
  { [ -f "$p" ] && [ -x "$p" ]; } || p=$(find "$BUILD" -type f -name "$1" -perm -u+x 2>/dev/null | head -n 1)
  printf '%s' "$p"
}

# Один тест checks/<тикет>/NN.in и любые из файлов рядом:
#   NN.out — точный вывод; NN.grep — строки, которые должны встретиться; NN.regex — шаблоны grep -E;
#   NN.code — код возврата (по умолчанию 0); NN.err — строки, которые должны быть в stderr;
#   NN.args — аргументы командной строки (одной строкой, разбиваются по пробелам).
run_case() { # программа база -> 0 пройден, 1 нет (пояснение уже в отчёте)
  local prog=$1 base=$2 name code want=0 msg="" line args=()
  name=$(basename "$base")
  [ -f "$base.args" ] && read -r -a args <"$base.args"
  limited "$prog" ${args[@]+"${args[@]}"} <"$base.in" >"$TMP/out" 2>"$TMP/err"; code=$?
  [ -f "$base.code" ] && want=$(tr -d ' \r\n' <"$base.code")
  if [ "$code" -eq 142 ]; then msg="программа не завершилась за $TIME_LIMIT с: бесконечный цикл или ждёт ввода, которого нет"
  elif grep -q -E 'Sanitizer|runtime error:' "$TMP/err"; then msg="санитайзер нашёл ошибку памяти или неопределённое поведение (stderr ниже)"
  elif [ "$code" -gt 128 ]; then msg="программа аварийно завершилась (сигнал $((code - 128)))"
  elif [ "$code" -ne "$want" ]; then msg="код возврата $code, а должен быть $want"
  fi
  rm -f "$TMP/diff"
  if [ -z "$msg" ] && [ -f "$base.out" ] && ! diff -u -L ожидалось -L получено "$base.out" "$TMP/out" >"$TMP/diff" 2>&1; then
    msg="вывод не совпадает с ожидаемым"
  fi
  if [ -z "$msg" ] && [ -f "$base.grep" ]; then
    while IFS= read -r line || [ -n "$line" ]; do
      [ -z "$line" ] && continue
      grep -q -F -e "$line" "$TMP/out" || { msg="в выводе нет «${line}»"; break; }
    done <"$base.grep"
  fi
  if [ -z "$msg" ] && [ -f "$base.regex" ]; then
    while IFS= read -r line || [ -n "$line" ]; do
      [ -z "$line" ] && continue
      grep -q -E -e "$line" "$TMP/out" || { msg="ни одна строка вывода не подходит под шаблон  $line"; break; }
    done <"$base.regex"
  fi
  if [ -z "$msg" ] && [ -f "$base.err" ]; then
    while IFS= read -r line || [ -n "$line" ]; do
      [ -z "$line" ] && continue
      grep -q -F -e "$line" "$TMP/err" || { msg="в stderr нет сообщения «${line}»"; break; }
    done <"$base.err"
  fi
  if [ -z "$msg" ]; then say "  ✓ тест $name"; return 0; fi
  say "  ✗ тест $name: $msg"
  [ -f "$base.args" ] && say "      аргументы: $(cat "$base.args")"
  if [ -s "$base.in" ]; then say "    ввод:"; show "$base.in" 10; fi
  if [ -s "$TMP/diff" ]; then say "    разница (- ожидалось, + получено):"; show "$TMP/diff" 30
  else say "    вывод программы:"; show "$TMP/out" 20; fi
  if [ -s "$TMP/err" ]; then say "    stderr:"; show "$TMP/err" 20; fi
  return 1
}

io_tests() { # задание [цель] -> собрать цель (по умолчанию = задание) и прогнать checks/<тикет>/*.in
  local task=$1 target=${2:-$1} inp bad=0 n=0
  build "$target"
  for inp in "checks/$task"/*.in; do
    [ -e "$inp" ] || fail "нет тестов checks/$task/*.in"
    n=$((n + 1))
    run_case "$(exe "$target")" "${inp%.in}" || bad=$((bad + 1))
  done
  [ "$bad" -eq 0 ] || fail "не пройдено тестов: $bad из $n"
}

unit_tests() { # цель [аргументы] -> собрать модульные тесты и запустить; вывод — в отчёт
  local target=$1 code; shift
  build "$target"
  limited "$(exe "$target")" "$@" >"$TMP/unit" 2>&1 </dev/null; code=$?
  if [ "$code" -eq 0 ]; then sed 's/^/  /' "$TMP/unit" >&3; return 0; fi
  sed 's/^/  /' "$TMP/unit" | head -n 60 >&3
  [ "$code" -eq 142 ] && fail "тесты не завершились за $TIME_LIMIT с"
  grep -q -E 'Sanitizer|runtime error:' "$TMP/unit" && fail "санитайзер нашёл ошибку памяти или неопределённое поведение"
  [ "$code" -gt 128 ] && fail "тесты аварийно завершились (сигнал $((code - 128)))"
  fail "модульные тесты не пройдены"
}

qt_tests() { # цель -> то же для тестов на Qt: окна рисуются без экрана
  QT_QPA_PLATFORM=offscreen unit_tests "$@"
}

utf8() { # файл… -> сохранены в UTF-8 (иначе русские буквы в выводе станут «кракозябрами»)
  local f
  for f in "$@"; do
    [ -f "$f" ] || fail "нет файла $f"
    perl -e 'local $/; my $s = <STDIN>; exit(utf8::decode($s) ? 0 : 1)' <"$f" ||
      fail "$f сохранён не в UTF-8 (скорее всего, в Windows-1251) — пересохраните в UTF-8, см. памятку курса"
  done
}

forbid() { # файл слово… -> в коде (без комментариев) нет запрещённых условием слов
  local f=$1 w; shift
  [ -f "$f" ] || fail "нет файла $f"
  for w in "$@"; do
    if sed 's://.*$::' "$f" | grep -q -w -F -e "$w"; then fail "в $f не должно быть «${w}» — перечитайте условие"; fi
  done
  ok "в $f нет: $*"
}

require() { # файл строка… -> в коде есть то, что условие требует использовать
  local f=$1 w; shift
  [ -f "$f" ] || fail "нет файла $f"
  for w in "$@"; do
    sed 's://.*$::' "$f" | grep -q -F -e "$w" || fail "в $f не найдено «${w}» — по условию это нужно использовать"
  done
  ok "в $f есть: $*"
}
find_tool() { # имя -> путь к clang-tidy/clang-format: из PATH или из Homebrew LLVM на macOS
  local t
  t=$(command -v "$1" 2>/dev/null) && { printf '%s' "$t"; return 0; }
  for t in /opt/homebrew/opt/llvm/bin/"$1" /opt/homebrew/opt/llvm@*/bin/"$1" /usr/local/opt/llvm/bin/"$1"; do
    [ -x "$t" ] && { printf '%s' "$t"; return 0; }
  done
  return 1
}

tidy() { # файл… -> clang-tidy по .clang-tidy задания и compile_commands.json сборки проверок
  local t extra=()
  t=$(find_tool clang-tidy) || fail "не найден clang-tidy — установите его (памятка курса, неделя 13)"
  configure
  # На macOS clang-tidy из Homebrew не знает, где системные заголовки, — подскажем ему SDK.
  if [ "$(uname)" = Darwin ] && command -v xcrun >/dev/null 2>&1; then
    extra=(--extra-arg=-isysroot --extra-arg="$(xcrun --show-sdk-path)")
  fi
  quiet "clang-tidy: $*" "$t" -p "$BUILD" --quiet ${extra[@]+"${extra[@]}"} "$@"
}

format_check() { # файл… -> оформлены по .clang-format (clang-format ничего бы не поменял)
  local t
  t=$(find_tool clang-format) || fail "не найден clang-format — установите его (памятка курса, неделя 13)"
  quiet "clang-format: $*" "$t" --dry-run --Werror "$@"
}
export -f limited show configure build exe run_case io_tests unit_tests qt_tests utf8 forbid require find_tool tidy format_check

# ---- Запуск проверок --------------------------------------------------------
read_tasks() {
  grep -v '^#' checks/tasks.txt | grep -v '^[[:space:]]*$'
}

run_task() { # task -> 0 пройдено, 1 нет
  local task=$1 log="$TMP/$1.log"
  if [ ! -f "checks/$task/check.sh" ]; then echo "  нет checks/$task/check.sh" >"$log"; return 1; fi
  bash "checks/$task/check.sh" >"$log" 2>&1 3>&1 </dev/null
}

MODE=${1:-}
ONLY=
case "$MODE" in --score|--report) ;; "") ;; *) ONLY=$MODE; MODE=;; esac

earned=0; max=0; rows=""
while read -r task points title; do
  [ -n "$ONLY" ] && [ "$ONLY" != "$task" ] && continue
  max=$((max + points))
  run_task "$task"; rc=$?
  [ "$MODE" != "--score" ] && echo "${BOLD}$title${OFF}"
  [ "$MODE" != "--score" ] && cat "$TMP/$task.log"
  if [ $rc -eq 0 ]; then
    earned=$((earned + points)); rows="$rows| ✅ | $title | $points из $points |
"
    [ "$MODE" != "--score" ] && echo "  ${GREEN}$points из $points баллов${OFF}"
  else
    rows="$rows| ❌ | $title | 0 из $points |
"
    [ "$MODE" != "--score" ] && echo "  ${RED}0 из $points баллов${OFF}"
  fi
done < <(read_tasks)

if [ "$MODE" = "--score" ]; then
  echo "$earned $max"
  exit 0
fi
echo
echo "${BOLD}Итого: $earned из $max баллов${OFF}"
if [ -n "$IN_CI" ]; then
  if [ -n "${GITHUB_STEP_SUMMARY:-}" ]; then
    { echo "### Автотесты: $earned из $max баллов"; echo
      echo "| | Задание | Баллы |"; echo "|---|---|---|"; printf '%s' "$rows"; } >>"$GITHUB_STEP_SUMMARY"
  fi
  if [ -n "${GITHUB_OUTPUT:-}" ]; then
    { echo "earned=$earned"; echo "max=$max"; } >>"$GITHUB_OUTPUT"
  fi
else
  echo "${DIM}То же самое GitHub Actions запустит после git push.${OFF}"
fi
[ "$earned" = "$max" ]
