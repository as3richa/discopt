#!/bin/bash

set -euo pipefail

BINARY=bin/$1
DATADIR=data/$1
FILENAMES=$(cat ${DATADIR}/assignment.list)

for f in ${FILENAMES}; do
  COMMAND="${BINARY} <${DATADIR}/${f} >${DATADIR}/${f}.out 2>/dev/null"
  echo "++ ${COMMAND}"
  eval "time ${COMMAND}"
done
