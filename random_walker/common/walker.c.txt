#include "walker.h"
#include <stdlib.h>

void walker_step(Walker *w, int width, int height, char **cells) {
    double r = (double)rand() / RAND_MAX;
    double p_up = w->prob_up;
    double p_down = w->prob_down;
    double p_left = w->prob_left;
    double p_right = w->prob_right;

    int nx = w->x;
    int ny = w->y;

    if(r < p_up) ny--;
    else if(r < p_up + p_down) ny++;
    else if(r < p_up + p_down + p_left) nx--;
    else nx++;

    // Wrap-around pre mriežku
    if(nx < 0) nx = width - 1;
    if(nx >= width) nx = 0;
    if(ny < 0) ny = height - 1;
    if(ny >= height) ny = 0;

    // Kontrola prekážok
    if(cells[ny][nx] != '#') {
        w->x = nx;
        w->y = ny;
    }
}