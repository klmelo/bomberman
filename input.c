#include "game.h"
#include <stdio.h>

// Função para processar as entradas do jogador
void process_input(GameState *game) {
    if (game->in_menu) {
        if (IsKeyPressed(KEY_N)) {
            game->current_level = 1;
            game->player.lives = 3;
            game->player.score = 0;
            game->player.keys_collected = 0;
            game->player.bombs = 3;
            game->player.is_alive = true;
            game->game_over = false;

            char filename[20];
            sprintf(filename, "mapa%d.txt", game->current_level);
            load_level(game, filename);
            game->in_menu = false;
        }
        else if (IsKeyPressed(KEY_C)) {
            load_game(game);
            game->in_menu = false;
        }
        else if (IsKeyPressed(KEY_S)) {
            save_game(game);
            game->in_menu = false;
        }
        else if (IsKeyPressed(KEY_Q)) {
            game->game_over = true;
        }
        else if (IsKeyPressed(KEY_V)) {
            game->in_menu = false;
        }
    } else {
        if (IsKeyPressed(KEY_TAB)) {
            game->in_menu = true;
        }

        if (!game->game_over && !game->level_complete && game->player.is_alive) {
            if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) move_player(game, 1, 0);
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) move_player(game, -1, 0);
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) move_player(game, 0, -1);
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) move_player(game, 0, 1);
            if (IsKeyPressed(KEY_B)) plant_bomb(game);
        } else if (game->game_over && IsKeyPressed(KEY_N)) {
            init_game(game);
            game->in_menu = true;
        }
    }
}
