import matplotlib.pyplot as plt
from pathlib import Path

# Текущая директория (где лежит скрипт)
CUR_DIR = Path(".")

def load_csv(filename):
    """Загружает CSV без pandas"""
    threads = []
    times = []
    
    with open(filename, 'r') as f:
        # Пропускаем заголовок
        header = f.readline()
        
        # Читаем строки
        for line in f:
            line = line.strip()
            if not line:
                continue
            t, time_val = line.split(',')
            threads.append(int(t))
            times.append(float(time_val))
    
    return threads, times

# Загружаем данные
print("Загрузка данных...")
threads1, times1 = load_csv("result1.csv")
threads2, times2 = load_csv("result2.csv")
print("Данные загружены успешно")

# Сортируем по потокам (на всякий случай)
pairs1 = sorted(zip(threads1, times1))
pairs2 = sorted(zip(threads2, times2))
threads1 = [p[0] for p in pairs1]
times1 = [p[1] for p in pairs1]
threads2 = [p[0] for p in pairs2]
times2 = [p[1] for p in pairs2]

# Находим время на 1 потоке для расчета ускорения
t1_v1 = times1[0]  # первый элемент - это 1 поток
t1_v2 = times2[0]

# Расчет ускорения и эффективности
speedup1 = [t1_v1 / t for t in times1]
speedup2 = [t1_v2 / t for t in times2]
efficiency1 = [s / t for s, t in zip(speedup1, threads1)]
efficiency2 = [s / t for s, t in zip(speedup2, threads2)]

# График времени (PNG и PDF)
plt.figure(figsize=(8, 5))
plt.plot(threads1, times1, marker='o', label='Вариант 1')
plt.plot(threads2, times2, marker='o', label='Вариант 2')
plt.title('Время работы от числа потоков')
plt.xlabel('Число потоков')
plt.ylabel('Время, с')
plt.grid(True)
plt.legend()
plt.savefig("time_vs_threads.png", dpi=150, bbox_inches='tight')
plt.savefig("time_vs_threads.pdf", bbox_inches='tight')
plt.close()

# График ускорения (PNG и PDF)
plt.figure(figsize=(8, 5))
plt.plot(threads1, speedup1, marker='o', label='Вариант 1')
plt.plot(threads2, speedup2, marker='o', label='Вариант 2')
plt.plot(threads1, threads1, linestyle='--', color='gray', label='Линейное ускорение')
plt.title('Ускорение от числа потоков')
plt.xlabel('Число потоков')
plt.ylabel('S(p)')
plt.grid(True)
plt.legend()
plt.savefig("speedup_vs_threads.png", dpi=150, bbox_inches='tight')
plt.savefig("speedup_vs_threads.pdf", bbox_inches='tight')
plt.close()

# График эффективности (PNG и PDF)
plt.figure(figsize=(8, 5))
plt.plot(threads1, efficiency1, marker='o', label='Вариант 1')
plt.plot(threads2, efficiency2, marker='o', label='Вариант 2')
plt.axhline(y=1.0, linestyle='--', color='gray', alpha=0.5, label='Идеал = 1.0')
plt.title('Эффективность от числа потоков')
plt.xlabel('Число потоков')
plt.ylabel('E(p)')
plt.grid(True)
plt.legend()
plt.savefig("efficiency_vs_threads.png", dpi=150, bbox_inches='tight')
plt.savefig("efficiency_vs_threads.pdf", bbox_inches='tight')
plt.close()

# Вывод таблицы результатов
print("\n" + "="*60)
print("Сводная таблица результатов")
print("="*60)

print("\nВариант 1:")
print(f"{'threads':>8} {'time_s':>10} {'speedup':>10} {'efficiency':>12}")
print("-" * 42)
for t, time, s, e in zip(threads1, times1, speedup1, efficiency1):
    print(f"{t:8d} {time:10.4f} {s:10.4f} {e:12.4f}")

print("\nВариант 2:")
print(f"{'threads':>8} {'time_s':>10} {'speedup':>10} {'efficiency':>12}")
print("-" * 42)
for t, time, s, e in zip(threads2, times2, speedup2, efficiency2):
    print(f"{t:8d} {time:10.4f} {s:10.4f} {e:12.4f}")

print(f"\nФайлы сохранены:")
print("  - time_vs_threads.png/pdf")
print("  - speedup_vs_threads.png/pdf")
print("  - efficiency_vs_threads.png/pdf")