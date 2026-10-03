"""Генератор файла заявок «Диспетчера» для проверок: python3 checks/gen_requests.py N SEED > файл.csv

Один и тот же SEED — один и тот же файл. Формат — тикет DSP-2: UTF-8, «;», заголовок,
12 полей, описание может содержать «;». Только стандартная библиотека Python.
"""

import random
import sys
from datetime import datetime, timedelta

HEADER = "номер;создана;дом;подъезд;квартира;телефон;вид;срочность;состояние;мастер;выполнена;описание"
KINDS = ["сантехника", "электрика", "лифт", "отопление", "уборка", "прочее"]
KIND_WEIGHTS = [30, 20, 10, 20, 12, 8]
PRIORITIES = ["аварийная", "обычная", "плановая"]
PRIORITY_WEIGHTS = [10, 60, 30]
MASTERS = {
    "сантехника": ["Петров", "Сидоров", "Кузнецов"],
    "электрика": ["Иванов", "Смирнов", "Волков"],
    "лифт": ["Морозов", "Лебедев"],
    "отопление": ["Козлов", "Новиков"],
    "уборка": ["Соколов", "Попов"],
    "прочее": ["Васильев", "Фёдоров"],
}
UNIVERSAL = ["Васильев", "Фёдоров"]
STREETS = ["Лесная", "Северная", "Садовая", "Школьная", "Мира", "Полевая", "Заводская", "Речная"]
TEXTS = {
    "сантехника": ["Течёт кран на кухне", "Засор в ванной", "Течёт стояк; вода в подъезде", "Нет горячей воды"],
    "электрика": ["Не работает розетка", "Нет света в подъезде", "Искрит щиток; запах гари", "Мигает свет"],
    "лифт": ["Застрял лифт", "Лифт не открывает двери", "Скрежет в лифте"],
    "отопление": ["Холодные батареи", "Течёт батарея", "Шумит стояк отопления"],
    "уборка": ["Не убран мусор у подъезда", "Грязно в лифте", "Разбито стекло; осколки на лестнице"],
    "прочее": ["Сломан почтовый ящик", "Не закрывается дверь подъезда", "Покрасить перила"],
}
SLA = {"аварийная": timedelta(hours=1), "обычная": timedelta(hours=24), "плановая": timedelta(days=7)}


def rows(n, seed, start=datetime(2026, 9, 1, 0, 0)):
    rnd = random.Random(seed)
    t = start
    for i in range(1, n + 1):
        t += timedelta(minutes=rnd.randint(1, 20))
        kind = rnd.choices(KINDS, KIND_WEIGHTS)[0]
        prio = rnd.choices(PRIORITIES, PRIORITY_WEIGHTS)[0]
        status = rnd.choices(["новая", "назначена", "выполнена", "закрыта", "отменена"], [8, 15, 30, 44, 3])[0]
        master = "" if status == "новая" else rnd.choice(MASTERS[kind] + UNIVERSAL)
        done = ""
        if status in ("выполнена", "закрыта"):
            late = rnd.random() < 0.15
            limit = SLA[prio]
            took = limit * (1 + rnd.random()) if late else limit * rnd.uniform(0.1, 0.95)
            done = (t + took).strftime("%Y-%m-%d %H:%M")
        house = f"{rnd.choice(STREETS)} {rnd.randint(1, 40)}"
        phone = "+79" + "".join(str(rnd.randint(0, 9)) for _ in range(9))
        yield ";".join([str(i), t.strftime("%Y-%m-%d %H:%M"), house, str(rnd.randint(1, 6)),
                        str(rnd.randint(1, 200)), phone, kind, prio, status, master, done,
                        rnd.choice(TEXTS[kind])])


def main():
    n = int(sys.argv[1]) if len(sys.argv) > 1 else 100
    seed = int(sys.argv[2]) if len(sys.argv) > 2 else 1
    out = sys.stdout
    out.write(HEADER + "\n")
    for line in rows(n, seed):
        out.write(line + "\n")


if __name__ == "__main__":
    main()
