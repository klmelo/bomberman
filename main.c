#include <raylib.h>
#include "game.h"
#include "input.h"

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 700

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Bomberman");
    SetTargetFPS(60);

    GameState game;
    init_game(&game);

    // carrega texturas
    game.playerTexture = LoadTexture("sprites/player.png");
    game.enemyTexture = LoadTexture("sprites/enemy.png");
    game.wallTexture = LoadTexture("sprites/wall.png");
    game.breakableWallTexture = LoadTexture("sprites/breakable_wall.png");
    game.boxTexture = LoadTexture("sprites/box.png");
    game.keyTexture = LoadTexture("sprites/key.png");
    game.bombTexture = LoadTexture("sprites/bomb.png");
    game.explosionTexture = LoadTexture("sprites/explosion.png");

    while (!WindowShouldClose())
    {
        process_input(&game);
        update_game(&game);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        if (game.in_menu)
        {
            draw_menu(&game);
        }
        else
        {
            draw_game(&game);
        }
        EndDrawing();
    }

    // libera recursos
    unload_level(&game);

    UnloadTexture(game.playerTexture);
    UnloadTexture(game.enemyTexture);
    UnloadTexture(game.wallTexture);
    UnloadTexture(game.breakableWallTexture);
    UnloadTexture(game.boxTexture);
    UnloadTexture(game.keyTexture);
    UnloadTexture(game.bombTexture);
    UnloadTexture(game.explosionTexture);

    CloseWindow();
    return 0;
}
