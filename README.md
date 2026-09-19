# Mini C Compiler

A small educational C compiler built from first principles in C. The project focuses on understanding how a compiler transforms source code into machine-level instructions rather than hiding the process behind a parser generator or compiler framework.

> **Status:** v1.0 - expressions, local variables and control flow

## What it can compile

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
