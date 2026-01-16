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
#define LOOP_TIME 200

typedef struct { int x, y; } coords;
typedef struct { int8_t dx, dy; } offset;
typedef struct {
    int x, y;
    offset b[4];
    uint8_t type;
} tetromino;

// globals
static int8_t board[BOARD_SIZE] = {0};
tetromino *piece = NULL;
int play = 1;
int score = 0;
int elapsed = 0;

void loop();
void polling_loop();

// random_generation
tetromino *generate_tetromino();

// movement
int move_piece(int key);
int left_move();
int right_move();
int apply_gravity();
int rotate(int key);

// collisions
int calc_offset_x(int dx);
int calc_offset_y(int dy);
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

// score
void update_score(int deleted_rows, int sum_of_heights);


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

    int dx_0, dx_1, dx_2, dx_3,
        dy_0, dy_1, dy_2, dy_3 = 0;
   switch (type) {
       case 'O':
           dx_0 = 0; dy_0 = 1; dx_1 = 1; dy_1 = 1;
           dx_2 = 1; dy_2 = 0; dx_3 = 0; dy_3 = 0;
           break;
       case 'I':
           dx_0 = 0; dy_0 = -1; dx_1 = 0; dy_1 = 1;
           dx_2 = 0; dy_2 = 2; dx_3 = 0; dy_3 = 0;
           break;
       case 'L':
           dx_0 = 0; dy_0 = 1; dx_1 = 0; dy_1 = 2;
           dx_2 = 1; dy_2 = 0; dx_3 = 0; dy_3 = 0;
           break;
       case 'J':
           dx_0 = 0; dy_0 = 1; dx_1 = 0; dy_1 = 2;
           dx_2 = -1; dy_2 = 0; dx_3 = 0; dy_3 = 0;
           break;
       case 'S':
           dx_0 = -1; dy_0 = 0; dx_1 = 0; dy_1 = 1;
           dx_2 = 1; dy_2 = 1; dx_3 = 0; dy_3 = 0;
           break;
       case 'Z':
           dx_0 = -1; dy_0 = 1; dx_1 = 0; dy_1 = 1;
           dx_2 = 1; dy_2 = 0; dx_3 = 0; dy_3 = 0;
           break;
       case 'T':
           dx_0 = -1; dy_0 = 0; dx_1 = 0; dy_1 = 1;
           dx_2 = 1; dy_2 = 0; dx_3 = 0; dy_3 = 0;
           break;
   }

   piece->b[0].dx = dx_0;
   piece->b[1].dx = dx_1;
   piece->b[2].dx = dx_2;
   piece->b[3].dx = dx_3;
   piece->b[0].dy = dy_0;
   piece->b[1].dy = dy_1;
   piece->b[2].dy = dy_2;
   piece->b[3].dy = dy_3;
   return piece;
}

// movement
void polling_loop() {
    int elapsed = 0;
    int key = 0;
    int regenerate = 0;

    regenerate = apply_gravity();
    while (elapsed < LOOP_TIME) {
        sleep_ms(25);
        elapsed += 25;
        // Check input during waiting...
        key = manage_input();
        if (key) { regenerate = move_piece(key); print_board(); }
        
        if (regenerate) {
            freeze_piece();
            clean_complete_rows();
            free(piece);
            piece = generate_tetromino();
            break;
        }
    }
    print_board();
}

int move_piece(int key) {
    if (key == 75 || key == 97) return left_move();
    if (key == 77 || key == 100) return right_move();
    if (key == 80 || key == 115) {
        int grind = apply_gravity();
        if (!grind) score++;
        return grind;
    }
    if (key == 111 || key == 112) return rotate(key);
}

int left_move() {
   int leftest_x = piece->x;
   for (int i = 0; i < 4; i++) {
       int x = piece->x + piece->b[i].dx;
       int y = piece->y + piece->b[i].dy;
       if (board[get_idx_from_coords(x-1, y)] != 0) return 0;
       if (x < leftest_x) { leftest_x = x;}
   }
   if (leftest_x == 0) return 0;
   piece->x--;
   return 0;
}

int right_move() {
   int rightest_x = piece->x;
   for (int i = 0; i < 4; i++) {
       int x = piece->x + piece->b[i].dx;
       int y = piece->y + piece->b[i].dy;
       if (board[get_idx_from_coords(x+1, y)] != 0) return 0;
       if (x > rightest_x) { rightest_x = x; }
   }
   if (rightest_x == W - 1) return 0;
   piece->x++;
   return 0;
}

int apply_gravity() {
   int downest_y = piece->y;
   for (int i = 0; i < 4; i++) {
       int x = piece->x + piece->b[i].dx;
       int y = piece->y + piece->b[i].dy;
       if (board[get_idx_from_coords(x, y-1)] != 0) return 1;
       if (y < downest_y) { downest_y = y; }
   }
   if (downest_y == 0) return 1;
   piece->y--;
   return 0;
}

int rotate(int key) {
    int left_offset_x = 0;
    int right_offset_x = 0;
    int bottom_offset_y = 0;
    tetromino *temp_tetromino = malloc(sizeof(tetromino));
    copy_tetromino(temp_tetromino, piece);
    for (int i = 0; i < 4; i++) {
        int old_dx = temp_tetromino->b[i].dx;
        int old_dy = temp_tetromino->b[i].dy;
        if (key == 112) {            // clockwise
            temp_tetromino->b[i].dx = old_dy;
            temp_tetromino->b[i].dy = -old_dx;

        } else {
            // counterclockwise
            temp_tetromino->b[i].dx = -old_dy;
            temp_tetromino->b[i].dy = old_dx;
        }
        
        int offset_x = calc_offset_x(temp_tetromino->b[i].dx);
        if (offset_x < 0 && offset_x < left_offset_x) left_offset_x = offset_x;
        if (offset_x > 0 && offset_x > right_offset_x) right_offset_x = offset_x;
        int offset_y = calc_offset_y(temp_tetromino->b[i].dy);
        if (bottom_offset_y > offset_y) bottom_offset_y = offset_y;
    }
    
    temp_tetromino->x -= left_offset_x + right_offset_x;
    temp_tetromino->y -= bottom_offset_y;
    
    // if rotation would overlap with other tetromini, cancel operation
    for (int i = 0; i < 4; i++) {
        int x = temp_tetromino->x + temp_tetromino->b[i].dx;
        int y = temp_tetromino->y + temp_tetromino->b[i].dy;
        if (board[get_idx_from_coords(x, y)] != 0) return 0;
    }

    copy_tetromino(piece, temp_tetromino);
    free(temp_tetromino);

    return 0;
}

int calc_offset_x(int dx) {
    int x = piece->x + dx;
    if (x < 0) return x;
    if (x > W-1) return x-W+1;
    return 0;
}

int calc_offset_y(int dy) {
    return piece->y + dy;
}

// cleaning
void clean_complete_rows() {
    uint8_t piece_board[W*H] = {0};
    int cleaned_rows = 0;
    int sum_of_heights = 0;

    for (int y = 0; y < H; y++) {
        int to_clean = 1;
        for (int x = 0; x < W; x++) {
            if (board[get_idx_from_coords(x, y)] == 0) {
                to_clean = 0;
                break;
            }
        }

        if (to_clean) { cleaned_rows++; sum_of_heights += y + 1; continue; }
        for (int x = 0; x < W; x++)
            piece_board[get_idx_from_coords(x, y - cleaned_rows)] = board[get_idx_from_coords(x, y)];

    }

    // add n = cleaned_rows empty rows at the top
    for (int y = H - 1; y > H - 1 - cleaned_rows; y--) {
        for (int x = 0; x < W; x++) piece_board[get_idx_from_coords(x, y)] = 0;
    }

    // update global board with temp piece_board and free allocated mem
    for (int i = 0; i < W*H; i++) board[i] = piece_board[i];

    // increment user score
    update_score(cleaned_rows, sum_of_heights);
}

// transform piece in a wall component
void freeze_piece() {
    int overflow = piece->y >= H;
    board[get_idx_from_coords(piece->x, piece->y)] = 1;
    for (int i = 0; i < 4; i++) {
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
    // buffer size: title + top line + rows + bottom line + null
    char buf[1024];
    int pos = 0;
    // title
    pos += sprintf(buf + pos, "TETRIS\n\n[A]: left; [S]: down; [D]: right; [P]: clockwise; [O]: counter-cw;\n\nScore: %d\n", score);
    // top line
    buf[pos++] = '_';
    for (int x = 0; x < W; x++) {
        buf[pos++] = '_';
        buf[pos++] = '_';
    }
    buf[pos++] = '_';
    buf[pos++] = '\n';
    // board rows
    for (int y = H - 1; y >= 0; y--) {
        buf[pos++] = 219;
        for (int x = 0; x < W; x++) {
            uint8_t val = board[get_idx_from_coords(x, y)] || piece_or_not(x, y);
            if (val) {
                buf[pos++] = 177;
                buf[pos++] = 177;
            } else {
                buf[pos++] = ' ';
                buf[pos++] = ' ';
            }
        }
        buf[pos++] = 219;
        buf[pos++] = '\n';
    }
    // bottom line
    for (int x = 0; x < W + 1; x++) {
        buf[pos++] = 219;
        buf[pos++] = 219;
    }
    buf[pos] = '\0';
    printf("%s", buf);
}

int piece_or_not(int x, int y) {
    if (piece->x == x && piece->y == y) return 1;
    int i = 0;
    while (i < 4) {
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
    for (int i = 0; i < 4; i++) {
        new->b[i].dx = old->b[i].dx;
        new->b[i].dy = old->b[i].dy;
    }
}

// score
void update_score(int deleted_rows, int sum_of_heights) {
    int magic = 1;
    if (deleted_rows == 4) magic *= 4;
    int points = (sum_of_heights) * deleted_rows * magic;
    score += points;
}