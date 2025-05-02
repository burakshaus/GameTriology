Dino Game (SDL2 Version)
A simple Dino Runner game implemented using SDL2, inspired by the Chrome offline dinosaur game!
Requirements
SDL2

SDL2_image

SDL2_ttf

SDL2_mixer

A C compiler (e.g., gcc)

Asset files (in the /assets folder)

How to Build : gcc -o sdl main.c -I/opt/homebrew/include -D_THREAD_SAFE -L/opt/homebrew/lib -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf
Adjust include/library paths if needed depending on your platform.
How to Run : ./sdl
Controls
Space: Jump

Down Arrow: Bend

Click Replay Button: Restart after Game Over
Project Structure
/assets/
    main-character1.png
    main-character2.png
    ...
    cactus1.png
    cloud.png
    font.ttf
    scoreup.wav
    dead.wav

main.c
README.md
Features
Player character with running animation

Jumping and bending

Random obstacles (cactus, big cactus)

Moving background clouds

Score tracking

Sound effects for scoring and dying

Game Over screen with replay button

To-Do
Add dynamic cactus speed increasing with score

Add high score saving

Add more obstacle types
