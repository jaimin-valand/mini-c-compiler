# Getting Started

## Option 1 - Native Linux / WSL

Install GCC/Clang and Make, then:

```bash
git clone https://github.com/jaimin-valand/mini-c-compiler.git
cd mini-c-compiler
make test
```

Compile a program:

```bash
printf 'int main() { int x = 20; return x * 2 + 2; }\n' | ./mini-cc > program.s
gcc program.s -o program
./program
echo $?
```

## Option 2 - Docker

No local compiler installation is required.

```bash
git clone https://github.com/jaimin-valand/mini-c-compiler.git
cd mini-c-compiler
docker build -t mini-c-compiler .
docker run --rm -it mini-c-compiler
```

The image builds the compiler and executes the full test suite during `docker build`.

## Option 3 - GitHub Codespaces

Open the repository in GitHub Codespaces. The included `.devcontainer/devcontainer.json` installs a C++ development container and automatically runs `make test`.

## Platform note

The compiler is implemented in portable C11, but its generated output currently targets x86-64 GNU assembly. For Windows, use WSL, a Linux container, or another Unix-like x86-64 environment.

## Verify the toolchain

```bash
cc --version
make --version
gcc --version
```

Then run:

```bash
make clean
make test
```
