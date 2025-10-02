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
-Wno-null-dereference
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
    -nostdlib \
    --target=wasm32 \
    $WarningFlags \
    $LinkerFlags \
    -o ../build/game.wasm \
    game.c
printf 'index.html platform.js\n'
ln -f index.html platform.js ../build

if true
then
 cd ../ws
 printf 'ws.c\n'
 clang \
  -I./libs/wsServer/include -I./libs/wsServer/src \
  $CompilerFlags \
  $WarningFlags \
  -o ../build/ws \
  ws.c
fi

printf '%s\n' "update" | websocat 'ws://localhost:1234/'