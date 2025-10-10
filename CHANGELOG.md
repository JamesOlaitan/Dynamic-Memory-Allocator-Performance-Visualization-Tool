# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-10-10

### Added
- 🔧 **CMake Build System** with presets (Debug, Release, ASan, UBSan, Sanitizers)
- ⚙️ **TOML Configuration** with CLI override support (no hard-coded values)
- 📋 **ConfigManager** class for centralized configuration management
- 🧪 **Comprehensive GoogleTest Suite** with 30+ test cases
  - Split/coalesce invariants
  - Alignment verification
  - Boundary conditions
  - Thread safety
  - Fragmentation metrics
  - Metadata integrity
- 📊 **Latency Percentile Visualization** (p50, p95, p99)
- 🔄 **CI/CD Pipeline** via GitHub Actions
  - Multi-platform builds (Linux, macOS, Windows)
  - Sanitizers job (ASan + UBSan)
  - Static analysis (clang-tidy)
  - Format checking (clang-format)
  - Automated benchmarks on main branch
- 📦 **Release Automation** for tagged versions
- 📝 **Documentation**
  - README with badges
  - CONTRIBUTING guidelines
  - Architecture documentation
  - CSV schema specification
- 🎨 **Pure Matplotlib Visualizations** (removed seaborn dependency)
- 📁 **Timestamped Output** for reports and plots
- 🗂️ **Standardized Directory Structure** (`reports/`, `config/`, `docs/`)

### Changed
- 🔀 **Removed Hard-Coded Values** from all test executables
- 📍 **Output Directory** changed from root to `reports/` subdirectory
- 📈 **Visualization Style** modernized with consistent matplotlib styling
- 🐍 **Python Requirements** simplified (removed unnecessary dependencies)

### Fixed
- 🐛 **Thread-Safety Issues** in allocator metrics
- 🔒 **Cross-Platform Compatibility** for timestamp formatting
- 🧵 **Race Conditions** in concurrent allocation tests

### Technical Details
- **C++ Standard**: C++17
- **Build System**: CMake 3.20+
- **Dependencies**: GoogleTest 1.14.0, Google Benchmark 1.8.3, toml11 3.8.1
- **Testing**: 30+ unit tests, 3 integration test modes, 3 benchmark suites
- **Metrics**: 8+ captured per operation (timestamp, latency, fragmentation, etc.)
- **Visualizations**: 11 plot types including new percentile plots
- **Platforms**: Linux, macOS, Windows (all tested in CI)

## [0.1.0] - 2024-XX-XX (Pre-production)

### Added
- Initial buddy allocator implementation
- Basic logging to CSV
- Manual build process
- Basic visualization with seaborn
- Simple test cases

### Known Issues (Resolved in 1.0.0)
- Hard-coded configuration values
- Limited test coverage
- No CI/CD pipeline
- Platform-specific code
- Inconsistent output paths

---

## Release Notes

### Version 1.0.0 - Production Release

Key highlights:

**For Users:**
- Easy configuration via TOML files or CLI
- Cross-platform support (Linux, macOS, Windows)
- Professional visualization suite
- Comprehensive documentation

**For Developers:**
- Modern CMake build system
- Automated testing and CI/CD
- Code quality tools (sanitizers, clang-tidy, clang-format)
- Extensive test coverage
- Clear contribution guidelines

**Performance:**
- Allocation throughput: 1-10M ops/sec (size-dependent)
- Deallocation throughput: 2-15M ops/sec
- p99 latency: < 1 microsecond
- Thread-safe with mutex protection

### Migration Guide from 0.x

If upgrading from previous version:

1. **Build System**: Replace manual compile commands with CMake presets
   ```bash
   # Old
   g++ -o allocator_tests src/tests/allocator_tests.cpp ...
   
   # New
   cmake --preset release && cmake --build build/release
   ```

2. **Configuration**: Move hard-coded values to `config/default.toml`

3. **Output Paths**: Update paths to use `reports/` directory

4. **Visualization**: Remove seaborn imports, use updated main.py

5. **Tests**: Run new comprehensive test suite to verify compatibility

---

For detailed changes, see the [commit history](https://github.com/jamesolaitan/Dynamic-Memory-Allocator-Performance-Visualization-Tool/commits/main).

