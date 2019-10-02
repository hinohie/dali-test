#!/bin/bash

# Usage Function
Usage()
{
  echo "Usage: $(basename ${BASH_SOURCE[0]}) [OPTIONS]"
  echo " Optional Options:"
  # Just do a simple grep of this file so we can keep the command with the comment together
  grep ". )\ #" ${BASH_SOURCE[0]} | sed 's/# //' | awk -F ")" '{ printf "%-25s %s\n", $1, $2 }'
  exit 0
}

# Initialise the options
OPTS=$(getopt -o dh --long debug,help -n "$(basename "$0")" -- "$@")
if [ $? != 0 ]; then echo; Usage; fi
eval set -- "$OPTS"

# Go through all the options
if [[ $* > 1 ]] ; then
  while true;
  do
    case "$1" in
      -d|--debug ) # A Debug build of the test cases is done
        DEBUG_BUILD="-DCMAKE_BUILD_TYPE=Debug"
        shift
        ;;

      -h|--help ) # Help
        shift
        Usage
        ;;

      -- )
        shift
        ;;

    * )
      break
      ;;

    esac
  done
fi

command="( cd build/tizen ; cmake -DCMAKE_INSTALL_PREFIX=$DESKTOP_PREFIX ${DEBUG_BUILD} . ; make install -j8 )"
echo $command
eval $command
