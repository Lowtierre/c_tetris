/**
 * raw_terminal.h include le lib standard <stdio.h>, <stdlib.h>
 * e la logica per gestire i non-blocking input su sistemi
 * windows, linux e macos
 */

#include "./raw_terminal.h"
#include <time.h>
#include <stdint.h>

#define W 10
#define H 20
#define BOARD_SIZE (W*H)

typedef struct { int x, y; } coords;
typedef struct { int8_t dx, dy; } offset;
typedef struct {
    int x, y;
    offset b[3];
    uint8_t type;
} tetromino;

// globals
static int8_t board[BOARD_SIZE] = {0};
tetromino *piece = NULL;
int play = 1;
int score = 0;

void loop();
void polling_loop();

// random_generation
tetromino *generate_tetromino();

// movement
int manage_input();
void move_piece(tetromino *new, int key);
void left_move(tetromino *new);
void right_move(tetromino *new);
void apply_gravity(tetromino *new);
void rotate(tetromino *new, int key);

// collisions
int wall_collision(tetromino *new);
int side_collision(tetromino *new);
void freeze_piece();

// cleaning
void clean_complete_rows();

// display
void print_board();
int piece_or_not(int x, int y);

// utils
int get_idx_from_coords(int x, int y);
coords get_coords_from_idx(int i);
void copy_tetromino(tetromino *new, tetromino *old);

// core
int main() {
    srand(time(NULL));
    loop();
    return 0;
}

void loop() {
    piece = generate_tetromino();
    print_board();

    while (play) polling_loop();

    free(piece);
    clear_screen();
    printf("\nTETRIS\n\n");
    printf("Game over...\n\n");
    printf("%d deleted rows!\n\n", score);
}

// random_generation
tetromino *generate_tetromino() {
    char types[7] = {'O', 'I', 'L', 'J', 'S', 'Z', 'T'};
    char type = types[rand() % 7];

    tetromino *piece = malloc(sizeof(tetromino));
    piece->x = 4;
    piece->y = H - 1;

    switch (type) {
        case 'O':
            piece->b[0].dx = 0;
            piece->b[0].dy = 1;
            piece->b[1].dx = 1;
            piece->b[1].dy = 1;
            piece->b[2].dx = 1;
            piece->b[2].dy = 0;
            break;
        case 'I':
            piece->b[0].dx = 0;
            piece->b[0].dy = -1;
            piece->b[1].dx = 0;
            piece->b[1].dy = 1;
            piece->b[2].dx = 0;
            piece->b[2].dy = 2;
            break;
        case 'L':
            piece->b[0].dx = 0;
            piece->b[0].dy = 1;
            piece->b[1].dx = 0;
            piece->b[1].dy = 2;
            piece->b[2].dx = 1;
            piece->b[2].dy = 0;
            break;
        case 'J':
            piece->b[0].dx = 0;
            piece->b[0].dy = 1;
            piece->b[1].dx = 0;
            piece->b[1].dy = 2;
            piece->b[2].dx = -1;
            piece->b[2].dy = 0;
            break;
        case 'S':
            piece->b[0].dx = -1;
            piece->b[0].dy = 0;
            piece->b[1].dx = 0;
            piece->b[1].dy = 1;
            piece->b[2].dx = 1;
            piece->b[2].dy = 1;
            break;
        case 'Z':
            piece->b[0].dx = -1;
            piece->b[0].dy = 1;
            piece->b[1].dx = 0;
            piece->b[1].dy = 1;
            piece->b[2].dx = 1;
            piece->b[2].dy = 0;
            break;
        case 'T':
            piece->b[0].dx = -1;
            piece->b[0].dy = 0;
            piece->b[1].dx = 0;
            piece->b[1].dy = 1;
            piece->b[2].dx = 1;
            piece->b[2].dy = 0;
            break;
    }

    return piece;
}

// movement
void polling_loop() {
    tetromino *new = malloc(sizeof(tetromino));
    copy_tetromino(new, piece);
    apply_gravity(new);

    int elapsed = 0;
    int key = 0;
    while (elapsed < 1000) {
        int reprint = 0;
        // Check input during waiting...
        key = manage_input();
        if (key) { move_piece(new, key); }
        
        if (wall_collision(new)) {
            freeze_piece();
            clean_complete_rows();
            free(piece);
            piece = generate_tetromino();
            reprint = 1;
            break;
        }

        if (!side_collision(new)) {
            copy_tetromino(piece, new);
            if (key) reprint = 1;
        }

        if (reprint) print_board();
        sleep_ms(10);
        elapsed += 10;
    }
    
    print_board();
    free(new);
}

void move_piece(tetromino *new, int key) {
    if (key == 75 || key == 97) left_move(new);
    if (key == 77 || key == 100) right_move(new);
    if (key == 80 || key == 115) apply_gravity(new);
    if (key == 111 || key == 112) rotate(new, key);
}

void left_move(tetromino *new) {
    new->x--;
}

void right_move(tetromino *new) {
    new->x++;
}

void apply_gravity(tetromino *new) {
    new->y--;
}

void rotate(tetromino *new, int key) {
    for (int i = 0; i < 3; i++) {
        int old_dx = new->b[i].dx;
        int old_dy = new->b[i].dy;
        if (key == 112) {
            // clockwise
            new->b[i].dx = old_dy;
            new->b[i].dy = -old_dx;
        } else {
            // counterclockwise
            new->b[i].dx = -old_dy;
            new->b[i].dy = old_dx;
        }
    }
}

// collision (return 1 if collide)
int wall_collision(tetromino *new) {
    if (new->y < piece->y && (board[get_idx_from_coords(new->x, new->y)] == 1 || new->y < 0)) return 1;
    for (int i = 0; i < 3; i++) {
        int x = new->x + new->b[i].dx;
        int y = new->y + new->b[i].dy;
        if (y < (piece->y + piece->b[i].dy) && (board[get_idx_from_coords(x, y)] == 1 || y < 0)) return 1;
    }
    return 0;
}

int side_collision(tetromino *new) {
    if ((new->y == piece->y && board[get_idx_from_coords(new->x, new->y)] == 1) || (new->x < 0 || new->x >= W)) return 1;
    for (int i = 0; i < 3; i++) {
        int x = new->x + new->b[i].dx;
        int y = new->y + new->b[i].dy;
        if ((y == (piece->y + piece->b[i].dy) && board[get_idx_from_coords(x, y)] == 1) || (x < 0 || x >= W)) return 1;
    }
    return 0;
}

// cleaning
void clean_complete_rows() {
    uint8_t new_board[W*H] = {0};
    int cleaned_rows = 0;

    for (int y = 0; y < H; y++) {
        int to_clean = 1;
        for (int x = 0; x < W; x++) {
            if (board[get_idx_from_coords(x, y)] == 0) {
                to_clean = 0;
                break;
            }
        }

        if (to_clean) { cleaned_rows++; continue; }
        for (int x = 0; x < W; x++)
            new_board[get_idx_from_coords(x, y - cleaned_rows)] = board[get_idx_from_coords(x, y)];

    }

    // add n = cleaned_rows empty rows at the top
    for (int y = H - 1; y > H - 1 - cleaned_rows; y--) {
        for (int x = 0; x < W; x++) new_board[get_idx_from_coords(x, y)] = 0;
    }

    // update global board with temp new_board and free allocated mem
    for (int i = 0; i < W*H; i++) board[i] = new_board[i];

    // increment user score
    score += cleaned_rows;
}

// transform piece in a wall component
void freeze_piece() {
    int overflow = piece->y >= H;
    board[get_idx_from_coords(piece->x, piece->y)] = 1;
    for (int i = 0; i < 3; i++) {
        int x = piece->x + piece->b[i].dx;
        int y = piece->y + piece->b[i].dy;
        if (y >= H) overflow = 1;
        board[get_idx_from_coords(x, y)] = 1;
    }
    if (overflow) play = 0;
}

// display
void print_board() {
    clear_screen();
    printf("\nTETRIS\n\n");
    // print top line
    printf("_");
    for (int x = 0; x < W; x++) {
        printf("__");
    }
    printf("_\n");

    for (int y = H - 1; y >= 0; y--) {
        printf("|");
        for (int x = 0; x < W; x++) {
            uint8_t val = board[get_idx_from_coords(x, y)] || piece_or_not(x, y);
            if (val) {
                putchar(219);
                putchar(219);
            }
            else printf("  ");
        }
        printf("|");
        printf("\n");
    }

    // print bottom line
    for (int x = 0; x < W + 1; x++) {
        printf("||");
    }
}

int piece_or_not(int x, int y) {
    if (piece->x == x && piece->y == y) return 1;
    int i = 0;
    while (i < 3) {
        if (piece->x + piece->b[i].dx == x && piece->y + piece->b[i].dy == y) return 1;
        i++;
    }
    return 0;
}


// utils
int get_idx_from_coords(int x, int y) {
    return W*y + x;
}

coords get_coords_from_idx(int i) {
    int x = i % W;
    int y = i / W;
    coords point = { x, y };
    return point;
}

void copy_tetromino(tetromino *new, tetromino *old) {
    new->x = old->x;
    new->y = old->y;
    for (int i = 0; i < 3; i++) {
        new->b[i].dx = old->b[i].dx;
        new->b[i].dy = old->b[i].dy;
    }
}