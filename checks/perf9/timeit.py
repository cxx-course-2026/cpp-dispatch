"""DSP-9: время команды. python3 checks/perf9/timeit.py ПРЕДЕЛ_С программа аргументы…

Запускает команду три раза и берёт лучшее время (первый запуск греет файловый кэш).
Код 0 — команда успешна и уложилась в предел.
"""
import subprocess
import sys
import time

limit = float(sys.argv[1])
cmd = sys.argv[2:]
best = None
for _ in range(3):
    t0 = time.perf_counter()
    done = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, timeout=max(10, 4 * limit))
    elapsed = time.perf_counter() - t0
    if done.returncode != 0:
        print(f"команда завершилась с кодом {done.returncode}: {done.stderr.decode(errors='replace')[:500]}")
        sys.exit(1)
    best = elapsed if best is None else min(best, elapsed)
print(f"лучшее время: {best:.2f} с, предел {limit:.1f} с")
sys.exit(0 if best <= limit else 1)
