#!/bin/bash

declare -a TESTS
TESTS[0]="arithmetic"



for i in "${TESTS[@]}"
do
  echo "Testing $i..."
  rm examples/tests/$i.sf.*

  ./sf examples/tests/$i.sf &> /dev/null
  diff "examples/tests/$i.tp" "examples/tests/$i.sf.ps2"
	diff "examples/tests/$i.ti" "examples/tests/$i.sf.icg"
done
