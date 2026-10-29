# ps5-nid-map

C++ library providing PS5 import mappings:

- PRX filenames to module names.
- NIDs to export library names, grouped by module.

Key lookup uses two-level perfect hashing with seeds selected at compile time and NIDs stored as 64-bit integers. Tables reside in static storage, not on the stack.

On an Intel Core i5-7500 at 3.40 GHz, mean lookup times were 52 ns for filenames, 40 ns for modules and 113 ns for NIDs (MinGW Release; medians of three runs over all keys).

## License

This project is licensed under the [MIT License](LICENSE).
