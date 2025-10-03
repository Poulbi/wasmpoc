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

WasmCompilerFlags="
-nostdlib
-mbulk-memory
--target=wasm32
"
WasmLinkerFlags="
-Wl,--allow-undefined
-Wl,--no-entry
-Wl,--export-all
-Wl,--export=LogMessage
-Wl,-z,stack-size=$((64 * 1024))
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
 -O3
 "
 WasmCompilerFlags="$WasmCompilerFlags
 -flto
 "
 WasmLinkerFlags="$WasmLinkerFlags
 -Wl,--lto-O3
 "
fi

printf 'game.c\n'
clang \
    $CompilerFlags \
    $WasmCompilerFlags $WasmLinkerFlags \
    $WarningFlags \
    -o ../build/game.wasm \
    game.c
printf 'index.html platform.js favicon.ico\n'
ln -f index.html platform.js ../build
cp ../data/favicon.ico ../build

if false
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
