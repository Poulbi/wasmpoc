#!/bin/sh

tmux new-window './build/ws'
tmux split-pane -h 'zsh'
tmux select-pane -L
tmux split-pane './misc/deploy.sh'
