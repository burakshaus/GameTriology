Terminal Jump Game
A small terminal-based runner/jumper game written in C using pthread, ncurses, and termios for input handling.

How to Play
The player character (a stickman) stays in place while obstacles (stars *) move toward them.

Press Space to jump over incoming obstacles.

Press ESC to exit the game.

If you collide with a star (*), the game will show a collision animation and end.

Compilation Instructions
Make sure you have gcc, pthread, and ncurses installed.

Linux/macOS: gcc -o game game.c -pthread -lncurses

Note:

For Windows, you'd normally include #include <windows.h> instead of <unistd.h>, but this project is mainly for Linux/macOS terminals.
Run the Game
./game

Features
Multithreaded input handling
(separate thread reads keypresses without blocking the game)

Terminal Raw Mode for real-time key detection.

Simple collision detection between player and obstacles.

Score-based difficulty: Game speeds up over time as your score increases.

File Structure

File | Description
game.c | Main game file with all logic inside

Libraries Used
pthread: for threading

ncurses: for better control over terminal printing

termios: for raw mode input without waiting for Enter

Notes
Screen clearing uses system("clear") — could be optimized later for better performance.

Only one star appears at a time currently; extension possible for multiple obstacles.

human and star structures allow for easy extension (e.g., adding more enemies, better animations).




