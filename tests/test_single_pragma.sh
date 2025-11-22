#!/bin/bash
#
# test_single_pragma.sh - Test pragma file(s)
#
# This script validates that a pragma file can be successfully parsed.
# Handles both single-pragma and multi-pragma files.
#
# Usage: ./test_single_pragma.sh <pragma_file> <acc_roundtrip_binary>
#
# Exit codes:
#   0: All pragmas parsed successfully
#   1: One or more pragmas failed to parse
#

set -euo pipefail

if [ $# -lt 2 ]; then
    echo "Usage: $0 <pragma_file> <acc_roundtrip_binary>"
    exit 1
fi

PRAGMA_FILE="$1"
ROUNDTRIP_BIN="$2"

if [ ! -f "$PRAGMA_FILE" ]; then
    echo "Error: Pragma file not found: $PRAGMA_FILE"
    exit 1
fi

if [ ! -x "$ROUNDTRIP_BIN" ]; then
    echo "Error: acc_roundtrip binary not found or not executable: $ROUNDTRIP_BIN"
    exit 1
fi

# Process each pragma line in the file
failed=0
line_num=0

while IFS= read -r pragma || [ -n "$pragma" ]; do
    line_num=$((line_num + 1))

    # Skip empty lines
    if [ -z "$pragma" ]; then
        continue
    fi

    # Try to parse and unparse the pragma
    if ! roundtrip_output="$("$ROUNDTRIP_BIN" "$pragma" 2>/dev/null)"; then
        echo "Parse failed at line $line_num: $pragma"
        failed=$((failed + 1))
        continue
    fi

    # Normalise whitespace for comparison so formatting differences don't mask
    # structural mismatches.
    normalize() {
    echo "$1" | tr '\n' ' ' \
      | sed -e 's/[[:space:]]\+/ /g' \
             -e 's/ *: */:/g' \
             -e 's/, */,/g' \
             -e 's/ ,/,/g' \
             -e 's/) *, */) /g' \
             -e 's/ *( */(/g' \
             -e 's/^ //;s/ $//'
    }

    norm_input=$(normalize "$pragma")
    norm_output=$(normalize "$roundtrip_output")

    if [ "$norm_input" != "$norm_output" ]; then
        echo "Roundtrip mismatch at line $line_num:"
        echo "  input : $norm_input"
        echo "  output: $norm_output"
        failed=$((failed + 1))
    fi
done < "$PRAGMA_FILE"

if [ $failed -gt 0 ]; then
    echo "Parse failed: $failed pragma(s) failed"
    exit 1
fi

# If we got here, all pragmas parsed successfully
exit 0
