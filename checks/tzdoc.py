"""Разбор docs/TZ.md для проверок тикетов о ТЗ: разделы, пункты требований, ссылки на письма.

Письма заказчика — letters/NN-*.md, абзацы в них помечены **[А1]**, **[Б3]**…: буква — номер
письма (А — первое, Б — второе…), число — номер абзаца. Нужен только python3, без библиотек.

Проверка тикета — функции test_*() в checks/<тикет>/check_tz.py; run(globals()) запускает их,
печатает ✓/✗ по каждой и завершается с кодом 1, если хоть одна не прошла.
"""

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
TZ = ROOT / "docs" / "TZ.md"
LETTERS = ROOT / "letters"

REF = re.compile(r"\[([А-Я]\d+(?:\s*,\s*[А-Я]\d+)*)\]")


class Fail(Exception):
    pass


def need(cond, message):
    if not cond:
        raise Fail(message)


def text():
    need(TZ.is_file(), "нет файла docs/TZ.md")
    return TZ.read_text(encoding="utf-8")


def sections(spec):
    """{ключ: регулярка заголовка} -> {ключ: непустые строки раздела до следующего заголовка}."""
    lines = text().splitlines()
    heads = [i for i, line in enumerate(lines) if line.startswith("#")]
    out = {}
    for key, pattern in spec.items():
        found = [i for i, line in enumerate(lines) if re.match(pattern, line.strip())]
        need(found, f"в docs/TZ.md нет раздела, заголовок которого подходит под {pattern!r}")
        start = found[0]
        end = next((h for h in heads if h > start), len(lines))
        out[key] = [line.strip() for line in lines[start + 1:end] if line.strip()]
    return out


def items(lines, prefix):
    """Пункты вида «- ФТ-01. текст» -> [(номер, строка)]."""
    return [(m[1], line) for line in lines if (m := re.match(rf"[-*]\s*{prefix}-(\d{{2}})\b", line))]


def listed(lines):
    """Пункты маркированного или нумерованного списка."""
    return [line for line in lines if re.match(r"[-*]\s|\d+\.\s", line)]


def refs(line):
    """Ссылки на абзацы писем в строке: {"А3", "Б4"}."""
    return {r.strip() for m in REF.finditer(line) for r in m[1].split(",")}


def letter_paragraphs():
    """Все абзацы всех писем: {"А1", …, "Б8"}."""
    found = set()
    for path in sorted(LETTERS.glob("*.md")):
        found |= set(re.findall(r"\*\*\[([А-Я]\d+)\]\*\*", path.read_text(encoding="utf-8")))
    return found


def run(namespace):
    failed = 0
    for name, fn in namespace.items():
        if not (name.startswith("test_") and callable(fn)):
            continue
        title = (fn.__doc__ or name).strip()
        try:
            fn()
            print(f"✓ {title}")
        except Fail as e:
            failed += 1
            print(f"✗ {title}: {e}")
    sys.exit(1 if failed else 0)
