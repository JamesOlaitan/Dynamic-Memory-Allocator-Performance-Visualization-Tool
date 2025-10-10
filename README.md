# Dynamic Memory Allocator Performance Visualization Tool

![CI](https://github.com/jamesolaitan/Dynamic-Memory-Allocator-Performance-Visualization-Tool/workflows/CI/badge.svg)
![License](https://img.shields.io/badge/license-MIT-blue.svg)
![C++17](https://img.shields.io/badge/C++-17-blue.svg)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey)

A production-ready implementation of the classic **Buddy Memory Allocation Algorithm** in modern C++17, featuring comprehensive performance instrumentation, automated benchmarking, and visualization capabilities.

## ✨ Features

- **🚀 High-Performance Buddy Allocator**: Thread-safe implementation with O(log n) allocation/deallocation
- **📊 8+ Performance Metrics**: Captures throughput, latency, fragmentation, memory usage, and more
- **🔬 Comprehensive Testing**: GoogleTest unit tests, integration tests, and Google Benchmark stress tests
- **📈 Advanced Visualization**: 10+ plot types including latency percentiles (p50/p95/p99), heatmaps, and throughput trends
- **⚙️ Configurable**: TOML configuration with CLI override support (no hard-coded values)
- **🛡️ Production-Ready**: CI/CD pipeline with sanitizers, static analysis, and cross-platform builds
- **🎯 Zero Dependencies**: Header-only dependencies fetched automatically via CMake

## 📋 Table of Contents

- [Quick Start](#-quick-start)
- [Building from Source](#-building-from-source)
- [Configuration](#-configuration)
- [Running Tests](#-running-tests)
- [Benchmarking](#-benchmarking)
- [Visualization](#-visualization)
- [Architecture](#-architecture)
- [CSV Schema](#-csv-schema)
- [CI/CD Pipeline](#-cicd-pipeline)
- [Contributing](#-contributing)
- [License](#-license)

## 🚀 Quick Start

```bash
# Clone the repository
git clone https://github.com/jamesolaitan/Dynamic-Memory-Allocator-Performance-Visualization-Tool.git
cd Dynamic-Memory-Allocator-Performance-Visualization-Tool

# Build (Release mode)
cmake --preset release
cmake --build build/release

# Run unit tests
ctest --test-dir build/release --output-on-failure

# Run performance benchmark
./build/release/performance_tests --benchmark throughput --duration 10 --out reports

# Generate visualizations
pip install -r requirements.txt
python src/main/main.py --input reports/*.csv --output reports/plots --timestamp
```

## 🔨 Building from Source

### Prerequisites

- **CMake** 3.20+
- **C++17-compatible compiler** (GCC 9+, Clang 10+, MSVC 2019+)
- **Ninja** (optional, but recommended)
- **Python 3.8+** (for visualization)

### Build with CMake Presets

We provide several CMake presets for different build configurations:

```bash
# Debug build
cmake --preset debug
cmake --build build/debug

# Release build (optimized)
cmake --preset release
cmake --build build/release

# AddressSanitizer build
cmake --preset asan
cmake --build build/asan

# UndefinedBehaviorSanitizer build
cmake --preset ubsan
cmake --build build/ubsan

# Combined sanitizers
cmake --preset sanitizers
cmake --build build/sanitizers
```

### Manual Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON -DBUILD_BENCHMARKS=ON
cmake --build . -j$(nproc)
```

## ⚙️ Configuration

### TOML Configuration File

The allocator can be configured via `config/default.toml`:

```toml
[allocator]
min_order = 6          # Minimum block order (2^6 = 64 bytes)
max_order = 20         # Maximum block order (2^20 = 1MB)
alignment = 8          # Memory alignment in bytes

[testing]
num_operations = 1000  # Number of operations for tests
duration_seconds = 10.0 # Duration for time-based benchmarks
random_seed = 42       # Random seed for reproducibility
threads = 1            # Number of threads

[output]
directory = "reports"  # Output directory for CSV files
format = "csv"         # Output format (csv only for now)
```

### CLI Arguments

All configuration values can be overridden via command-line arguments:

```bash
./allocator_tests \
  --min-order 6 \
  --max-order 20 \
  --ops 10000 \
  --out reports \
  --config config/custom.toml
```

**Precedence**: CLI arguments > config file > defaults

### Available Flags

| Flag | Description | Default |
|------|-------------|---------|
| `--min-order` | Minimum buddy order (2^N bytes) | 6 |
| `--max-order` | Maximum buddy order (2^N bytes) | 20 |
| `--alignment` | Memory alignment in bytes | 8 |
| `--threads` | Number of threads | 1 |
| `--ops` | Number of operations | 1000 |
| `--duration` | Test duration in seconds | 10.0 |
| `--seed` | Random seed | 42 |
| `--out` | Output directory | reports |
| `--format` | Output format (csv\|json) | csv |
| `--config` | Path to config file | config/default.toml |

## 🧪 Running Tests

### Unit Tests (GoogleTest)

```bash
# Run all unit tests
ctest --test-dir build/release --output-on-failure

# Run specific test
./build/release/unit_tests --gtest_filter=CustomAllocatorTest.BasicAllocationDeallocation

# Verbose output
./build/release/unit_tests --gtest_verbose
```

### Integration Tests

```bash
# Sequential allocation test
./build/release/allocator_tests --test sequential --ops 5000

# Random allocation/deallocation test
./build/release/allocator_tests --test random --ops 5000

# Mixed size test
./build/release/allocator_tests --test mixed --ops 5000
```

### Performance Tests

```bash
# Fixed-size benchmark
./build/release/performance_tests --benchmark fixed --ops 100000

# Variable-size benchmark
./build/release/performance_tests --benchmark variable --ops 100000

# Throughput benchmark
./build/release/performance_tests --benchmark throughput --duration 30
```

## 📊 Benchmarking

### Stress Tests (Google Benchmark)

```bash
# Run all benchmarks
./build/release/stress_test

# Run specific benchmark
./build/release/stress_test --benchmark_filter=AllocationSpeed

# Custom parameters
./build/release/stress_test \
  --benchmark_min_time=5s \
  --benchmark_repetitions=3 \
  --min-order 8 \
  --max-order 18
```

### Expected Performance

On a modern CPU (e.g., Apple M1, Intel i7-12700K):

- **Allocation throughput**: ~1-10 million ops/sec (size-dependent)
- **Deallocation throughput**: ~2-15 million ops/sec
- **Latency p50**: < 100 nanoseconds
- **Latency p99**: < 1 microsecond

Results vary based on allocation size, fragmentation level, and system load.

## 📈 Visualization

### Generate Plots

```bash
# Install Python dependencies
pip install -r requirements.txt

# Generate all plots from CSV files in reports/
python src/main/main.py

# Specify input files and output directory
python src/main/main.py \
  --input reports/performance_tests_*.csv \
  --output reports/plots \
  --timestamp

# Generate specific plots
python src/main/main.py \
  --plots allocation_latency_percentiles memory_usage_over_time
```

### Available Plots

1. **Total Memory Usage Over Time** - Cumulative memory allocation timeline
2. **Allocation/Deallocation Rates** - Operation rates per time interval
3. **Allocation Latency Over Time** - Raw latency measurements
4. **Allocation Latency Percentiles** - p50, p95, p99 latencies (NEW!)
5. **Allocation Size Distribution** - Histogram of allocation sizes
6. **Memory Usage by Source** - Memory consumption per function/module
7. **Number of Allocations by Source** - Allocation count per source
8. **Average Allocation Latency by Source** - Mean latency per source
9. **Allocation Size vs. Time Heatmap** - 2D heatmap visualization
10. **Call Stack Trace Frequency** - Allocation frequency by call stack
11. **Throughput Trends** - Throughput over multiple benchmark runs

### Example Output

Plots are saved to `reports/plots/<timestamp>/` with filenames like:
- `performance_tests_allocation_latency_percentiles.png`
- `allocator_tests_total_memory_usage_over_time.png`

## 🏗️ Architecture

### Buddy Allocation Algorithm

The allocator maintains a pool of memory blocks organized by size (powers of 2). Each block is tracked by its "order" (log₂ of size):

```
Order 10 (1024 bytes): [--------Free--------]
                          ↓ Split
Order 9  (512 bytes):  [--Free--][--Free--]
                          ↓ Allocate
Order 9  (512 bytes):  [--Used--][--Free--]
```

**Key Operations:**

- **Allocate**: Find smallest free block ≥ requested size. Split larger blocks if needed.
- **Deallocate**: Mark block as free. Merge with buddy if both free (coalescing).
- **Split**: Divide block into two equal halves (buddies).
- **Coalesce**: Merge adjacent free buddy blocks into larger block.

### Components

```
src/
├── allocator/
│   ├── custom_allocator.h   # Buddy allocator interface
│   └── custom_allocator.cpp # Core allocation logic
├── logger/
│   ├── data_logger.h         # CSV logging interface
│   └── data_logger.cpp       # Thread-safe logging
├── config/
│   ├── config_manager.h      # Configuration management
│   └── config_manager.cpp    # TOML + CLI parsing
└── tests/
    ├── unit_tests.cpp        # GoogleTest unit tests
    ├── allocator_tests.cpp   # Integration tests
    ├── performance_tests.cpp # Performance benchmarks
    └── stress_test.cpp       # Google Benchmark stress tests
```

### Thread Safety

The allocator is fully thread-safe:
- All operations protected by `std::mutex`
- Atomic counters for statistics
- Lock-free reads for metrics (fragmentation, throughput)

## 📄 CSV Schema

All test executables output CSV files with the following schema:

| Column | Type | Description |
|--------|------|-------------|
| `Timestamp` | DateTime | ISO 8601 timestamp (YYYY-MM-DD HH:MM:SS) |
| `Operation` | String | "Allocation", "Deallocation", or "Summary" |
| `BlockSize` | Integer | Size of allocated/deallocated block (bytes) |
| `Time` | Float | Operation latency (seconds) or throughput (ops/sec) |
| `Fragmentation` | Float | Memory fragmentation ratio (0.0-1.0) |
| `Source` | String | Function/module name that triggered operation |
| `CallStack` | String | Call stack trace (simplified) |
| `MemoryAddress` | String | Hexadecimal memory address |
| `ThreadID` | String | Thread identifier |
| `AllocationID` | String | Unique allocation identifier (e.g., "Alloc42") |

**Note**: Summary rows have `Operation="Summary"` and use `Time`/`Fragmentation` columns differently:
- `Time`: Allocation throughput (ops/sec)
- `Fragmentation`: Deallocation throughput (ops/sec)
- `Source`: Final fragmentation percentage

## 🔄 CI/CD Pipeline

### GitHub Actions Workflows

#### CI Workflow (`.github/workflows/ci.yml`)

Runs on every push to `main` and on pull requests:

1. **Build & Test** (Ubuntu, macOS, Windows)
   - Debug and Release builds
   - Run unit tests and integration tests
   - Upload test results as artifacts

2. **Sanitizers** (Ubuntu, Clang 15)
   - AddressSanitizer + UndefinedBehaviorSanitizer
   - Catch memory leaks and undefined behavior

3. **Static Analysis** (Ubuntu)
   - clang-tidy checks for code quality

4. **Format Check** (Ubuntu)
   - clang-format verification (Google C++ style)

5. **Benchmarks** (Ubuntu, `main` branch only)
   - Run stress tests
   - Generate plots
   - Upload CSV and PNG artifacts

#### Release Workflow (`.github/workflows/release.yml`)

Triggers on Git tags (`v*`):

1. Create GitHub Release
2. Build release artifacts for Linux, macOS, Windows
3. Upload platform-specific archives (`.tar.gz` / `.zip`)

### Required Checks

For branch protection, mark as required:
- `build-and-test` (all platforms)
- `sanitizers`
- `format-check`
- `static-analysis`

## 🤝 Contributing

Contributions are welcome! Please follow these guidelines:

1. **Fork the repository** and create a feature branch
2. **Run tests** locally before submitting PR:
   ```bash
   cmake --preset sanitizers && cmake --build build/sanitizers
   ctest --test-dir build/sanitizers --output-on-failure
   ```
3. **Format code** with clang-format:
   ```bash
   find src -name '*.cpp' -o -name '*.h' | xargs clang-format -i
   ```
4. **Write tests** for new features
5. **Update documentation** if adding new features
6. **Submit PR** with clear description

### Code Style

- Follow Google C++ Style Guide (enforced by `.clang-format`)
- Use RAII for resource management
- Document public APIs with Doxygen comments
- Keep functions under 50 lines when possible

## 📝 License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **Buddy Allocation Algorithm**: Donald Knuth, *The Art of Computer Programming*
- **Google Benchmark**: Performance measurement framework
- **GoogleTest**: Unit testing framework
- **toml11**: TOML parser for C++

## 📧 Contact

**James Olaitan**

- GitHub: [@jamesolaitan](https://github.com/jamesolaitan)
- Project Link: [https://github.com/jamesolaitan/Dynamic-Memory-Allocator-Performance-Visualization-Tool](https://github.com/jamesolaitan/Dynamic-Memory-Allocator-Performance-Visualization-Tool)

---

**Built with ❤️ using modern C++17 and production-grade engineering practices.**
