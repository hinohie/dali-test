#!/bin/bash

# Retrieve all the installed tests
tests=$( ls -d $DESKTOP_PREFIX/bin/*.test )
num_tests=${#tests[@]}
num_passes=0
num_fails=0

# Execute each test executable in turn
for i in $tests ; do
  test=$(basename $i)
  echo Executing: $test
  $i

  # Check the test result
  if [ "$?" != "0" ]; then
    echo "$test Failed"
    ((num_fails++))
  else
    echo "$test Passed"
    ((num_passes++))
  fi
done

# Output the summary of test result
echo "Test Summary:"
echo "Total tests: $num_tests"
echo "$num_passes passed"
echo "$num_fails failed"

