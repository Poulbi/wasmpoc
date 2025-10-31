#!/bin/sh

ThisDir="$(dirname "$(readlink -f "$0")")"
cd "$ThisDir"
cd ..
  
setsid 4ed . &
tmux new-window './build/ws'
tmux split-pane -h 'zsh'
tmux select-pane -L
tmux split-pane './misc/deploy.sh'
$BROWSER http://localhost:8000
