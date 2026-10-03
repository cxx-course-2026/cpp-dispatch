"""DSP-2: ТЗ, версия 2 — ответы заказчика, этапы работ, приёмка и документация."""

import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tzdoc import REF, items, listed, need, refs, run, sections  # noqa: E402

SECTIONS = {
    "functions": r"^###\s*4\.1\.?\s+Требования к функциям",
    "quality": r"^###\s*4\.2\.?\s+Требования к надёжности",
    "works": r"^##\s*\d+\.?\s+Состав и содержание работ",
    "acceptance": r"^##\s*\d+\.?\s+Порядок контроля и приёмки",
    "docs": r"^##\s*\d+\.?\s+Требования к документированию",
    "questions": r"^##\s*\d+\.?\s+Открытые вопросы",
}
MUST_COVER = ["Б2", "Б3", "Б4", "Б5", "Б6", "Б7", "Б9"]  # абзацы второго письма с требованиями


def test_letter2_is_covered_by_requirements():
    """абзацы письма 2 покрыты требованиями"""
    s = sections(SECTIONS)
    cited = set().union(*(refs(line) for _, line in items(s["functions"], "ФТ") + items(s["quality"], "НФ")))
    missed = [p for p in MUST_COVER if p not in cited]
    need(not missed, f"абзацы письма 2, на которые не ссылается ни одно требование ФТ/НФ: {missed}")


def test_works_have_stages_with_dates():
    """этапы работ со сроками"""
    stages = listed(sections(SECTIONS)["works"])
    need(len(stages) >= 3, "в «Составе и содержании работ» нужно не меньше трёх этапов — пунктами списка")
    for line in stages:
        body = REF.sub("", re.sub(r"^([-*]|\d+\.)\s*", "", line))
        need(re.search(r"\d", body), f"у этапа нет срока — ни одного числа: {line!r}")
    need(any("Б8" in refs(line) for line in stages), "этапы должны опираться на сроки из письма 2 — ссылка [Б8]")


def test_every_function_has_acceptance_test():
    """каждое ФТ проверяется приёмочным испытанием"""
    s = sections(SECTIONS)
    fts = {n for n, _ in items(s["functions"], "ФТ")}
    tests = items(s["acceptance"], "ПИ")
    need(len(tests) >= 5, f"приёмочных испытаний {len(tests)}: нужны пункты «- ПИ-01. Что делаем и что ожидаем. (ФТ-01)»")
    ids = [n for n, _ in tests]
    need(len(ids) == len(set(ids)), "номера ПИ повторяются")
    covered = set()
    for n, line in tests:
        checks = set(re.findall(r"ФТ-(\d{2})", line))
        need(checks, f"ПИ-{n} не говорит, какое требование проверяет — ссылка вида (ФТ-03): {line!r}")
        unknown = checks - fts
        need(not unknown, f"ПИ-{n} ссылается на несуществующие требования: {sorted('ФТ-' + u for u in unknown)}")
        covered |= checks
    missed = sorted(fts - covered)
    need(not missed, f"требования без приёмочного испытания: {['ФТ-' + m for m in missed]}")


def test_documentation_list():
    """список документов для заказчика"""
    need(len(listed(sections(SECTIONS)["docs"])) >= 3, "в «Требованиях к документированию» — хотя бы три документа списком")


def test_answered_questions_keep_the_answer():
    """отвеченные вопросы сохранены с ответом"""
    qs = [q for q in listed(sections(SECTIONS)["questions"]) if "?" in REF.sub("", q)]
    answered = [q for q in qs if any(r.startswith("Б") for r in refs(q))]
    need(len(answered) >= 4, (
        f"вопросов с ответом из письма 2 — {len(answered)}, нужно не меньше четырёх: "
        "не удаляйте вопрос, допишите к нему «Ответ: … [Б4]»"
    ))


run(globals())
