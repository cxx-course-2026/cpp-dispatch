"""DSP-1: черновик ТЗ в docs/TZ.md.

Проверяется форма, которую можно проверить автоматически: разделы на месте, требования
пронумерованы, измеримы и прослеживаются до письма заказчика. Содержание читает
тимлид на ревью.
"""

import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tzdoc import REF, items, letter_paragraphs, listed, need, refs, run, sections, text  # noqa: E402

SECTIONS = {
    "general": r"^##\s*1\.?\s+Общие сведения",
    "purpose": r"^##\s*2\.?\s+Назначение",
    "object": r"^##\s*3\.?\s+Характеристика объекта",
    "system": r"^##\s*4\.?\s+Требования к системе",
    "functions": r"^###\s*4\.1\.?\s+Требования к функциям",
    "quality": r"^###\s*4\.2\.?\s+Требования к надёжности",
    "data": r"^###\s*4\.3\.?\s+Требования к входным",
    "questions": r"^##\s*\d+\.?\s+Открытые вопросы",
}
MUST_COVER = [f"А{n}" for n in range(2, 11)]  # абзацы первого письма с требованиями


def requirements():
    s = sections(SECTIONS)
    return items(s["functions"], "ФТ") + items(s["quality"], "НФ")


def test_no_todo_left():
    """в ТЗ не осталось TODO"""
    need("TODO" not in text(), "в docs/TZ.md остались TODO — замените их своим текстом")


def test_sections_are_filled():
    """разделы 1–3 и 4.3 заполнены"""
    s = sections(SECTIONS)
    for key in ("general", "purpose", "object", "data"):
        need(s[key], f"раздел {SECTIONS[key]!r} пустой")


def test_functional_requirements():
    """не меньше восьми функциональных требований"""
    fts = items(sections(SECTIONS)["functions"], "ФТ")
    need(len(fts) >= 8, f"функциональных требований {len(fts)}, нужно не меньше восьми: «- ФТ-01. …»")
    ids = [n for n, _ in fts]
    need(len(ids) == len(set(ids)), "номера ФТ повторяются")


def test_quality_requirements_are_measurable():
    """не меньше четырёх измеримых нефункциональных требований"""
    nfs = items(sections(SECTIONS)["quality"], "НФ")
    need(len(nfs) >= 4, f"нефункциональных требований {len(nfs)}, нужно не меньше четырёх: «- НФ-01. …»")
    ids = [n for n, _ in nfs]
    need(len(ids) == len(set(ids)), "номера НФ повторяются")
    for n, line in nfs:
        body = REF.sub("", re.sub(r"^[-*]\s*НФ-\d{2}\.?", "", line))
        need(re.search(r"\d", body), f"НФ-{n} не измеримо — в нём нет ни одного числа: {line!r}")


def test_every_requirement_traces_to_letter():
    """каждое требование ссылается на абзац письма"""
    known = letter_paragraphs()
    for _, line in requirements():
        r = refs(line)
        need(r, f"у требования нет ссылки на письмо вида [А3]: {line!r}")
        need(r <= known, f"ссылка на абзац, которого нет в письмах, {sorted(r - known)}: {line!r}")


def test_letter_is_covered():
    """абзацы А2–А10 покрыты требованиями"""
    cited = set().union(*(refs(line) for _, line in requirements()))
    missed = [p for p in MUST_COVER if p not in cited]
    need(not missed, f"абзацы письма 1 с требованиями, на которые не ссылается ни одно требование: {missed}")


def test_data_section_has_formats():
    """в 4.3 описаны входные и выходные данные"""
    need(len(listed(sections(SECTIONS)["data"])) >= 3,
         "в 4.3 нужны хотя бы три пункта списка: что на входе, что на выходе, в каком формате")


def test_open_questions():
    """не меньше пяти вопросов заказчику со ссылками"""
    real = [q for q in listed(sections(SECTIONS)["questions"]) if "?" in REF.sub("", q)]
    need(len(real) >= 5, f"открытых вопросов {len(real)}, нужно не меньше пяти — пунктов списка со знаком «?»")
    for q in real:
        need(refs(q), f"вопрос не ссылается на абзац письма: {q!r}")


run(globals())
