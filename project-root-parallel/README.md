# Forex Arbitrage Project

> A high-performance parallel Forex arbitrage detector and trading simulator.

![C++ Build](https://img.shields.io/badge/C%2B%2B-build-green) ![OpenMP](https://img.shields.io/badge/OpenMP-enabled-blue)

## Table of Contents
1. [Features](#features)
2. [Prerequisites](#prerequisites)
3. [Building](#building)
4. [Usage](#usage)
   - [Main Program](#main-program)
   - [Benchmarking](#benchmarking)
   - [Generating Plots](#generating-plots)
5. [Makefile Targets](#makefile-targets)
6. [Contributing](#contributing)
7. [License](#license)

---

## Features
- **Single‐timestamp analysis**: Build FX graph & detect one-shot arbitrage cycle.
- **Time‐series detection**: Scan historical tick data for opportunities.
- **Trading simulation**: Execute detected cycles on a virtual portfolio.
- **Benchmark driver**: Measure wall‐clock, CPU time, speedup, efficiency, and profitability across thread counts.
- **Visualization script**: Generate publication‐quality PDF charts of performance metrics.

---

## Prerequisites

- **C++17 toolchain** with OpenMP support:
  ```bash
  g++ --version
  make --version
  ```
- **Python 3** (for plotting):
  ```bash
  sudo apt update
  sudo apt install python3-pandas python3-matplotlib
  ```
  _or_ in a virtual environment:
  ```bash
  python3 -m venv venv
  source venv/bin/activate
  pip install pandas matplotlib
  ```

---

## Building

```bash
# Build main and benchmark executables\make all```

- **`arbitrage`**: main simulation binary
- **`benchmark`**: performance & profitability driver (no `main.o` linked)

---

## Usage

### Main Program

```bash
# Sequential run (1 thread)
make run-seq

# Parallel run (default 4 threads)
make run-parallel

# Override thread count:
make run-parallel PARALLEL_THREADS=8
```

This executes:
1. **Single‐timestamp analysis**
2. **Time‐series detection**
3. **Trading simulation** (prints portfolio & trade history)

### Benchmarking

```bash
# Execute benchmark across thread counts
make bench-csv
```

Generates `benchmark_results.csv` with columns:
```
threads, mean_wall_ms, stddev_wall_ms,
mean_cpu_ms,
speedup, efficiency, f_e,
mean_profit_usd, profit_usd_per_s, trade_count
```

### Generating Plots

```bash
# From CSV -> PDF charts
make plots
```

Produces:
- `wall_clock_time.pdf`  (bar chart with error bars)
- `speedup.pdf`          (speedup curve)
- `efficiency.pdf`       (efficiency curve)
- `profit_per_second.pdf` (profit-per-second curve)

---

## Makefile Targets

| Target          | Description                                    |
|-----------------|------------------------------------------------|
| `all`           | Build `arbitrage`                              |
| `run-seq`       | Run simulation (1 thread)                      |
| `run-parallel`  | Run simulation (N threads)                     |
| `benchmark`     | Build benchmarking driver                      |
| `bench-csv`     | Run `benchmark` → regenerate CSV               |
| `plots`         | Generate PDF plots from CSV (`make bench-csv`) |
| `clean`         | Remove binaries & object files                 |

---



