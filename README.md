# Mini C Compiler

[![C Compiler CI](https://github.com/jaimin-valand/mini-c-compiler/actions/workflows/ci.yml/badge.svg)](https://github.com/jaimin-valand/mini-c-compiler/actions/workflows/ci.yml)

> **A from-scratch educational C compiler that turns a small C-like language into x86-64 assembly.**

Mini C Compiler is a compact systems-programming project designed to make the compiler pipeline visible from end to end. Instead of relying on a parser generator or compiler framework, the implementation contains its own lexer, recursive-descent parser, AST, symbol resolution, stack-frame layout and x86-64 code generator.

## Why this project?

Compilers are a practical way to understand what happens between source code and executable software.

This project deliberately keeps the supported language small enough to inspect while still covering the core stages of a real compiler:

```text
C source
   |
   v
Lexer -> Parser -> AST -> Symbol Resolution -> x86-64 Codegen
                                                |
                                                v
                                      assembler + linker
                                                |
                                                v
                                            executable
```

## Supported language

- `int main() { ... }`
- integer literals
- local integer variables
- variable initialisation
- assignment
- unary negation
- `+`, `-`, `*`, `/`
- `<`, `<=`, `>`, `>=`, `==`, `!=`
- `if` / `else`
- `while`
- `return`
- nested blocks
- `//` comments
- source-location-aware parser errors

The output is GNU-style x86-64 assembly intended for Unix-like toolchains using the System V AMD64 ABI.

## Quick start

### Requirements

A Unix-like development environment with:

- C11 compiler: GCC or Clang
- GNU-compatible assembler/linker
- GNU Make
- Bash

### Build

```bash
make
```

### Run the automated test suite

```bash
make test
```

The tests compile the compiler, generate assembly from several programs, assemble/link those programs and verify their exit codes.

### Compile a program

```bash
echo 'int main() { int x = 20; return x * 2 + 2; }' | ./mini-cc > program.s
gcc program.s -o program
./program
echo $?
```

Expected:

```text
42
```

### Run the included examples

```bash
make examples
```

Examples are provided under `examples/`.

## Example

Input:

```c
int main() {
    int total = 0;
    int i = 1;

    while (i <= 10) {
        if (i == 7) {
            total = total + 100;
        } else {
            total = total + i;
        }

        i = i + 1;
    }

    return total;
}
```

The compiler produces assembly that can be passed directly to the system assembler/linker.

## Engineering highlights

### Hand-written lexer

The lexer recognises keywords, identifiers, integer literals, operators and punctuation while ignoring whitespace and line comments.

### Recursive-descent parser

Expression precedence is encoded directly in the parser:

```text
assignment
   |
equality
   |
comparison
   |
addition / subtraction
   |
multiplication / division
   |
unary
   |
primary
```

This keeps the grammar easy to inspect and extend.

### AST representation

The parser builds an explicit expression/statement tree rather than generating machine code while parsing. That separation makes later semantic analysis and optimisation possible.

### Symbol resolution

Local variables are resolved into stack-frame offsets. Undefined variables and duplicate declarations are rejected before assembly is generated.

### x86-64 code generation

Expressions are evaluated using `%rax`, `%rcx` and the machine stack. Control flow is emitted using generated labels for `if` and `while` statements.

### End-to-end validation

Tests do more than check parser functions: they compile complete source programs, assemble the generated output and execute the resulting binaries.

## Project structure

```text
mini-c-compiler/
├── .github/
│   └── workflows/
│       └── ci.yml
├── docs/
│   ├── architecture.md
│   └── roadmap.md
├── examples/
│   ├── arithmetic.c
│   ├── control_flow.c
│   └── factorial.c
├── src/
│   └── main.c
├── tests/
│   └── test.sh
├── .gitignore
├── LICENSE
├── Makefile
└── README.md
```

## Portability

The compiler implementation uses standard C11.

The generated assembly is **x86-64 specific** and currently targets Unix-like environments using GNU/Clang-compatible assembler and linker tooling. It is not a native Windows/MSVC compiler.

For a reproducible environment, use Ubuntu, Debian, WSL or another Linux distribution with GCC/Clang and Make installed.

## Development roadmap

### Completed

- [x] Lexer
- [x] Recursive-descent parser
- [x] AST
- [x] Integer expressions
- [x] Local variables
- [x] Assignment
- [x] Comparisons
- [x] `if` / `else`
- [x] `while`
- [x] Stack-frame allocation
- [x] x86-64 assembly generation
- [x] End-to-end tests
- [x] GitHub Actions CI

### Planned

- [ ] logical `&&` / `||` with short-circuit evaluation
- [ ] `for`, `break`, `continue`
- [ ] multiple functions
- [ ] function parameters and calls
- [ ] ABI-aware argument passing
- [ ] pointers
- [ ] arrays
- [ ] richer type checking
- [ ] explicit intermediate representation
- [ ] optimisation passes
- [ ] improved diagnostics with source spans
- [ ] backend abstraction

## Learning outcomes

This project demonstrates practical understanding of:

- lexical analysis
- parsing and grammar design
- abstract syntax trees
- symbol tables
- semantic validation
- stack-frame layout
- control-flow translation
- x86-64 assembly
- compiler testing
- CI automation
- systems-level C programming

## Licence

MIT. See [LICENSE](LICENSE).

---

**Built as part of a systems-engineering portfolio exploring how fundamental software infrastructure works from first principles.**