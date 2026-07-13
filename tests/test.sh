#!/bin/bash

set -euo pipefail

INPUT=$1
OUTPUT=${INPUT##*/}.output
REFERENCE=$2
./acc_tester.out --allow-invalid "${INPUT}"
diff "${OUTPUT}" "${REFERENCE}"
