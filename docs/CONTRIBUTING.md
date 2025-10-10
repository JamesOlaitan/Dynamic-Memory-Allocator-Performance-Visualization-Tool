# Contributing to Buddy Allocator

Thank you for considering contributing to the Dynamic Memory Allocator Performance Visualization Tool! This document provides guidelines and instructions for contributing.

## Code of Conduct

- Be respectful and inclusive
- Welcome newcomers and help them learn
- Focus on constructive feedback
- Maintain professional communication

## Getting Started

1. **Fork the repository** on GitHub
2. **Clone your fork** locally:
   ```bash
   git clone https://github.com/YOUR_USERNAME/Dynamic-Memory-Allocator-Performance-Visualization-Tool.git
   cd Dynamic-Memory-Allocator-Performance-Visualization-Tool
   ```
3. **Create a branch** for your changes:
   ```bash
   git checkout -b feature/your-feature-name
   ```

## Development Workflow

### Building and Testing

```bash
# Configure with sanitizers for development
cmake --preset sanitizers

# Build
cmake --build build/sanitizers

# Run all tests
ctest --test-dir build/sanitizers --output-on-failure

# Run specific test
./build/sanitizers/unit_tests --gtest_filter=CustomAllocatorTest.*
```

### Code Style

We follow the **Google C++ Style Guide** with minor modifications (see `.clang-format`).

**Before committing, format your code:**

```bash
# Format all C++ files
find src -name '*.cpp' -o -name '*.h' | xargs clang-format -i

# Check formatting (what CI does)
find src -name '*.cpp' -o -name '*.h' | xargs clang-format --dry-run --Werror
```

**Key style points:**
- 4 spaces for indentation (no tabs)
- 120 character line limit
- camelCase for functions and variables
- CamelCase for classes and structs
- Use descriptive names
- Document public APIs with Doxygen comments

### Static Analysis

Run clang-tidy before submitting:

```bash
cmake --preset debug
find src -name '*.cpp' | xargs clang-tidy -p build/debug
```

### Writing Tests

**All new features must include tests.**

#### Unit Tests (GoogleTest)

Add tests to `src/tests/unit_tests.cpp`:

```cpp
TEST(CustomAllocatorTest, YourNewFeature) {
    CustomAllocator allocator(6, 20);
    // Test your feature
    EXPECT_EQ(expected, actual);
}
```

#### Integration Tests

Add test functions to `src/tests/allocator_tests.cpp` for real-world scenarios.

#### Benchmarks

Add benchmarks to `src/tests/stress_test.cpp` for performance-critical changes:

```cpp
BENCHMARK_DEFINE_F(AllocatorFixture, YourBenchmark)(benchmark::State& state) {
    for (auto _ : state) {
        // Benchmark code
    }
}
BENCHMARK_REGISTER_F(AllocatorFixture, YourBenchmark)->Arg(1000);
```

## Submitting Changes

### Commit Messages

Follow conventional commits format:

```
type(scope): subject

body (optional)

footer (optional)
```

**Types:**
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style/formatting
- `refactor`: Code refactoring
- `test`: Adding/updating tests
- `perf`: Performance improvements
- `ci`: CI/CD changes

**Examples:**
```
feat(allocator): add double-free detection

Implement check to detect and prevent double-free errors.
Adds new test case to verify behavior.

Closes #42
```

```
fix(logger): correct timestamp formatting on Windows

Use localtime_s instead of localtime_r for MSVC compatibility.
```

### Pull Request Process

1. **Ensure all tests pass locally**
   ```bash
   cmake --preset sanitizers
   cmake --build build/sanitizers
   ctest --test-dir build/sanitizers
   ```

2. **Update documentation** if needed
   - Update README.md for new features
   - Add/update Doxygen comments
   - Update CHANGELOG.md (if exists)

3. **Create a Pull Request**
   - Use a clear, descriptive title
   - Reference related issues (e.g., "Closes #42")
   - Describe what changed and why
   - Include screenshots for UI changes (plots)

4. **Respond to code review**
   - Address all comments
   - Push new commits (don't force-push during review)
   - Mark conversations as resolved

5. **CI must pass**
   - All build configurations (Linux/macOS/Windows)
   - All tests
   - Sanitizers
   - Format check
   - Static analysis

### What to Include

✅ **Do include:**
- Unit tests for new functionality
- Documentation updates
- Benchmark results for performance changes
- Error handling
- Thread-safety considerations

❌ **Don't include:**
- Unrelated changes (create separate PRs)
- Commented-out code
- Debug print statements
- Generated files (build/, reports/)
- IDE-specific files

## Areas for Contribution

### High Priority

- 🐛 **Bug fixes** - Always welcome!
- 📝 **Documentation improvements** - Help others understand the code
- ✅ **Test coverage** - Add more test cases
- 🚀 **Performance optimizations** - With benchmarks to prove improvements

### Feature Ideas

- Support for custom alignment requirements per allocation
- Memory pool statistics and visualization
- JSON output format support
- Additional allocator strategies (slab, arena)
- Python bindings using pybind11
- Heap profiling mode (track all allocations)
- Memory access pattern analysis
- Integration with Tracy profiler

### Good First Issues

Look for issues labeled `good first issue` on GitHub. These are:
- Well-defined scope
- Good learning opportunities
- Mentorship available

## Questions?

- Open a [Discussion](https://github.com/jamesolaitan/Dynamic-Memory-Allocator-Performance-Visualization-Tool/discussions) for general questions
- Open an [Issue](https://github.com/jamesolaitan/Dynamic-Memory-Allocator-Performance-Visualization-Tool/issues) for bugs or feature requests
- Tag maintainers for review

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

Thank you for contributing! 🎉

