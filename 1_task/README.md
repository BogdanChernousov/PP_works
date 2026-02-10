# Task 1 (sum of sines)

Compute sum of sin(x) over one period for 10⁷ points.

## Build

```bash
# Double (default)
mkdir build && cd build
cmake .. && make
./task_1
```
```bash
# Float
mkdir build_float && cd build_float
cmake -DUSE_FLOAT=ON .. && make
./task_1
```
## Results
```bash
Double: 4.89582e-11
Float:  6.84089e-05
```