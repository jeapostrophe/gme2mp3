#!/bin/sh

export PATH=/usr/local/opt/llvm/bin:$PATH

clang++ -L/usr/local/opt/llvm/lib -I/usr/local/opt/llvm/include -Wall -std=c++17 gme2mp3.cc -lstdc++ -lc++fs -lgme -lmp3lame -o gme2mp3 || exit 1

G=$(pwd)

cd ~/Downloads/Video-Game-Music
find . -name '*.7z' -exec 7za x '{}' \;
$G/gme2mp3
