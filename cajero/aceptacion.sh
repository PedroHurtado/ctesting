#!/bin/bash
# Compila y ejecuta los tests de aceptación (Linux / WSL).
# Uso: ./aceptacion.sh [opciones de cucumber]    p. ej. ./aceptacion.sh --format progress
#
# PREFIJO: donde están instalados cucumber-cpp y sus dependencias (bloque 24, §3).
set -e
PREFIJO="${PREFIJO:-$HOME/cuke/env}"
export PATH="$PREFIJO/bin:$PREFIJO/share/rubygems/bin:$PATH"
export GEM_HOME="$PREFIJO/share/rubygems"          # sin esto, Ruby no encuentra la gema cucumber
[ -x "$PREFIJO/bin/x86_64-conda-linux-gnu-g++" ] && export CXX="$PREFIJO/bin/x86_64-conda-linux-gnu-g++"

cd "$(dirname "$0")"
cmake -S . -B build -DCMAKE_PREFIX_PATH="$PREFIJO" -DCucumberCpp_DIR="$PREFIJO/lib/cmake" > /dev/null
cmake --build build

build/cajero_steps > /dev/null &                   # 1. el servidor de pasos, en segundo plano
SERVIDOR=$!
trap 'kill $SERVIDOR 2>/dev/null || true' EXIT     # si cucumber falla, que no se quede colgado
sleep 1
ruby "$GEM_HOME/bin/cucumber" --publish-quiet "$@" # 2. Cucumber
