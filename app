#!/bin/bash

# Run with ./app

mkdir -p /home/ejrp/Coding/OtherRandomProjects/C_ChessEngine/build
cd /home/ejrp/Coding/OtherRandomProjects/C_ChessEngine/build

OUT="$(mktemp /tmp/output.XXXXXXXXXX)" || { echo "Failed to create temp file"; exit 1; }

# -fsanitize=address
cmake -DWHICH=app -DCMAKE_BUILD_TYPE=Debug .. >> "$OUT"
cmake --build . >> "$OUT"

if [ $? -ne 0 ]; then
    cat "$OUT"
    printf "\n================== ERROR while compiling with CMake ==================\n"
    exit 1
fi

./chess_app