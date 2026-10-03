"""DSP-6: журнал изменений и руководство оператора — форма, которую можно проверить автоматически."""

import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tzdoc import ROOT, need, run  # noqa: E402


def read(rel):
    p = ROOT / rel
    need(p.is_file(), f"нет файла {rel}")
    return p.read_text(encoding="utf-8")


def section(text, pattern):
    lines = text.splitlines()
    starts = [i for i, l in enumerate(lines) if re.match(pattern, l.strip())]
    need(starts, f"нет раздела, заголовок которого подходит под {pattern!r}")
    i = starts[0]
    level = len(lines[i]) - len(lines[i].lstrip("#"))
    end = next((j for j in range(i + 1, len(lines)) if lines[j].startswith("#") and
                len(lines[j]) - len(lines[j].lstrip("#")) <= level), len(lines))
    return "\n".join(lines[i + 1:end])


def test_changelog():
    """CHANGELOG.md: [Unreleased] и [0.1.0] с датой и списком добавленного"""
    text = read("CHANGELOG.md")
    need(re.search(r"^##\s*\[Unreleased\]", text, re.M), "нет раздела «## [Unreleased]»")
    body = section(text, r"^##\s*\[0\.1\.0\]\s*-\s*\d{4}-\d{2}-\d{2}")
    added = section(body, r"^###\s*Добавлено")
    items = [l for l in added.splitlines() if re.match(r"\s*[-*]\s", l)]
    need(len(items) >= 3, f"в «### Добавлено» версии 0.1.0 пунктов {len(items)}, нужно не меньше трёх")


def test_user_guide_sections():
    """руководство оператора: четыре раздела ГОСТ 19.505"""
    text = read("docs/USER_GUIDE.md")
    for n, title in [(1, "Назначение программы"), (2, "Условия выполнения"), (3, "Выполнение программы"),
                     (4, "Сообщения оператору")]:
        body = section(text, rf"^##\s*{n}\.?\s+{title}")
        need(body.strip(), f"раздел «{n}. {title}» пустой")


def test_user_guide_commands():
    """в разделе 3 — все команды с примерами"""
    body = section(read("docs/USER_GUIDE.md"), r"^##\s*3\.?\s+Выполнение программы")
    for cmd in ("count", "show", "report", "find", "check"):
        need(re.search(rf"dispatch\s+{cmd}\b", body), f"в разделе 3 нет примера «dispatch {cmd} …»")


def test_user_guide_messages():
    """в разделе 4 — сообщения об ошибках"""
    body = section(read("docs/USER_GUIDE.md"), r"^##\s*4\.?\s+Сообщения оператору")
    for msg in ("не открыть файл", "не файл заявок", "полей", "неизвестный вид работ", "не найдена", "уже был"):
        need(msg in body, f"в разделе 4 нет сообщения «{msg}…» и что с ним делать")


run(globals())
