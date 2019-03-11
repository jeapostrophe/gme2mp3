#!/bin/sh

export PATH=/usr/local/opt/llvm/bin:$PATH

clang++ -L/usr/local/opt/llvm/lib -I/usr/local/opt/llvm/include -Wall -std=c++17 gme2mp3.cc -lstdc++ -lc++fs -lgme -lmp3lame -o gme2mp3 || exit 1
