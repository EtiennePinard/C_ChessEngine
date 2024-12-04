# Chess Engine

A chess engine written in C with two main functionalities:  
- **Perft** routine for move generation and position testing  
- **Chess playing app** featuring an interactive GUI built with SDL2

This project was greatly inspired by [Sabastian Lague's chess video](https://www.youtube.com/watch?v=U4ogK0MIzqk)

## Build Instructions

### Requirements
- **CMake** (minimum version 3.x)  
- **SDL2**, **SDL2_TTF**, and **SDL2_image** (required for the chess application)

### How to Build
You can run the helper build scripts in the repo's root directory. 

- **To run the perft functionality**:
```bash
./perft 
```  

- **To play chess against the bot**
```bash
./app
``` 