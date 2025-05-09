# Taken from https://stackoverflow.com/a/23324703
ROOT_DIR = $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

# Compiler and flags
CC = gcc

CFLAGS_COMMON = -Wall -Wextra -Werror
CFLAGS_DEBUG  = $(CFLAGS_COMMON) -g
CFLAGS_RELEASE = $(CFLAGS_COMMON) -O3

# Default to debug
CFLAGS ?= $(CFLAGS_DEBUG)

TARGETS = build/test build/perft build/engine build/app

# LDFLAGS
app_LDFLAGS = -lm -lSDL2 -lSDL2_ttf -lSDL2_image

# Program arguments
PARG ?= test

# Asset linking for chess app
ASSET_SRC = chess_app/assets
ASSET_DST = build/assets

# Source files for each target
test_SRC = \
    testing/unitTest/unitTest.c \
    testing/unitTest/tests/testCharBuffer.c \
    testing/unitTest/tests/testFenString.c \
    testing/unitTest/tests/testMath.c \
    testing/unitTest/tests/testBoard.c \
    testing/unitTest/tests/testTranspositionTable.c \
    testing/logChessStructs.c \
    src/utils/fenString.c \
    src/utils/charBuffer.c \
    src/state/zobristKey.c \
    src/state/board.c \
    src/bot/transpositionTable.c

perft_SRC = \
    testing/perft/perft.c \
    testing/perft/perftTranspositionTable.c \
    testing/logChessStructs.c \
    src/engine/chessGameEmulator.c \
    src/engine/moveGenerator.c \
    src/utils/fenString.c \
    src/utils/charBuffer.c \
    src/state/board.c \
    src/state/zobristKey.c \
    src/magicBitBoard/magicBitBoard.c \
    src/magicBitBoard/rook.c \
    src/magicBitBoard/bishop.c

engine_SRC = \
    src/entryPoint.c \
    src/UCICommandProcessing.c \
    src/bot/bot.c \
    src/bot/pieceSquareTable.c \
    src/bot/transpositionTable.c \
    src/bot/repetitionTable.c \
    src/engine/chessGameEmulator.c \
    src/engine/moveGenerator.c \
    src/utils/fenString.c \
    src/utils/charBuffer.c \
    src/state/board.c \
    src/state/zobristKey.c \
    src/state/engineState.c \
    src/magicBitBoard/magicBitBoard.c \
    src/magicBitBoard/rook.c \
    src/magicBitBoard/bishop.c \
    testing/logChessStructs.c

app_SRC = \
    src/engine/chessGameEmulator.c \
    src/engine/moveGenerator.c \
    src/utils/fenString.c \
    src/utils/charBuffer.c \
    src/state/board.c \
    src/state/zobristKey.c \
    src/magicBitBoard/magicBitBoard.c \
    src/magicBitBoard/rook.c \
    src/magicBitBoard/bishop.c \
    src/bot/bot.c \
    src/bot/pieceSquareTable.c \
    src/bot/repetitionTable.c \
    src/bot/transpositionTable.c \
    testing/logChessStructs.c \
    chess_app/app.c \
    chess_app/appInit.c \
    chess_app/eventHandler.c \
    chess_app/events.c \
    chess_app/render.c \
    chess_app/overlay.c

# Convert .c files to .o in build/ directory
test_OBJ = $(patsubst %.c,build/%.o,$(test_SRC))
perft_OBJ= $(patsubst %.c,build/%.o,$(perft_SRC))
engine_OBJ = $(patsubst %.c,build/%.o,$(engine_SRC))
app_OBJ = $(patsubst %.c,build/%.o,$(app_SRC))

.PHONY: all
all: $(TARGETS)

# Compile .c to .o into build/
build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

define build_template
build_$(1): $$($(1)_OBJ)
	$$(CC) $$(CFLAGS) $$^ -o build/$(1) $$($(1)_LDFLAGS)
endef

$(eval $(call build_template,test))
$(eval $(call build_template,perft))
$(eval $(call build_template,engine))
$(eval $(call build_template,app))

# Symlink for the chess pieces images and font assets
build/assets:
	@ln -sf $(ROOT_DIR)/$(ASSET_SRC) $(ROOT_DIR)/$(ASSET_DST)


# Execution rules
test: build_test
	cd build && ./test

perft: build_perft
	cd build && ./perft $(PARG)

engine: build_engine
	cd build && ./engine

app: build_app build/assets
	cd build && ./app


# Help rule
help:
	@echo "make test       - Build and run unit tests"
	@echo "make perft      - Build and run perft (PARG=...)"
	@echo "make engine     - Build and run chess engine (UCI)"
	@echo "make app        - Build and run SDL2 GUI app"
	@echo "make clean      - Remove build directory"
	@echo "make help       - Prints this help message"
	@echo "--------------------------------------------------------------------------------------"
	@echo "Use make perft PARG='<arguments>' to provide arguments to the perft program from make"
	@echo "Default argument is PARG=test"
	@echo "--------------------------------------------------------------------------------------"


# Clean everything
.PHONY: clean
clean:
	@rm -r build

# All build targets
.PHONY: test perft engine app build_test build_perft build_engine build_app help
