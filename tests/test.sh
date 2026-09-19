#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

make clean >/dev/null
make >/dev/null

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

run_case() {
  local expected="$1"
  local source="$2"
  printf '%s\n' "$source" | ./mini-cc > "$TMP/test.s"
  gcc "$TMP/test.s" -o "$TMP/test"
  set +e
  "$TMP/test"
  local actual=$?
  set -e
  if [[ "$actual" -ne "$expected" ]]; then
    echo "FAIL: expected $expected, got $actual"
    echo "SOURCE: $source"
    exit 1
  fi
  echo "PASS: $source -> $actual"
}

run_case 42 'int main() { return 2 + 5 * 8; }'
run_case 9 'int main() { return (2 + 1) * 3; }'
run_case 6 'int main() { return 12 / 2; }'
run_case 1 'int main() { return 10 > 3; }'
run_case 0 'int main() { return 10 == 3; }'
run_case 1 'int main() { return -1 < 2; }'
run_case 12 'int main() { int x = 5; x = x + 7; return x; }'
run_case 55 'int main() { int total = 0; int i = 1; while (i <= 10) { total = total + i; i = i + 1; } return total; }'
run_case 100 'int main() { int x = 7; if (x == 7) { return 100; } else { return 1; } }'
run_case 15 'int main() { int x; x = 15; return x; }'
run_case 1 'int main() { int x = 4; int y = 4; return x == y; }'
run_case 1 'int main() { int x = 4; int y = 5; return x != y; }'

./mini-cc < examples/control_flow.c > "$TMP/example.s"
gcc "$TMP/example.s" -o "$TMP/example"
set +e
"$TMP/example"
example_status=$?
set -e
[[ "$example_status" -eq 155 ]] || { echo "FAIL: control_flow example returned $example_status, expected 155"; exit 1; }

echo "All compiler tests passed."