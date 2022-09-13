#!/bin/bash
DEBUG=0
if [ "$1" = "-d" ]; then
    DEBUG=1
    shift
fi
INP="$1"
if [ ! -f "$INP" ]; then
    echo "Error: input file not found"
    exit 1
fi
BASENAME="${INP%.bf}"
BFRT="$(dirname "$0")/bfrt.o"
BFC="$(dirname "$0")/bfc"

if [ $DEBUG = 1 ]; then
    cat "$INP" | "$BFC" >"$BASENAME.s"
    as -o "$BASENAME.o" "$BASENAME.s"
    ld -o "$BASENAME" "$BFRT" "$BASENAME.o"
else
    OBJFILE="$(mktemp tmp.XXXXXX.o)"
    cat "$INP" | "$BFC" | as -o "$OBJFILE"
    ld -o "$BASENAME" "$BFRT" "$OBJFILE"
    rm -f "$OBJFILE"
fi
