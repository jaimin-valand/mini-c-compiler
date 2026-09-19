# Architecture

## Compiler pipeline

```text
C source
   |
   v
Lexer / tokenizer
   |
   v
Recursive-descent parser
   |
   v
AST + local symbol table
   |
   v
x86-64 code generator
   |
   v
GNU assembler + linker
   |
   v
Executable
```

## Lexer

The lexer converts source characters into tokens. It recognises keywords, identifiers, integer literals, operators and punctuation. It also skips whitespace and `//` comments.

## Parser

The parser is a recursive-descent parser. Operator precedence is encoded directly in the call hierarchy:

```text
assignment
  equality
    comparison
      addition
        multiplication
          unary
            primary
```

Statements currently include declarations, assignments, returns, blocks, `if`/`else`, `while`, and expression statements.

## Symbol table and stack frame

Every local variable receives an 8-byte stack slot. The symbol table stores the variable name and its negative `%rbp` offset. A small integer-only language can therefore use a straightforward stack frame without a general register allocator.

The generated function uses the conventional x86-64 frame shape:

```asm
push %rbp
mov  %rsp, %rbp
sub  $N, %rsp
...
leave
ret
```

## Expression generation

Expressions are evaluated into `%rax`. For binary operators the right-hand side is temporarily pushed onto the stack, then the left-hand side is evaluated into `%rax`. The right-hand side is restored into `%rcx`.

Comparisons use `cmp` followed by a `set*` instruction and zero-extension of `%al` into `%rax`.

## Control flow

`if` and `while` statements receive unique assembly labels. Conditions are evaluated as integer truth values: zero is false and any non-zero value is true.

## Deliberate limitations

This is an educational compiler, not a production C implementation. It currently has:

- only the `main` function;
- only the `int` type;
- no pointers or arrays;
- no function calls;
- no `for`, `switch`, `break`, or `continue`;
- no preprocessor;
- no full C type system;
- no ABI-aware multi-function calling convention;
- no optimisation passes.

These limitations are intentional so each compiler stage remains inspectable and testable.