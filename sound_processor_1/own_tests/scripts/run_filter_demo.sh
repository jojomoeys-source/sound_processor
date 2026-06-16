#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="${PROJECT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)}"
BIN="${BIN:-$PROJECT_DIR/build/sound_processor}"
OUT_DIR="${OUT_DIR:-$PROJECT_DIR/own_tests/filter_outputs}"
SAMPLES_DIR="${SAMPLES_DIR:-$PROJECT_DIR/own_tests/samples}"
INPUT="${INPUT:-$SAMPLES_DIR/input.wav}"
OVERLAY="${OVERLAY:-$SAMPLES_DIR/overlay.wav}"

mkdir -p "$OUT_DIR" "$SAMPLES_DIR"

if [[ ! -f "$INPUT" ]]; then
    "$BIN" -o "$INPUT" -f generator sin 440 2000
fi

"$BIN" -o "$OVERLAY" -f generator sin 880 2000

"$BIN" -i "$INPUT" -o "$OUT_DIR/ampl.wav" -f ampl 0.5
"$BIN" -i "$INPUT" -o "$OUT_DIR/normalize.wav" -f normalize
"$BIN" -i "$INPUT" -o "$OUT_DIR/silence.wav" -f silence sec 0.5 0.7
"$BIN" -i "$INPUT" -o "$OUT_DIR/timestretch.wav" -f timestretch 1.5
"$BIN" -i "$INPUT" -o "$OUT_DIR/lowpass.wav" -f lowpass 51
"$BIN" -i "$INPUT" -o "$OUT_DIR/highpass.wav" -f highpass 51
"$BIN" -i "$INPUT" -o "$OUT_DIR/bandpass.wav" -f bandpass 101 21
"$BIN" -i "$INPUT" -o "$OUT_DIR/reject.wav" -f reject 101 21
"$BIN" -i "$INPUT" -o "$OUT_DIR/mute.wav" -f mute 0.5 0.7
"$BIN" -i "$INPUT" -o "$OUT_DIR/mix.wav" -f mix "$OVERLAY" 0.25

"$BIN" -o "$OUT_DIR/generator_am.wav" -f generator am 0.8 440 40 0.5 2000
"$BIN" -o "$OUT_DIR/generator_fm.wav" -f generator fm 0.8 440 40 20 2000

printf 'Used input WAV: %s\n' "$INPUT"
printf 'Generated WAV files in: %s\n' "$OUT_DIR"
