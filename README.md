# Chess Engine

A UCI chess engine written in C with the following functionalities:  
- **Perft** routine for move generation and position testing  
- **UCI Engine** for using this engine in other chess application 
- **Chess app** for playing against the engine's bot
  
This project was greatly inspired by [Sebastian Lague's chess video](https://www.youtube.com/watch?v=U4ogK0MIzqk)

## Build Instructions

### Requirements
- **GNU Make**  
- **POSIX** system or Windows with **Git Bash** and **MinGW**
- **SDL3**, **SDL3_TTF**, and **SDL3_image** (for the chess app)

### Chess engine and perft
To build the chess engine, you can use the `make perft` and `make engine` command 
to compile and run the perft routine and the UCI engine. 

### SDL Chess application
To compile and run the chess engine you will likely need to use the
.env file to set the include path, library path and binary path for the
SDL, SDL_ttf and SDL_image library. The .env includes these environments 
variables. 

```bash
# Environment variable for compiling the app with SDL3
SDL_INCLUDE_PATH=
SDL_LIBRARY_PATH=
SDL_BIN_PATH=

SDL_TTF_INCLUDE_PATH=
SDL_TTF_LIBRARY_PATH=
SDL_TTF_BIN_PATH=

SDL_IMAGE_INCLUDE_PATH=
SDL_IMAGE_LIBRARY_PATH=
SDL_IMAGE_BIN_PATH=
```

Note that the BIN_PATH will only be used on Windows where dynamic 
linking can be quite annoying if the library is not in the same folder
as the executable. This means that if you are on Windows the Makefile 
will copy the SDL3.dll, SDL3_ttf.dll, and SDL3_image.dll files to the 
build directory. 

You can then run `make app` to compile and run the app.