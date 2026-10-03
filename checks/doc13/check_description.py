"""DSP-13: docs/DESCRIPTION.md — разделы ГОСТ 19.402 на месте и заполнены."""

import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tzdoc import ROOT, need, run  # noqa: E402

TITLES = ["Общие сведения", "Функциональное назначение", "Описание логической структуры",
          "Используемые технические средства", "Вызов и загрузка", "Входные данные", "Выходные данные"]


def body(n, title):
    p = ROOT / "docs" / "DESCRIPTION.md"
    need(p.is_file(), "нет файла docs/DESCRIPTION.md")
    lines = p.read_text(encoding="utf-8").splitlines()
    starts = [i for i, l in enumerate(lines) if re.match(rf"^##\s*{n}\.?\s+{title}", l.strip())]
    need(starts, f"нет раздела «## {n}. {title}»")
    end = next((j for j in range(starts[0] + 1, len(lines)) if lines[j].startswith("## ")), len(lines))
    return "\n".join(lines[starts[0] + 1:end])


def test_sections():
    """все семь разделов ГОСТ 19.402 заполнены"""
    for n, title in enumerate(TITLES, 1):
        need(len(body(n, title).split()) >= 10, f"раздел «{n}. {title}» почти пустой — меньше 10 слов")


def test_structure():
    """в логической структуре — модули и классы"""
    text = body(3, "Описание логической структуры")
    for word in ("dispatch_core", "dispatch_gui_lib", "Registry"):
        need(word in text, f"в разделе 3 не упомянут {word}")


run(globals())
