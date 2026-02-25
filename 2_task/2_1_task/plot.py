import matplotlib.pyplot as plt
import csv

def read_csv(filename):
    p = []
    t = []
    with open(filename) as f:
        reader = csv.reader(f)
        next(reader)
        for row in reader:
            p.append(int(row[0]))
            t.append(float(row[1]))
    return p, t

p1, t1 = read_csv("results_20000.csv")
p2, t2 = read_csv("results_40000.csv")

s1 = [t1[0] / x for x in t1]
s2 = [t2[0] / x for x in t2]

plt.plot(p1, p1, '--', label="Ideal (S = p)")
plt.plot(p1, s1, '-o', label="N = 20000")
plt.plot(p2, s2, '-o', label="N = 40000")

plt.xlabel("Threads")
plt.ylabel("Speedup S(p) = T₁ / Tₚ")
plt.title("OpenMP Scalability")
plt.grid(True)
plt.legend()

plt.savefig("speedup.pdf")
plt.savefig("speedup.png", dpi=150)
print("Done")