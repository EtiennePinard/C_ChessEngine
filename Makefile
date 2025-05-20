# Taken from https://stackoverflow.com/a/23324703
ROOT_DIR = $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

# Compiler and flags
CC = gcc

CFLAGS_COMMON = -Wall -Wextra -Werror
CFLAGS_DEBUG  = $(CFLAGS_COMMON) -g
CFLAGS_RELEASE = $(CFLAGS_COMMON) -O3

# Default to debug
CFLAGS ?= $(CFLAGS_DEBUG)

BUILD_RULES = \
	build_engineTest \
	build_perft \
	build_engine \
	build_app \
	build_visualizePST \
	build_visualizeEval

RUN_RULES = test perft engine app visualizePST visualizeEval

RULES = $(BUILD_RULES) $(RUN_RULES) build/assets help

# LDFLAGS
app_LDFLAGS = -lm -lSDL2 -lSDL2_ttf -lSDL2_image
visualizePST_LDFLAGS = -lSDL2 -lSDL2_ttf -lSDL2_image
visualizeEval_LDFLAGS = -lSDL2 -lSDL2_ttf -lSDL2_image

# Program arguments
PARG ?= test

# Asset linking for gui app
ASSET_SRC = gui_app/assets
ASSET_DST = build/assets

# Source files for each target
engineTest_SRC = \
    engine/testing/unitTest/unitTest.c \
    engine/testing/unitTest/tests/testCharBuffer.c \
    engine/testing/unitTest/tests/testFenString.c \
    engine/testing/unitTest/tests/testMath.c \
    engine/testing/unitTest/tests/testBoard.c \
    engine/testing/unitTest/tests/testTranspositionTable.c \
    engine/testing/logChessStructs.c \
    engine/src/utils/fenString.c \
    engine/src/utils/charBuffer.c \
    engine/src/state/zobristKey.c \
    engine/src/state/board.c \
    engine/src/bot/transpositionTable.c

perft_SRC = \
    engine/testing/perft/perft.c \
    engine/testing/perft/perftTranspositionTable.c \
    engine/testing/logChessStructs.c \
    engine/src/moveHandler/movePlayer.c \
    engine/src/moveHandler/moveGenerator.c \
    engine/src/utils/fenString.c \
    engine/src/utils/charBuffer.c \
    engine/src/state/board.c \
    engine/src/state/zobristKey.c \
    engine/src/magicBitBoard/magicBitBoard.c \
    engine/src/magicBitBoard/rook.c \
    engine/src/magicBitBoard/bishop.c

chessEngine_SRC = \
    engine/src/entryPoint.c \
    engine/src/UCICommandProcessing.c \
    engine/src/bot/bot.c \
    engine/src/bot/pieceSquareTable.c \
    engine/src/bot/transpositionTable.c \
    engine/src/bot/moveOrdering.c \
    engine/src/bot/repetitionTable.c \
    engine/src/moveHandler/movePlayer.c \
    engine/src/moveHandler/moveGenerator.c \
    engine/src/utils/fenString.c \
    engine/src/utils/charBuffer.c \
    engine/src/state/board.c \
    engine/src/state/zobristKey.c \
    engine/src/state/engineState.c \
    engine/src/magicBitBoard/magicBitBoard.c \
    engine/src/magicBitBoard/rook.c \
    engine/src/magicBitBoard/bishop.c \
    engine/testing/logChessStructs.c

sdlFramework_SRC = \
	gui_app/sdl_framework/appInit.c \
    gui_app/sdl_framework/appRunner.c \
    gui_app/sdl_framework/eventHandler.c \
    gui_app/sdl_framework/appCleanup.c

app_SRC = \
    engine/src/moveHandler/movePlayer.c \
    engine/src/moveHandler/moveGenerator.c \
    engine/src/utils/fenString.c \
    engine/src/utils/charBuffer.c \
    engine/src/state/board.c \
    engine/src/state/zobristKey.c \
    engine/src/magicBitBoard/magicBitBoard.c \
    engine/src/magicBitBoard/rook.c \
    engine/src/magicBitBoard/bishop.c \
    engine/src/bot/bot.c \
    engine/src/bot/pieceSquareTable.c \
    engine/src/bot/repetitionTable.c \
    engine/src/bot/transpositionTable.c \
    engine/src/bot/moveOrdering.c \
    engine/testing/logChessStructs.c \
    gui_app/chess_app/app.c \
    gui_app/chess_app/events.c \
    gui_app/chess_app/render.c \
    gui_app/chess_app/overlay.c \
	$(sdlFramework_SRC)

visualizePST_SRC = $(sdlFramework_SRC) gui_app/visualize_pst/visualizePieceSquareTable.c engine/src/bot/pieceSquareTable.c

visualizeEval_SRC = \
	gui_app/visualize_static_evaluation/visualizeStaticEvaluation.c \
	engine/src/bot/pieceSquareTable.c \
	engine/src/bot/transpositionTable.c \
	engine/src/bot/repetitionTable.c \
	engine/src/bot/bot.c \
	engine/src/moveHandler/movePlayer.c \
	engine/src/moveHandler/moveGenerator.c \
    engine/src/utils/fenString.c \
    engine/src/utils/charBuffer.c \
    engine/src/state/board.c \
    engine/src/state/zobristKey.c \
    engine/src/magicBitBoard/magicBitBoard.c \
    engine/src/magicBitBoard/rook.c \
    engine/src/magicBitBoard/bishop.c \
	engine/src/moveHandler/moveGenerator.c \
	$(sdlFramework_SRC)

# Convert .c files to .o in build/ directory
engineTest_OBJ = $(patsubst %.c,build/%.o,$(engineTest_SRC))
perft_OBJ= $(patsubst %.c,build/%.o,$(perft_SRC))
chessEngine_OBJ = $(patsubst %.c,build/%.o,$(chessEngine_SRC))
app_OBJ = $(patsubst %.c,build/%.o,$(app_SRC))
visualizePST_OBJ = $(patsubst %.c,build/%.o,$(visualizePST_SRC))
visualizeEval_OBJ = $(patsubst %.c,build/%.o,$(visualizeEval_SRC))


# We only want the build rules when calling all else its too chaotic when executing all the programs
.PHONY: all
all: $(BUILD_RULES)

# Compile .c to .o into build/
build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

define build_template
build_$(1): $$($(1)_OBJ)
	$$(CC) $$(CFLAGS) $$^ -o build/$(1) $$($(1)_LDFLAGS)
endef

$(eval $(call build_template,engineTest))
$(eval $(call build_template,perft))
$(eval $(call build_template,chessEngine))
$(eval $(call build_template,app))
$(eval $(call build_template,visualizePST))
$(eval $(call build_template,visualizeEval))

# Symlink for the chess pieces images and font assets
build/assets:
    # The -T is to avoid creating recursive symlinks
	@ln -sfT $(ROOT_DIR)/$(ASSET_SRC) $(ROOT_DIR)/$(ASSET_DST)


# Execution rules
test: build_engineTest
	@cd build && ./engineTest

perft: build_perft
	@cd build && ./perft $(PARG)

engine: build_chessEngine
	@cd build && ./chessEngine

app: build_app build/assets
	@cd build && ./app

visualizePST: build_visualizePST build/assets
	@cd build && ./visualizePST

visualizeEval: build_visualizeEval build/assets
	@cd build && ./visualizeEval


# Help rule
help:
	@echo "make test             - Build and run the engine's unit tests"
	@echo "make perft            - Build and run perft (PARG=...)"
	@echo "make engine           - Build and run chess engine (UCI)"
	@echo "make app              - Build and run SDL2 GUI app" 
	@echo "make visualizePST     - Build and run an app to visualize the piece square table"
	@echo "make visualizeEval    - Build and run an app to visualize the static evaluation"
	@echo "make all              - Executes all build rules. This is the default rule"
	@echo "make clean            - Remove build directory"
	@echo "make help             - Prints this help message"
	@echo "--------------------------------------------------------------"
	@echo "Use make perft PARG='<arguments>' to provide arguments to the perft program from make"
	@echo "Default argument is PARG=test"
	@echo "--------------------------------------------------------------"


# Clean everything
.PHONY: clean
clean:
	@rm -r build

# All build rules
.PHONY: $(RULES)
