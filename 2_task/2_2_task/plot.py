import matplotlib.pyplot as plt
import csv

def read_csv(filename):
    p = []
    s = []
    with open(filename) as f:
        reader = csv.reader(f)
        next(reader)  # пропускаем заголовок
        for row in reader:
            p.append(int(row[0]))
            s.append(float(row[1]))
    return p, s

p, s = read_csv("results.csv")

plt.plot(p, p, '--', label="Ideal (S = p)")
plt.plot(p, s, '-o', label="Measured")

plt.xlabel("Threads")
plt.ylabel("Speedup S(p)")
plt.title("OpenMP Scalability")
plt.grid(True)
plt.legend()

plt.savefig("speedup.pdf")
plt.savefig("speedup.png", dpi=150)
print("Done")