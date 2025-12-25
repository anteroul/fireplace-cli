#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <curses.h>

#define FRAME_DELAY 128000
#define FIRE_HEIGHT 32
#define MAX_HEAT    23

int fire[FIRE_HEIGHT][512];
enum { MODE_BASIC, MODE_256 };
int color_mode = MODE_BASIC;

char flame_char(int heat)
{
    if (heat < 6) return '.';
    if (heat < 12) return '^';
    if (heat < 18) return '/';
    return 'A';
}

void init_fire_colors()
{
    if (COLORS >= 256) {
        color_mode = MODE_256;
        for (int i = 0; i <= MAX_HEAT; i++) {
            init_pair(i + 1, 16 + i, -1);
        }
    } else {
        color_mode = MODE_BASIC;
        init_pair(1, COLOR_RED, -1);
        init_pair(2, COLOR_YELLOW, -1);
        init_pair(3, COLOR_WHITE, -1);
    }
    attron(A_BOLD | COLOR_PAIR(1));
}

int main(void)
{
    srand(time(NULL));
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);

    if (!has_colors()) {
        endwin();
        fprintf(stderr, "Terminal has no color support.\n");
        return 1;
    }

    start_color();
    use_default_colors();
    init_fire_colors();

    int term_h, term_w;
    getmaxyx(stdscr, term_h, term_w);
    if (term_w > 512) term_w = 512;

    // Clear fire buffer
    for (int y = 0; y < FIRE_HEIGHT; y++)
        for (int x = 0; x < term_w; x++)
            fire[y][x] = 0;

    while (1) {
        getmaxyx(stdscr, term_h, term_w);
        int base_y = term_h - FIRE_HEIGHT;
        erase();

        // Ignite bottom row randomly
        for (int x = 0; x < term_w; x++) {
            fire[FIRE_HEIGHT - 1][x] = rand() % (MAX_HEAT + 1);
        }

        // Propagate heat upwards
        for (int y = FIRE_HEIGHT - 2; y >= 0; y--) {
            for (int x = 0; x < term_w; x++) {
                int left  = (x > 0) ? fire[y + 1][x - 1] : 0;
                int below = fire[y + 1][x];
                int right = (x < term_w - 1) ? fire[y + 1][x + 1] : 0;
                int new_heat = (left + below + right) / 3;
                new_heat -= rand() % 2;
                if (new_heat < 0) new_heat = 0;
                fire[y][x] = new_heat;
            }
        }

        // Draw fire
        for (int y = 0; y < FIRE_HEIGHT; y++) {
            for (int x = 0; x < term_w; x++) {
                int heat = fire[y][x];
                if (heat > MAX_HEAT) heat = MAX_HEAT;

                if (color_mode == MODE_256) {
                    attron(COLOR_PAIR(heat + 1));
                } else {
                    // 8-color fallback
                    if (heat < 8) attron(COLOR_PAIR(1));
                    else if (heat < 16) attron(COLOR_PAIR(2));
                    else attron(COLOR_PAIR(3));
                }

                mvaddch(base_y + y, x, flame_char(heat));
            }
        }

        // Optional log base
        if (color_mode == MODE_256) attron(COLOR_PAIR(MAX_HEAT / 2));
        else attron(COLOR_PAIR(3));
        for (int x = 0; x < term_w; x++)
            mvaddch(term_h - 1, x, '=');

        refresh();
        usleep(FRAME_DELAY);

        if (getch() == 'q')
            break;
    }

    endwin();
    return 0;
}
