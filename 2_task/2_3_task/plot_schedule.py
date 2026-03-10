import matplotlib.pyplot as plt

# Читаем данные из файла
schedule_types = []
chunks = []
times = []

with open('schedule_results.csv', 'r') as f:
    next(f)  # пропускаем заголовок
    for line in f:
        sched, chunk, time = line.strip().split(',')
        schedule_types.append(sched)
        chunks.append(int(chunk))
        times.append(float(time))

static_chunks = []
static_times = []
dynamic_chunks = []
dynamic_times = []
guided_chunks = []
guided_times = []

for i in range(len(schedule_types)):
    if schedule_types[i] == 'static':
        static_chunks.append(chunks[i])
        static_times.append(times[i])
    elif schedule_types[i] == 'dynamic':
        dynamic_chunks.append(chunks[i])
        dynamic_times.append(times[i])
    else:  # guided
        guided_chunks.append(chunks[i])
        guided_times.append(times[i])

# Сортировка по размеру чанка
static_pairs = sorted(zip(static_chunks, static_times))
dynamic_pairs = sorted(zip(dynamic_chunks, dynamic_times))
guided_pairs = sorted(zip(guided_chunks, guided_times))

static_chunks = [p[0] for p in static_pairs]
static_times = [p[1] for p in static_pairs]
dynamic_chunks = [p[0] for p in dynamic_pairs]
dynamic_times = [p[1] for p in dynamic_pairs]
guided_chunks = [p[0] for p in guided_pairs]
guided_times = [p[1] for p in guided_pairs]

plt.figure(figsize=(10, 6))

plt.plot(static_chunks, static_times, 'o-', label='static', linewidth=2)
plt.plot(dynamic_chunks, dynamic_times, 's-', label='dynamic', linewidth=2)
plt.plot(guided_chunks, guided_times, '^-', label='guided', linewidth=2)

# Отметка лучшего результата
best_time = min(static_times + dynamic_times + guided_times)
best_idx = (static_times + dynamic_times + guided_times).index(best_time)
best_sched = (schedule_types * 3)[best_idx]
best_chunk = (static_chunks + dynamic_chunks + guided_chunks)[best_idx]

plt.plot(best_chunk, best_time, 'r*', markersize=15, 
         label=f'Лучший: {best_sched}, chunk={best_chunk}, {best_time:.2f}с')

plt.xscale('log')
plt.xlabel('Размер чанка (chunk)')
plt.ylabel('Время (с)')
plt.title('Влияние параметров schedule на производительность')
plt.grid(True, alpha=0.3)
plt.legend()

# Сохраняем график
plt.savefig('schedule_comparison.png', dpi=150, bbox_inches='tight')
plt.savefig('schedule_comparison.pdf', bbox_inches='tight')
plt.close()

print("График сохранен: schedule_comparison.png и schedule_comparison.pdf")
print(f"\nЛучший результат: {best_sched}, chunk={best_chunk}, время={best_time:.2f}с")