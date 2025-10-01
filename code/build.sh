#!/bin/sh

ThisDir="$(dirname "$(readlink -f "$0")")"
cd "$ThisDir"

mkdir -p ../build

Mode="$1"
if [ "$Mode" != "release" ]
then
 Mode="debug" 
fi
>&2 printf 'Mode: %s\n' "$Mode"

CompilerFlags="
-fdiagnostics-absolute-paths
--target=wasm32
-nostdlib
"
  
WarningFlags="
-Wall
-Wextra
-Wno-unused-variable
-Wno-unused-but-set-variable
-Wno-write-strings
-Wno-pointer-arith
-Wno-unused-parameter
-Wno-unused-function
"

LinkerFlags="
-Wl,--allow-undefined
-Wl,--no-entry
-Wl,--export-all
-Wl,--export=LogMessage
"

if [ "$Mode" = "debug" ]
then
 CompilerFlags="$CompilerFlags
 -O0
 -g -g3 -ggdb
 "
elif [ "$Mode" = "release" ]
then
 CompilerFlags="$CompilerFlags
 -O2
 "
fi

printf 'game.c\n'
clang \
    $CompilerFlags \
    $WarningFlags \
    $LinkerFlags \
    -o ../build/game.wasm \
    game.c
printf 'index.html platform.js\n'
cp index.html platform.js ../build
