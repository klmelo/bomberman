#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <raylib.h>



// inicializa o estado do jogo
void init_game(GameState *game) {
    game->player.last_dx = 0;
    game->player.last_dy = 0;
    game->grid = NULL;
    game->rows = 0;
    game->cols = 0;
    game->enemies = NULL;
    game->enemy_count = 0;
    game->bombs = NULL;
    game->bomb_count = 0;
    game->current_level = 1;
    game->in_menu = true;
    game->game_over = false;
    game->level_complete = false;
    game->explosions = NULL;
    game->explosion_count = 0;

    game->player.lives = 3;
    game->player.bombs = 3;
    game->player.score = 0;
    game->player.keys_collected = 0;
    game->player.is_alive = true;
}


// atualiza explosões (remove as que já passaram da duração)
void update_explosions(GameState *game) {
    double now = GetTime();
    int write_index = 0;

    for (int i = 0; i < game->explosion_count; i++) {
        double elapsed = now - game->explosions[i].start_time;
        if (elapsed < game->explosions[i].duration) {
            if (write_index != i) {
                game->explosions[write_index] = game->explosions[i];
            }
            write_index++;
        }
    }

    game->explosion_count = write_index;
    game->explosions = (Explosion *)realloc(game->explosions, game->explosion_count * sizeof(Explosion));
}


// carrega o mapa pelo arquivo de texto
void load_level(GameState *game, const char *filename) {
    unload_level(game);

    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Erro ao abrir o arquivo %s\n", filename);
        exit(1);
    }

    char line[256];
    game->rows = 0;
    game->cols = 0;
    game->enemy_count = 0;

    // Conta linhas, colunas máximas e inimigos
    while (fgets(line, sizeof(line), file)) {
        int len = strlen(line);
        if (line[len - 1] == '\n') len--;
        game->rows++;
        if (len > game->cols) game->cols = len;
        for (int j = 0; j < len; j++) {
            if (line[j] == 'E') game->enemy_count++;
        }
    }

    game->grid = (char **)malloc(game->rows * sizeof(char *));
    for (int i = 0; i < game->rows; i++) {
        game->grid[i] = (char *)malloc(game->cols * sizeof(char));
        memset(game->grid[i], EMPTY, game->cols * sizeof(char));
    }

    game->enemies = (Enemy *)malloc(game->enemy_count * sizeof(Enemy));
    int enemy_index = 0;
    game->keys_remaining = 0;

    rewind(file);

    // preenche o grid, posição do jogador e inimigos
    for (int i = 0; i < game->rows; i++) {
        if (!fgets(line, sizeof(line), file)) break;
        int len = strlen(line);
        if (line[len - 1] == '\n') line[len - 1] = '\0';
        len--;

        for (int j = 0; j < game->cols; j++) {
            if (j < len) {
                char c = line[j];
                if (c == 'P') {
                    game->player.pos.x = j;
                    game->player.pos.y = i;
                    game->grid[i][j] = EMPTY;
                }
                else if (c == 'E') {
                    game->enemies[enemy_index].pos.x = j;
                    game->enemies[enemy_index].pos.y = i;
                    game->enemies[enemy_index].direction = GetRandomValue(0, 3);
                    game->enemies[enemy_index].last_move_time = GetTime();
                    game->enemies[enemy_index].is_alive = true;
                    enemy_index++;
                    game->grid[i][j] = EMPTY;
                }
                else {
                    game->grid[i][j] = c;
                    if (c == KEY) game->keys_remaining++;
                }
            } else {
                game->grid[i][j] = EMPTY;
            }
        }
    }

    fclose(file);

    game->player.keys_collected = 0;
    game->player.bombs = 3;
    game->level_complete = false;
}


// libera memória do nível atual
void unload_level(GameState *game) {
    if (game->grid) {
        for (int i = 0; i < game->rows; i++) {
            free(game->grid[i]);
        }
        free(game->grid);
        game->grid = NULL;
    }

    if (game->enemies) {
        free(game->enemies);
        game->enemies = NULL;
    }

    if (game->bombs) {
        free(game->bombs);
        game->bombs = NULL;
    }

    if (game->explosions) {
        free(game->explosions);
        game->explosions = NULL;
    }

    game->bomb_count = 0;
    game->explosion_count = 0;
}


// desenha o jogo na tela
void draw_game(const GameState *game) {
    for (int i = 0; i < game->rows; i++) {
        for (int j = 0; j < game->cols; j++) {
            switch (game->grid[i][j]) {
                case WALL:
                    DrawTexture(game->wallTexture, j * BLOCK_SIZE, i * BLOCK_SIZE, WHITE);
                    break;
                case BREAKABLE_WALL:
                    DrawTexture(game->breakableWallTexture, j * BLOCK_SIZE, i * BLOCK_SIZE, WHITE);
                    break;
                case BOX:
                    DrawTexture(game->boxTexture, j * BLOCK_SIZE, i * BLOCK_SIZE, WHITE);
                    break;
                case KEY:
                    DrawTexture(game->keyTexture, j * BLOCK_SIZE, i * BLOCK_SIZE, WHITE);
                    break;
                case BOMB:
                    DrawTexture(game->bombTexture, j * BLOCK_SIZE, i * BLOCK_SIZE, WHITE);
                    break;
                case EXPLOSION:
                    DrawRectangle(j * BLOCK_SIZE, i * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, RED);
                    break;
                default:
                    break;
            }
        }
    }

    if (game->player.is_alive) {
        DrawTexture(game->playerTexture, game->player.pos.x * BLOCK_SIZE, game->player.pos.y * BLOCK_SIZE, WHITE);
    }

    for (int i = 0; i < game->enemy_count; i++) {
        if (game->enemies[i].is_alive) {
            DrawTexture(game->enemyTexture, game->enemies[i].pos.x * BLOCK_SIZE, game->enemies[i].pos.y * BLOCK_SIZE, WHITE);
        }
    }

    for (int i = 0; i < game->explosion_count; i++) {
        DrawTexture(game->explosionTexture, game->explosions[i].pos.x * BLOCK_SIZE, game->explosions[i].pos.y * BLOCK_SIZE, WHITE);
    }

    // desenha a HUD que fica em baixo no jogo
    Rectangle uiRect = {0, SCREEN_HEIGHT - UI_HEIGHT, SCREEN_WIDTH, UI_HEIGHT};
    DrawRectangleRec(uiRect, LIGHTGRAY);

    DrawText(TextFormat("Vidas: %d", game->player.lives), 20, SCREEN_HEIGHT - UI_HEIGHT + 20, 20, BLACK);
    DrawText(TextFormat("Bombas: %d", game->player.bombs), 20, SCREEN_HEIGHT - UI_HEIGHT + 50, 20, BLACK);
    DrawText(TextFormat("Pontos: %d", game->player.score), 200, SCREEN_HEIGHT - UI_HEIGHT + 20, 20, BLACK);
    DrawText(TextFormat("Chaves: %d/5", game->player.keys_collected), 200, SCREEN_HEIGHT - UI_HEIGHT + 50, 20, BLACK);
    DrawText(TextFormat("Nivel: %d", game->current_level), 400, SCREEN_HEIGHT - UI_HEIGHT + 20, 20, BLACK);

    if (game->game_over) {
        DrawText("GAME OVER", SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 - 20, 40, RED);
        DrawText("Pressione N para novo jogo ou Q para sair", SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT/2 + 30, 20, BLACK);
    }

    if (game->level_complete) {
        DrawText("NIVEL COMPLETO!", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 - 20, 40, GREEN);
        DrawText("Preparando proximo nivel...", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 + 30, 20, BLACK);
    }
}


// desenha o menu principal
void draw_menu(GameState *game) {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.8f));

    DrawText("BOMBERMAN", SCREEN_WIDTH/2 - 150, 100, 50, WHITE);

    DrawText("N - Novo Jogo", SCREEN_WIDTH/2 - 100, 200, 30, WHITE);
    DrawText("C - Carregar Jogo", SCREEN_WIDTH/2 - 100, 250, 30, WHITE);
    DrawText("S - Salvar Jogo", SCREEN_WIDTH/2 - 100, 300, 30, WHITE);
    DrawText("Q - Sair", SCREEN_WIDTH/2 - 100, 350, 30, WHITE);
    DrawText("V - Voltar ao Jogo", SCREEN_WIDTH/2 - 100, 400, 30, WHITE);
}


// trata as entradas do jogador
void handle_input(GameState *game) {
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


// atualiza o estado do jogo (inimigos, bombas, explosões, colisões et cetera)
void update_game(GameState *game) {
    if (game->game_over || game->in_menu || game->level_complete) return;

    update_enemies(game);
    update_bombs(game);
    update_explosions(game);
    check_collisions(game);

    if (game->player.keys_collected >= 5) {
        game->level_complete = true;
        double current_time = GetTime();
        while (GetTime() - current_time < 2.0) {
            // delay para transição de nível
        }
        next_level(game);
    }

    if (game->player.lives <= 0) {
        game->player.is_alive = false;
        game->game_over = true;
    }
}


// move o jogador (se possível)
void move_player(GameState *game, int dx, int dy) {
    int new_x = game->player.pos.x + dx;
    int new_y = game->player.pos.y + dy;

    if (is_valid_position(game, new_x, new_y)) {
        if (!is_obstacle(game, new_x, new_y) && !is_position_occupied_by_bomb(game, new_x, new_y)) {
            game->player.pos.x = new_x;
            game->player.pos.y = new_y;
            game->player.last_dx = dx;
            game->player.last_dy = dy;

            if (game->grid[new_y][new_x] == KEY) {
                game->player.keys_collected++;
                game->grid[new_y][new_x] = EMPTY;
                game->keys_remaining--;
            }
        }
    }
}


// planta bomba se o jogador tiver disponível
void plant_bomb(GameState *game) {
    if (game->player.bombs > 0 && game->bomb_count < MAX_BOMBS) {
        int bx = game->player.pos.x + game->player.last_dx;
        int by = game->player.pos.y + game->player.last_dy;

        if (is_valid_position(game, bx, by) && game->grid[by][bx] == EMPTY &&
            !is_position_occupied_by_bomb(game, bx, by)) {

            game->bombs = (Bomb *)realloc(game->bombs, (game->bomb_count + 1) * sizeof(Bomb));
            game->bombs[game->bomb_count].pos.x = bx;
            game->bombs[game->bomb_count].pos.y = by;
            game->bombs[game->bomb_count].planted_time = GetTime();
            game->bombs[game->bomb_count].exploded = false;
            game->bomb_count++;

            game->grid[by][bx] = BOMB;

            game->player.bombs--;
        }
    }
}


// atualiza movimentação dos inimigos
void update_enemies(GameState *game) {
    double current_time = GetTime();

    for (int i = 0; i < game->enemy_count; i++) {
        if (!game->enemies[i].is_alive) continue;

        if (current_time - game->enemies[i].last_move_time > 0.5) {
            int dx = 0, dy = 0;
            switch (game->enemies[i].direction) {
                case 0: dy = -1; break;
                case 1: dx = 1; break;
                case 2: dy = 1; break;
                case 3: dx = -1; break;
            }

            int new_x = game->enemies[i].pos.x + dx;
            int new_y = game->enemies[i].pos.y + dy;

            if (is_valid_position(game, new_x, new_y)) {
                if (!is_obstacle(game, new_x, new_y) && !is_position_occupied_by_bomb(game, new_x, new_y)) {
                    bool free_pos = true;
                    for (int j = 0; j < game->enemy_count; j++) {
                        if (i != j && game->enemies[j].is_alive &&
                            game->enemies[j].pos.x == new_x &&
                            game->enemies[j].pos.y == new_y) {
                            free_pos = false;
                            break;
                        }
                    }
                    if (free_pos) {
                        game->enemies[i].pos.x = new_x;
                        game->enemies[i].pos.y = new_y;
                        game->enemies[i].last_move_time = current_time;
                        continue;
                    }
                }
            }

            game->enemies[i].direction = GetRandomValue(0, 3);
            game->enemies[i].last_move_time = current_time;
        }
    }
}


// atualiza bombas, explodindo as que estão no tempo
void update_bombs(GameState *game) {
    double current_time = GetTime();

    for (int i = 0; i < game->bomb_count; i++) {
        if (!game->bombs[i].exploded && current_time - game->bombs[i].planted_time >= BOMB_TIMER) {
            explode_bomb(game, i);
        }
    }
}

// verifica se a posição (x,y) está dentro dos limites do mapa
bool is_valid_position(const GameState *game, int x, int y) {
    return (x >= 0 && x < game->cols && y >= 0 && y < game->rows);
}

// verifica se a posição tem obstáculo (parede, caixa, bomba)
bool is_obstacle(const GameState *game, int x, int y) {
    char c = game->grid[y][x];
    return (c == WALL || c == BREAKABLE_WALL || c == BOX);
}

// verifica se já existe bomba na posição (x,y)
bool is_position_occupied_by_bomb(const GameState *game, int x, int y) {
    for (int i = 0; i < game->bomb_count; i++) {
        if (game->bombs[i].pos.x == x && game->bombs[i].pos.y == y && !game->bombs[i].exploded) {
            return true;
        }
    }
    return false;
}

// checa colisões do jogador com inimigos e explosões
void check_collisions(GameState *game) {
    if (!game->player.is_alive) return;

    // jogador x inimigos
    for (int i = 0; i < game->enemy_count; i++) {
        if (game->enemies[i].is_alive &&
            game->enemies[i].pos.x == game->player.pos.x &&
            game->enemies[i].pos.y == game->player.pos.y) {
            
            game->player.lives--;
            if (game->player.lives <= 0) {
                game->player.is_alive = false;
                game->game_over = true;
            } else {
                // respawn do jogador: pode colocar na posição inicial ou no último checkpoint
                // aqui tô colocando na posição inicial do level (recarrega)
                char filename[20];
                sprintf(filename, "mapa%d.txt", game->current_level);
                load_level(game, filename);
            }
            break;
        }
    }

    // jogador x explosões
    for (int i = 0; i < game->explosion_count; i++) {
        if (game->explosions[i].pos.x == game->player.pos.x &&
            game->explosions[i].pos.y == game->player.pos.y) {
            
            game->player.lives--;
            if (game->player.lives <= 0) {
                game->player.is_alive = false;
                game->game_over = true;
            } else {
                char filename[20];
                sprintf(filename, "mapa%d.txt", game->current_level);
                load_level(game, filename);
            }
            break;
        }
    }
}

// passa para o próximo nível
void next_level(GameState *game) {
    game->current_level++;
    char filename[20];
    sprintf(filename, "mapa%d.txt", game->current_level);
    load_level(game, filename);

    game->player.keys_collected = 0;
    game->level_complete = false;
}

// salva o estado do jogo em arquivo
void save_game(const GameState *game) {
    FILE *file = fopen("savegame.dat", "wb");
    if (!file) {
        printf("Erro ao abrir arquivo para salvar jogo.\n");
        return;
    }

    fwrite(&game->player, sizeof(Player), 1, file);
    fwrite(&game->current_level, sizeof(int), 1, file);
    fwrite(&game->player.lives, sizeof(int), 1, file);
    fwrite(&game->player.score, sizeof(int), 1, file);
    fwrite(&game->player.keys_collected, sizeof(int), 1, file);
    fwrite(&game->player.bombs, sizeof(int), 1, file);

    // salva grid
    fwrite(&game->rows, sizeof(int), 1, file);
    fwrite(&game->cols, sizeof(int), 1, file);
    for (int i = 0; i < game->rows; i++) {
        fwrite(game->grid[i], sizeof(char), game->cols, file);
    }

    // salva inimigos
    fwrite(&game->enemy_count, sizeof(int), 1, file);
    fwrite(game->enemies, sizeof(Enemy), game->enemy_count, file);

    // salva bombas
    fwrite(&game->bomb_count, sizeof(int), 1, file);
    fwrite(game->bombs, sizeof(Bomb), game->bomb_count, file);

    fclose(file);
    printf("Jogo salvo com sucesso.\n");
}

// carrega o estado do jogo do arquivo salvo
void load_game(GameState *game) {
    FILE *file = fopen("savegame.dat", "rb");
    if (!file) {
        printf("Nenhum arquivo de salvamento encontrado.\n");
        return;
    }

    fread(&game->player, sizeof(Player), 1, file);
    fread(&game->current_level, sizeof(int), 1, file);
    fread(&game->player.lives, sizeof(int), 1, file);
    fread(&game->player.score, sizeof(int), 1, file);
    fread(&game->player.keys_collected, sizeof(int), 1, file);
    fread(&game->player.bombs, sizeof(int), 1, file);

    // Libera grid antiga
    if (game->grid) {
        for (int i = 0; i < game->rows; i++) {
            free(game->grid[i]);
        }
        free(game->grid);
        game->grid = NULL;
    }

    fread(&game->rows, sizeof(int), 1, file);
    fread(&game->cols, sizeof(int), 1, file);

    game->grid = malloc(game->rows * sizeof(char *));
    for (int i = 0; i < game->rows; i++) {
        game->grid[i] = malloc(game->cols * sizeof(char));
        fread(game->grid[i], sizeof(char), game->cols, file);
    }

    fread(&game->enemy_count, sizeof(int), 1, file);
    free(game->enemies);
    game->enemies = malloc(game->enemy_count * sizeof(Enemy));
    fread(game->enemies, sizeof(Enemy), game->enemy_count, file);

    fread(&game->bomb_count, sizeof(int), 1, file);
    free(game->bombs);
    game->bombs = malloc(game->bomb_count * sizeof(Bomb));
    fread(game->bombs, sizeof(Bomb), game->bomb_count, file);

    fclose(file);
    game->in_menu = false;
    game->game_over = false;
    game->level_complete = false;
    printf("Jogo carregado com sucesso.\n");
}

// função para adicionar explosão na lista e no grid
void add_explosion(GameState *game, int x, int y) {
    game->explosions = (Explosion *)realloc(game->explosions, (game->explosion_count + 1) * sizeof(Explosion));
    game->explosions[game->explosion_count].pos.x = x;
    game->explosions[game->explosion_count].pos.y = y;
    game->explosions[game->explosion_count].start_time = GetTime();
    game->explosions[game->explosion_count].duration = 0.5;
    game->explosion_count++;
}

// explode a bomba no índice bomb_index, gerando explosões nas 4 direções
void explode_bomb(GameState *game, int bomb_index) {
    Bomb *bomb = &game->bombs[bomb_index];
    if (bomb->exploded) return;
    bomb->exploded = true;

    int cx = bomb->pos.x;
    int cy = bomb->pos.y;
    int range = EXPLOSION_RANGE;

    // remove bomba do grid
    game->grid[cy][cx] = EMPTY;
    add_explosion(game, cx, cy);

    // explode para cima
    for (int y = cy - 1; y >= cy - range && y >= 0; y--) {
        if (game->grid[y][cx] == WALL) break;
        add_explosion(game, cx, y);
        if (game->grid[y][cx] == BREAKABLE_WALL || game->grid[y][cx] == BOX) {
            game->grid[y][cx] = EMPTY;
            break;
        }
    }

    // explode para baixo
    for (int y = cy + 1; y <= cy + range && y < game->rows; y++) {
        if (game->grid[y][cx] == WALL) break;
        add_explosion(game, cx, y);
        if (game->grid[y][cx] == BREAKABLE_WALL || game->grid[y][cx] == BOX) {
            game->grid[y][cx] = EMPTY;
            break;
        }
    }

    // explode para esquerda
    for (int x = cx - 1; x >= cx - range && x >= 0; x--) {
        if (game->grid[cy][x] == WALL) break;
        add_explosion(game, x, cy);
        if (game->grid[cy][x] == BREAKABLE_WALL || game->grid[cy][x] == BOX) {
            game->grid[cy][x] = EMPTY;
            break;
        }
    }

    // explode para direita
    for (int x = cx + 1; x <= cx + range && x < game->cols; x++) {
        if (game->grid[cy][x] == WALL) break;
        add_explosion(game, x, cy);
        if (game->grid[cy][x] == BREAKABLE_WALL || game->grid[cy][x] == BOX) {
            game->grid[cy][x] = EMPTY;
            break;
        }
    }

    // mata inimigos atingidos pela explosão
    for (int i = 0; i < game->enemy_count; i++) {
        if (!game->enemies[i].is_alive) continue;
        for (int j = 0; j < game->explosion_count; j++) {
            if (game->enemies[i].pos.x == game->explosions[j].pos.x &&
                game->enemies[i].pos.y == game->explosions[j].pos.y) {
                game->enemies[i].is_alive = false;
                game->player.score += 50;
            }
        }
    }

    // devolve uma bomba ao jogador
    game->player.bombs++;
}

