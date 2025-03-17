#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 200
#define FRAME_DELAY 160 // ~60 FPS (1000ms / 60 = 16.67ms per frame)
#define JUMP_DURATION 500 // Jump duration in milliseconds (4 seconds for slow motion)
#define JUMP_HEIGHT 50 // Total jump height (from y=150 to y=100)
#define JUMP_SPEED 1 // Pixels to move per frame (slower for slow motion)
#define CACTUS_SPEED 0.001


bool checkCollision(SDL_Rect a, SDL_Rect b) {
    bool xOverlap = (a.x < b.x + b.w) && (a.x + a.w > b.x);
    bool yOverlap = (a.y < b.y + b.h) && (a.y + a.h > b.y);
    return xOverlap && yOverlap;
}


// Function to load a texture from an image file
SDL_Texture* loadTexture(const char* filePath, SDL_Renderer* renderer) {
    SDL_Texture* texture = NULL;

    // Load the image into a surface
    SDL_Surface* surface = IMG_Load(filePath);
    if (!surface) {
        printf("Unable to load image %s! SDL_image Error: %s\n", filePath, IMG_GetError());
        return NULL;
    }

    // Convert the surface into a texture
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface); // Free the surface as it's no longer needed
    if (!texture) {
        printf("Unable to create texture from %s! SDL Error: %s\n", filePath, SDL_GetError());
    }

    return texture;
}

int main() {

    if (TTF_Init() == -1) {
    printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
    return 1;
    }

    TTF_Font* font = TTF_OpenFont("assets/font.ttf", 24);
    if (!font) {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        return 1;
    }
    SDL_Color textColor = {0, 0, 0, 255}; // Black color

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return 1;
    }  

    // Initialize SDL_image
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }


    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return 1;
    }

    // Create a window
    SDL_Window* window = SDL_CreateWindow(
        "Dino Game", 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        SCREEN_WIDTH, 
        SCREEN_HEIGHT, 
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create a renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Load character textures
    const char* imagePath1 = "assets/main-character1.png";
    const char* imagePath2 = "assets/main-character2.png";
    const char* imagePath3 = "assets/main-character3.png";
    const char* imagePath4 = "assets/main-character4.png";
    const char* imagePath5 = "assets/main-character5.png";
    const char* imagePath6 = "assets/main-character6.png";
    const char* imagePath7 = "assets/cactus1.png";
    const char* imagePath8 = "assets/gameover_text.png";
    const char* imagePath9 = "assets/replay_button.png";
    const char* imagePath10 = "assets/cloud.png";
    SDL_Texture* texture1 = loadTexture(imagePath1, renderer);
    SDL_Texture* texture2 = loadTexture(imagePath2, renderer);
    SDL_Texture* texture3 = loadTexture(imagePath3, renderer);
    SDL_Texture* texture4 = loadTexture(imagePath4, renderer);
    SDL_Texture* texture5 = loadTexture(imagePath5, renderer);
    SDL_Texture* texture6 = loadTexture(imagePath6, renderer);
    SDL_Texture* texture7 = loadTexture(imagePath7, renderer);
    SDL_Texture* texture8 = loadTexture(imagePath8, renderer);
    SDL_Texture* texture9 = loadTexture(imagePath9, renderer);
    SDL_Texture* texture10 = loadTexture(imagePath10, renderer);

    if (!texture1 || !texture2 || !texture3 || !texture5 || !texture6) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    Mix_Chunk* scoreUpSound = Mix_LoadWAV("assets/scoreup.wav");
    Mix_Chunk* deadSound = Mix_LoadWAV("assets/dead.wav");
    if (!scoreUpSound && !deadSound) {
        printf("Failed to load score up and (or) dead sound! SDL_mixer Error: %s\n", Mix_GetError());
    }

    // Define the destination rectangle for the character
    SDL_Rect destRect;
    destRect.x = 0; // X position of the character
    destRect.y = 150; // Y position of the character (normal state)
    destRect.w = 50;  // Width of the character
    destRect.h = 50;  // Height of the character (normal state)

    SDL_Rect cactus_Rect;
    cactus_Rect.x = 580;
    cactus_Rect.y = 180;
    cactus_Rect.w = 20;
    cactus_Rect.h = 20;

    SDL_Rect big_cactus_Rect;
    big_cactus_Rect.x = 560;
    big_cactus_Rect.y = 160;
    big_cactus_Rect.w = 40;
    big_cactus_Rect.h = 40;


    SDL_Rect gameover_Rect;
    gameover_Rect.x = 150;
    gameover_Rect.y = 80;
    gameover_Rect.w = 300;
    gameover_Rect.h = 30;

    SDL_Rect replay_button_Rect;
    replay_button_Rect.x = 270;
    replay_button_Rect.y = 130;
    replay_button_Rect.h = 30;
    replay_button_Rect.w = 50;

    SDL_Rect cloud_Rect; 
    cloud_Rect.x = 300;
    cloud_Rect.y = 10;
    cloud_Rect.w = 50;
    cloud_Rect.h = 50;

    SDL_Rect cloud_Rect2;  // Second cloud
    cloud_Rect2.x = 400;   // Different starting position
    cloud_Rect2.y = 30;    // Different height
    cloud_Rect2.w = 50;
    cloud_Rect2.h = 50;

    // Add with other cloud variables
    SDL_Rect cloud_Rect3;  // Third cloud
    cloud_Rect3.x = 200;   // Different starting position
    cloud_Rect3.y = 50;    // Different height
    cloud_Rect3.w = 50;
    cloud_Rect3.h = 50;

    int cloud_frame_counter3 = 0;
    int cloud_move_frequency3 = 150;  // Different speed
    bool cloud_moving_left3 = true;

    // Main loop variables
    bool running = true;
    bool isBending = false; // Track if the player is bending
    bool isJumping = false; // Track if the player is jumping
    int frame = 0; // Frame counter
    Uint32 lastFrameTime = SDL_GetTicks(); // Time of the last frame
    Uint32 jumpStartTime = 0; // Time when the jump started
    bool isAscending = true; // Track if the player is moving upwards
    SDL_Event event;
    bool gameOver = false; // Add a game over flag
    int score = 0;
    bool isBigCactus = false; 
    float cactus_speed = 0.001;
    int cloud_frame_counter = 0;
    int cloud_move_frequency = 100;
    bool cloud_moving_left = true;  // Direction flag
    int cloud_frame_counter2 = 0;
    int cloud_move_frequency2 = 120;  // Different speed
    bool cloud_moving_left2 = true;

    while (running) {
    while (SDL_PollEvent(&event)) {
        switch (score)
        {
        case  20 ... 29:
            cactus_speed = 0.002;
            break;
        case  30 ... 49:
            cactus_speed = 0.01;
            break;
        case  50 ... 59:
            cactus_speed = 0.02;
            break;
        case  60 ... 69:
            cactus_speed = 0.03;
            break;
        case  70 ... 79:
            cactus_speed = 0.07;
            break;
        case  80 ... 89:
            cactus_speed = 0.1;
            break;
        default:
            break;
        }
        
        
        if (event.type == SDL_QUIT) {  
            running = false;
        }
        
        if ((event.key.keysym.sym == SDLK_SPACE || event.key.keysym.sym == SDLK_UP) && gameOver) {
            if (isBigCactus)
            {
                big_cactus_Rect.x = SCREEN_WIDTH;
            } else
            {
                cactus_Rect.x = SCREEN_WIDTH;
            }
            destRect.x = 0;
            destRect.y = 150;
            gameOver = false;
            score = 0;
        }

        if (!gameOver) { // Allow inputs only if game is not over
            if (event.type == SDL_KEYDOWN) {
                if ((event.key.keysym.sym == SDLK_s || event.key.keysym.sym == SDLK_DOWN) && !isJumping) {
                    isBending = true;
                    destRect.h = 30;
                    destRect.y = 170;
                }
                if ((event.key.keysym.sym == SDLK_SPACE || event.key.keysym.sym == SDLK_UP) && !isJumping) {
                    isJumping = true;
                    jumpStartTime = SDL_GetTicks();
                    isAscending = true;
                }
            }

            if (event.type == SDL_KEYUP) {
                if (event.key.keysym.sym == SDLK_s || event.key.keysym.sym == SDLK_DOWN) {
                    isBending = false;
                    destRect.h = 50;
                    destRect.y = 150;
                }
            }
        }
    }

    if (!gameOver) { // Stop updating game logic after collision
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastFrameTime >= FRAME_DELAY) {
            frame = (frame + 1) % 2;
            lastFrameTime = currentTime;
        }

        if (isJumping && !gameOver) {
            Uint32 jumpElapsedTime = currentTime - jumpStartTime;

            if (jumpElapsedTime < JUMP_DURATION / 2) {
                if (isBigCactus)
                {
                    if (checkCollision(big_cactus_Rect, destRect))
                    {
                        gameOver = true;
                    }
                    else {
                        if (destRect.y != 100) {
                        destRect.y -= JUMP_SPEED;
                        }
                    }
                    
                }
                else {
                    if (checkCollision(cactus_Rect, destRect))
                    {
                        gameOver = true;
                    }
                    else {
                        if (destRect.y != 100) {
                        destRect.y -= JUMP_SPEED;
                        }
                    }
                }
            } else if (jumpElapsedTime < JUMP_DURATION) {
                if (isBigCactus)
                {
                    if (checkCollision(big_cactus_Rect, destRect))
                    {
                        gameOver = true;
                    }
                    else {
                         if (destRect.y != 150) {
                            destRect.y += JUMP_SPEED;
                        }
                    }
                }
                else {
                    if (checkCollision(cactus_Rect, destRect))
                    {
                        gameOver = true;
                    }
                    else {
                        if (destRect.y != 150) {
                            destRect.y += JUMP_SPEED;
                        }
                    }
                }
               
            } else {
                isJumping = false;
            }
        }

        if ((cactus_Rect.x <= 0 || big_cactus_Rect.x <=0)) { // Reset cactus position
            cactus_Rect.x = SCREEN_WIDTH;
            big_cactus_Rect.x = SCREEN_WIDTH;
            score += 1;
            // Play the sound effect
            Mix_PlayChannel(-1, scoreUpSound, 0);

            // randomly choose cactus type
            isBigCactus = (rand() % 2) == 0; // 50% chance for each type 
            
        }

        if (isBigCactus)
        {
            if (checkCollision(big_cactus_Rect, destRect)) {
                gameOver = true; // Stop game updates
                Mix_PlayChannel(-1, deadSound, 0);
            }
        } else {
            if (checkCollision(cactus_Rect, destRect)) {
                gameOver = true; // Stop game updates
                Mix_PlayChannel(-1, deadSound, 0);
        }
        }
        
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    SDL_Texture* currentTexture;
    if (isJumping) {
        currentTexture = texture3;
    } else if (isBending) {
        currentTexture = (frame == 0) ? texture5 : texture6;
    } else {
        currentTexture = (frame == 0) ? texture1 : texture2;
    }

    SDL_RenderCopy(renderer, currentTexture, NULL, &destRect);
    if (isBigCactus && !gameOver)
    {
        SDL_RenderCopy(renderer, texture7, NULL, &big_cactus_Rect);
        big_cactus_Rect.x -= CACTUS_SPEED;
    } else if(!isBigCactus && !gameOver){
        SDL_RenderCopy(renderer, texture7, NULL, &cactus_Rect);
        cactus_Rect.x -= CACTUS_SPEED;
    }
    

    char scoreText[20];
    sprintf(scoreText, "Score: %d", score);
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, scoreText, textColor);
    if (!textSurface) {
        printf("Failed to render text! SDL_ttf Error: %s\n", TTF_GetError());
    } else {
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect textRect = {SCREEN_WIDTH - textSurface->w - 10, 10, textSurface->w, textSurface->h};

    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
    }

    
    cloud_frame_counter++;
    if (cloud_frame_counter >= cloud_move_frequency) {
        if (cloud_moving_left) {
            cloud_Rect.x -= 1;
            if (cloud_Rect.x <= 200) {
                cloud_moving_left = false;
            }
        } else {
            cloud_Rect.x += 1;
            if (cloud_Rect.x >= 300) {
                cloud_moving_left = true;
            }
        }
        cloud_frame_counter = 0;
    }

    cloud_frame_counter2++;
    if (cloud_frame_counter2 >= cloud_move_frequency2) {
        if (cloud_moving_left2) {
            cloud_Rect2.x -= 1;
            if (cloud_Rect2.x <= 350) {
                cloud_moving_left2 = false;
            }
        } else {
            cloud_Rect2.x += 1;
            if (cloud_Rect2.x >= 450) {
                cloud_moving_left2 = true;
            }
        }
        cloud_frame_counter2 = 0;
    }

    // Third cloud movement
    cloud_frame_counter3++;
    if (cloud_frame_counter3 >= cloud_move_frequency3) {
        if (cloud_moving_left3) {
            cloud_Rect3.x -= 1;
            if (cloud_Rect3.x <= 150) {
                cloud_moving_left3 = false;
            }
        } else {
            cloud_Rect3.x += 1;
            if (cloud_Rect3.x >= 250) {
                cloud_moving_left3 = true;
            }
        }
        cloud_frame_counter3 = 0;
    }

    // Render all clouds
    SDL_RenderCopy(renderer, texture10, NULL, &cloud_Rect);
    SDL_RenderCopy(renderer, texture10, NULL, &cloud_Rect2);
    SDL_RenderCopy(renderer, texture10, NULL, &cloud_Rect3);


    if (gameOver) {
        SDL_RenderCopy(renderer, texture8, NULL, &gameover_Rect);
        SDL_RenderCopy(renderer, texture4, NULL, &destRect);
        SDL_RenderCopy(renderer, texture9, NULL, &replay_button_Rect);
        if (isBigCactus)
        {
            SDL_RenderCopy(renderer, texture7, NULL, &big_cactus_Rect);
        }
        else {
            SDL_RenderCopy(renderer, texture7, NULL, &cactus_Rect);
        }
        
    }

    SDL_RenderPresent(renderer);
    
}

Mix_FreeChunk(scoreUpSound);
Mix_FreeChunk(deadSound);
Mix_CloseAudio();
SDL_DestroyTexture(texture1);
SDL_DestroyTexture(texture2);
SDL_DestroyTexture(texture3);
SDL_DestroyTexture(texture5);
SDL_DestroyTexture(texture6);
SDL_DestroyRenderer(renderer);
SDL_DestroyWindow(window);
IMG_Quit();
SDL_Quit();

return 0;

}