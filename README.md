# Chess Engine

A chess engine written in C with two main functionalities:  
- **Perft** routine for move generation and position testing  
- **Chess app** for playing against the engine's bot
  
This project was greatly inspired by [Sabastian Lague's chess video](https://www.youtube.com/watch?v=U4ogK0MIzqk)

## Build Instructions

### Requirements
- **CMake** (minimum version 3.x)  
- **SDL2**, **SDL2_TTF**, and **SDL2_image** (required for the chess app)

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

## TODO

### App
- **Rewind moves**: ability to go back in moves
- **Sounds** (Upgrade to SDL3 before doing audio)
- **Evaluation bar**: Eval bar from stockfish and/or the engine bot

### Bot
- **UCI**: Make the bot read commands of the uci protocol from stdin 
- **Bot comparison**: Program which puts two version of the bot against one another to see if the bot actually gets better
- **Time control**: Bot needs to take time into account
- **Quiescence Search**: Bot needs to end search on quiet positions, not just at required depth
- **Move ordering**: In the search function to speed it up
- **More endgame knowledge**: Increase the bot's performance in the endgame
- **Mate in n eval bar**: Bot can give evaluation in mate-in-n 
