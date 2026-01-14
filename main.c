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
 #define LOOP_TIME 500
 
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
 int elapsed = 0;
 
 void loop();
 void polling_loop();
 
 // random_generation
 tetromino *generate_tetromino();
 
 // movement
 int manage_input();
 int move_piece(int key);
 int left_move();
 int right_move();
 int apply_gravity();
 int rotate(int key);
 
 // collisions
 int wall_collision();
 void freeze_piece();
 
 // cleaning
 void clean_complete_rows();
 
 // display
 void print_board();
 int piece_or_not(int x, int y);
 
 // utils
 int get_idx_from_coords(int x, int y);
 coords get_coords_from_idx(int i);
 
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
    int elapsed = 0;
    int key = 0;
    int regenerate = 0;

    regenerate = apply_gravity();
    while (elapsed < LOOP_TIME) {
        sleep_ms(10);
        elapsed += 10;
        int reprint = 0;
        // Check input during waiting...
        key = manage_input();
        if (key) { regenerate = move_piece(key); }
        
        if (regenerate) {
            freeze_piece();
            clean_complete_rows();
            free(piece);
            piece = generate_tetromino();
            reprint = 1;
            break;
        }

        if (key) reprint = 1;
        if (reprint) print_board();

    }
    
    print_board();
}
 
 int move_piece(int key) {
     if (key == 75 || key == 97) return left_move();
     if (key == 77 || key == 100) return right_move();
     if (key == 80 || key == 115) return apply_gravity();
     if (key == 111 || key == 112) return rotate(key);
 }
 
 int left_move() {
    int leftest_x = piece->x;
    for (int i = 0; i < 3; i++) {
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
    for (int i = 0; i < 3; i++) {
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
    for (int i = 0; i < 3; i++) {
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
     for (int i = 0; i < 3; i++) {
         int old_dx = piece->b[i].dx;
         int old_dy = piece->b[i].dy;
         if (key == 112) {
             // clockwise
             piece->b[i].dx = old_dy;
             piece->b[i].dy = -old_dx;
         } else {
             // counterclockwise
             piece->b[i].dx = -old_dy;
             piece->b[i].dy = old_dx;
         }
     }
     return 0;
 }
 
 // collision (return 1 if collide)
 int wall_collision() {
     if (piece->y < piece->y && (board[get_idx_from_coords(piece->x, piece->y)] == 1 || piece->y < 0)) return 1;
     for (int i = 0; i < 3; i++) {
         int x = piece->x + piece->b[i].dx;
         int y = piece->y + piece->b[i].dy;
         if (y < (piece->y + piece->b[i].dy) && (board[get_idx_from_coords(x, y)] == 1 || y < 0)) return 1;
     }
     return 0;
 }
 
 // cleaning
 void clean_complete_rows() {
     uint8_t piece_board[W*H] = {0};
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
             piece_board[get_idx_from_coords(x, y - cleaned_rows)] = board[get_idx_from_coords(x, y)];
 
     }
 
     // add n = cleaned_rows empty rows at the top
     for (int y = H - 1; y > H - 1 - cleaned_rows; y--) {
         for (int x = 0; x < W; x++) piece_board[get_idx_from_coords(x, y)] = 0;
     }
 
     // update global board with temp piece_board and free allocated mem
     for (int i = 0; i < W*H; i++) board[i] = piece_board[i];
 
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
