#!/bin/sh
ThisDir="$(dirname "$(readlink -f "$0")")"
cd "$ThisDir"
  
cd ../build
python -m http.server
