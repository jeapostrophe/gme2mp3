#!/bin/sh

export PATH=/usr/local/opt/llvm/bin:$PATH

clang++ -L/usr/local/opt/llvm/lib -I/usr/local/opt/llvm/include -Wall -std=c++17 gme2mp3.cc -lstdc++ -lc++fs -lgme -lmp3lame -o gme2mp3 || exit 1

G=$(pwd)

cd ~/Downloads/Video-Game-Music
find . -name '*.7z' -exec 7za x '{}' \;
$G/gme2mp3

# find . -name '*.mp3' -exec rm '{}' \;
# cd NES; $G/gme2mp3; cd ..
# cd SNES; $G/gme2mp3; cd ..
# find . -name '*.mp3' -print0 | xargs -0 id3info
