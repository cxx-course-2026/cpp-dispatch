"""DSP-14: протокол приёмки по ТЗ, отчёт и презентация."""

import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tzdoc import ROOT, need, run  # noqa: E402


def read(rel):
    p = ROOT / rel
    need(p.is_file(), f"нет файла {rel}")
    return p.read_text(encoding="utf-8")


def test_acceptance_covers_tz():
    """протокол приёмки: каждое ПИ из ТЗ — с результатом"""
    tz = read("docs/TZ.md")
    pis = sorted(set(re.findall(r"^[-*]\s*ПИ-(\d{2})", tz, re.M)))
    need(pis, "в docs/TZ.md нет приёмочных испытаний ПИ-NN")
    rows = {}
    for line in read("docs/ACCEPTANCE.md").splitlines():
        m = re.match(r"^\|\s*ПИ-(\d{2})\s*\|", line)
        if m:
            rows[m[1]] = line
    missed = [f"ПИ-{n}" for n in pis if n not in rows]
    need(not missed, f"в протоколе нет строк для испытаний из ТЗ: {missed}")
    for n, line in rows.items():
        cells = [c.strip() for c in line.strip().strip("|").split("|")]
        need(len(cells) >= 4 and cells[-1] in ("пройдено", "не пройдено"),
             f"ПИ-{n}: последний столбец — «пройдено» или «не пройдено»: {line!r}")
    if any(line.strip().strip("|").split("|")[-1].strip() == "не пройдено" for line in rows.values()):
        need(re.search(r"^##\s*Замечания", read("docs/ACCEPTANCE.md"), re.M),
             "есть непройденные испытания — нужен раздел «## Замечания»")


def test_report_sections():
    """отчёт: шесть разделов"""
    text = read("docs/REPORT.md")
    for n, title in enumerate(["Постановка задачи", "Описание классов и методов", "Описание интерфейса",
                               "Результаты тестирования", "Входные и выходные данные", "Выводы"], 1):
        m = re.search(rf"^##\s*{n}\.?\s+{title}.*$", text, re.M)
        need(m, f"в отчёте нет раздела «## {n}. {title}»")
        rest = text[m.end():]
        nxt = re.search(r"^## ", rest, re.M)
        body = rest[:nxt.start()] if nxt else rest
        need(len(body.split()) >= 15, f"раздел «{n}. {title}» почти пустой")


def test_slides():
    """презентация: не меньше восьми слайдов"""
    text = read("docs/SLIDES.md")
    slides = [s for s in re.split(r"^---\s*$", text, flags=re.M) if s.strip()]
    front = slides and re.match(r"^\s*marp\s*:", slides[0], re.M) and "#" not in slides[0]
    count = len(slides) - (1 if front else 0)
    need(count >= 8, f"слайдов {count}, нужно не меньше восьми (разделитель — строка ---)")


run(globals())
