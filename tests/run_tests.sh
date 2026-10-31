#!/usr/bin/env bash
cd ~/csce465-agentsec/hw3/csce465-lab3 || exit 1
BIN=./legacy_log_parser_fixed
D=tests/data
mkdir -p "$D" markers
pass=0; fail=0

# check NAME EXPECTED_EXIT FILE
check() {
  local name=$1 want=$2 file=$3
  out=$($BIN "$file" 2>&1); rc=$?
  if [ "$rc" -eq "$want" ]; then
    echo "PASS: $name (exit $rc)"; pass=$((pass+1))
  else
    echo "FAIL: $name (exit $rc, wanted $want)"; fail=$((fail+1))
  fi
  echo "      output: $out" | head -n 3
}

printf 'hello log\n' > $D/short.log
head -c 128 /dev/zero | tr '\0' 'A' > $D/max128.log
head -c 129 /dev/zero | tr '\0' 'A' > $D/over129.log

check "valid short input"            0 $D/short.log
check "exactly max (128 bytes)"      0 $D/max128.log
check "one byte over (129 bytes)"    1 $D/over129.log
check "missing file"                 2 $D/does_not_exist.log

# original exploit payload: must be rejected, no segfault, no marker
rm -f markers/ret2win.txt
check "original exploit payload"     1 payload.bin
if [ -e markers/ret2win.txt ]; then
  echo "FAIL: marker was created, exploit worked!"; fail=$((fail+1))
else
  echo "PASS: marker NOT created"; pass=$((pass+1))
fi

echo "== $pass passed, $fail failed =="
