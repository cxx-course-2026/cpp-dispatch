#!/usr/bin/env bash
# Забрать новые тикеты, письма и проверки из шаблона проекта: make update.
#
# Шаблон cpp-dispatch публичный: в нём нет решений, только тикеты, письма заказчика и проверки.
# Обновляются tickets/, letters/, checks/, cmake/flags.cmake, README.md, Makefile и workflow проверок.
# Ваш код (src/, include/, tests/), документы (docs/) и CMakeLists.txt не трогаются.
set -euo pipefail
cd "$(dirname "$0")/.."
URL=${TEMPLATE_URL:-https://codeload.github.com/cxx-course-2026/cpp-dispatch/tar.gz/refs/heads/main}

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
curl -fsSL "$URL" | tar xz -C "$tmp" --strip-components=1

for d in tickets letters checks; do
  rm -rf "$d"
  cp -R "$tmp/$d" "$d"
done
mkdir -p cmake .github/workflows
cp "$tmp/cmake/flags.cmake" cmake/flags.cmake
cp "$tmp/.clang-format" "$tmp/.clang-tidy" .
cp "$tmp/README.md" "$tmp/Makefile" .
cp "$tmp/.github/workflows/checks.yml" .github/workflows/checks.yml

echo "Готово: тикетов в tickets/ — $(ls tickets | wc -l | tr -d ' ')."
if git rev-parse --git-dir >/dev/null 2>&1; then
  git status --short -- tickets letters checks cmake README.md Makefile .github
  echo "Закоммитьте обновление отдельным коммитом: git add -A tickets letters checks cmake README.md Makefile .github && git commit -m 'Новые тикеты из шаблона'"
fi
