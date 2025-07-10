#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <raylib.h>

// constantes do jogo
#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 700
#define BLOCK_SIZE 20
#define UI_HEIGHT 100
#define MAX_BOMBS 10
#define MAX_ENEMIES 10
#define BOMB_TIMER 3
#define EXPLOSION_RANGE 3

// tipos de blocos
typedef enum {
    EMPTY = ' ',
    WALL = '#',
    BREAKABLE_WALL = '=',
    BOX = 'C',
    KEY = 'K',
    BOMB = 'B',
    EXPLOSION = '*'
} BlockType;

// estruturas básicas
typedef struct {
    int x;
    int y;
} Position;

typedef struct {
    Position pos;
    double start_time;
    double duration;  // duração da explosão (segundos)
} Explosion;

typedef struct {
    int last_dx;
    int last_dy;
    Position pos;
    int lives;
    int bombs;
    int score;
    int keys_collected;
    bool is_alive;
} Player; //valores que o player tem

typedef struct {
    Position pos;
    int direction;
    double last_move_time;
    bool is_alive;
} Enemy; //variáveis dos inimigos que uso pra saber sobre eles

typedef struct {
    Position pos;
    double planted_time;
    bool exploded;
} Bomb; 

typedef struct {
    char **grid;              // matriz do mapa
    int rows;
    int cols;

    Player player;

    Enemy *enemies;
    int enemy_count;

    Bomb *bombs;
    int bomb_count;

    int keys_remaining;
    int current_level;

    bool in_menu;
    bool game_over;
    bool level_complete;

    Texture2D playerTexture;
    Texture2D enemyTexture;
    Texture2D wallTexture;
    Texture2D breakableWallTexture;
    Texture2D boxTexture;
    Texture2D keyTexture;
    Texture2D bombTexture;
    Texture2D explosionTexture;

    Explosion *explosions;
    int explosion_count;
} GameState;


// protótipos das funções do jogo

void init_game(GameState *game);

void load_level(GameState *game, const char *filename);
void unload_level(GameState *game);

void draw_game(const GameState *game);
void draw_menu(GameState *game);

void handle_input(GameState *game);
void update_game(GameState *game);

void move_player(GameState *game, int dx, int dy);
void plant_bomb(GameState *game);
void update_enemies(GameState *game);
void update_bombs(GameState *game);
void update_explosions(GameState *game);

void explode_bomb(GameState *game, int bomb_index);
void add_explosion(GameState *game, int x, int y);

void check_collisions(GameState *game);

void save_game(const GameState *game);
void load_game(GameState *game);

bool is_valid_position(const GameState *game, int x, int y);
bool is_obstacle(const GameState *game, int x, int y);
bool is_position_occupied_by_bomb(const GameState *game, int x, int y);

void reset_level(GameState *game);
void next_level(GameState *game);

#endif
