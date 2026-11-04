# Diff

A small C++ tool that prints the minimum insertions and deletions needed to turn
one string into another, followed by an edit script. A replacement costs two
edits: one deletion and one insertion.

## Build and run

Requires a C++17 compiler.

```sh
c++ -std=c++17 -O2 diff.cpp -o diff
./diff abc abd
```

Output:

```text
2
ab{-c}{+d}
```

`{+text}` inserts text; `{-text}` removes it. Unmarked text stays unchanged.
Literal braces and backslashes are escaped as `\{`, `\}`, and `\\`;
newlines, carriage returns, and tabs use `\n`, `\r`, and `\t`.

Quote arguments containing spaces, or use `""` for an empty string:

```sh
./diff "hello world" "hello there"
./diff "" abc
./diff abc ""
```

The original stdin format is also supported: `START_LENGTH START FINAL_LENGTH FINAL`.
Strings in this format are whitespace-delimited. For an empty string, write `0`
and omit its token; for example, `0 3 abc` compares an empty string with `abc`.
Declared lengths must match. Successful comparisons exit with code 0; invalid
input exits with code 2. Run `./diff --help` for usage.

## Algorithm

The original two-row dynamic programming and divide-and-conquer path
reconstruction are retained. Work rows are freed before recursion, keeping
auxiliary storage linear: O(n + m), including the path. Runtime remains quadratic.
Lengths and edit counts operate on bytes, so ASCII strings are the intended input.
When several minimum scripts exist, one is printed.

## CMake and tests

CMake is optional. Python 3 is used for the regression tests.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The executable is `build/diff` on Unix, or `build\Release\diff.exe` with Visual Studio.
Tests compare counts against an independent LCS oracle and replay every script,
including 961 exhaustive small-string pairs, randomized inputs, trailing edits,
empty strings, literal edit markers, and invalid lengths. CI covers Linux,
macOS, and Windows.
