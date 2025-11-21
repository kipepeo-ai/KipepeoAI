# Integration Tests

Integration tests for end-to-end functionality.

## Test Structure

- `test_android_integration.cpp` - Android NDK integration tests
- `test_llm_video_pipeline.cpp` - LLM + video compression pipeline tests
- `test_performance.cpp` - Performance benchmarks

## Running Tests

```bash
cd build
ctest -R integration
```

