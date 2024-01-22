# ON WINDOWS: Commands to run to get the .dll
# Note: Use MinGW to get the gcc executable
# cd ./src
# gcc -c fenString.c moveGenerator.c chessComputer.c chessGameEmulator.c chessComputer.c
# gcc -shared -o chess_engine.dll fenString.o moveGenerator.o chessComputer.o chessGameEmulator.o chessComputer.o

# ON LINUX: Use CMake to create the .so files
# cd ./engine_src/
# To clean cmake files: cmake --build ./build --config Debug --target clean --
# To build the library: cmake --build ./build --config Debug --target all --

cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(chess_engine VERSION 1.0.0 LANGUAGES C)
add_library(chess_engine SHARED 
    src/moveGenerator.c
    src/chessGameEmulator.c
    src/chessBot.c

    src/state/board.c 
    src/state/gameState.c 
    src/state/move.c 
    src/state/piece.c 
    
    src/magicBitBoard/magicBitBoard.c 
    src/magicBitBoard/rook.c 
    src/magicBitBoard/bishop.c 
    
    src/utils/fenString.c
    src/utils/utils.c 
)

set_target_properties(chess_engine PROPERTIES
    PUBLIC_HEADER moveGenerator.h
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    LIBRARY_OUTPUT_NAME "chess_engine"
    PREFIX ""
    SUFFIX ".so"
)
