Dino Game (OpenGL + GLFW + FreeType)
A simple clone of the Chrome Dino game built using OpenGL, GLFW, and FreeType.
Features running, jumping, crouching, obstacles (cacti and birds), clouds, and collision detection!

How to Compile
You must have GLFW, FreeType, and OpenGL installed on your system.
(Instructions below assume you're on macOS with Homebrew.)

gcc -I/opt/homebrew/include/freetype2 -L/opt/homebrew/lib -lfreetype \
    $(pkg-config --cflags glfw3 freetype2) dinogame_GL.c -o dinogame \
    $(pkg-config --libs glfw3 freetype2) -framework OpenGL -framework Cocoa -framework IOKit

Requirements
GLFW3 (for window creation and input)

OpenGL (for rendering)

FreeType2 (for font rendering, if you plan to add text)

stb_image.h (included for loading images)

Install dependencies with Homebrew:
brew install glfw freetype

Controls

Key	Action
Space	Jump
↓ (Down Arrow)	Crouch (implementation not shown yet)
(Note: Crouching key might need to be completed — the provided code prepares crouch textures.)

Features
Running Dinosaur with smooth jumping and crouching animation

Obstacle Generation:

Cacti (small and big)

Birds (at different heights)

Clouds move independently at different speeds

Collision Detection:

Game ends on hitting an obstacle

Game Over screen (prepared in code)

Project Structure (currently)
dinogame_GL.c — Main source file

stb_image.h — Header for loading textures

Textures — Images for dinosaur, cacti, birds, clouds, replay button, and game over screen (assumed)

Notes
Textures (character, obstacles, etc.) must be placed correctly in your project folder.

The current version prints collision hitboxes and debug info to the terminal.

Gravity and jump physics are fine-tuned for a smoother experience.

The game is currently optimized for 600x200 window size.
