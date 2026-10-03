"""DSP-14: журнал изменений версии 1.0.0."""

import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tzdoc import ROOT, need, run  # noqa: E402


def section(text, pattern):
    lines = text.splitlines()
    starts = [i for i, l in enumerate(lines) if re.match(pattern, l.strip())]
    need(starts, f"нет раздела, заголовок которого подходит под {pattern!r}")
    i = starts[0]
    level = len(lines[i]) - len(lines[i].lstrip("#"))
    end = next((j for j in range(i + 1, len(lines)) if lines[j].startswith("#") and
                len(lines[j]) - len(lines[j].lstrip("#")) <= level), len(lines))
    return "\n".join(lines[i + 1:end]), i


def test_changelog():
    """CHANGELOG.md: [1.0.0] с датой, не меньше пяти добавлений, выше [0.1.0]"""
    p = ROOT / "CHANGELOG.md"
    need(p.is_file(), "нет CHANGELOG.md")
    text = p.read_text(encoding="utf-8")
    body, at = section(text, r"^##\s*\[1\.0\.0\]\s*-\s*\d{4}-\d{2}-\d{2}")
    added, _ = section(body, r"^###\s*Добавлено")
    items = [l for l in added.splitlines() if re.match(r"\s*[-*]\s", l)]
    need(len(items) >= 5, f"в «### Добавлено» версии 1.0.0 пунктов {len(items)}, нужно не меньше пяти")
    _, old = section(text, r"^##\s*\[0\.1\.0\]")
    need(at < old, "раздел [1.0.0] должен быть выше [0.1.0]: новое — сверху")


run(globals())
