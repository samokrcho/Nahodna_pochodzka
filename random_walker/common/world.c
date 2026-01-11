#include "world.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

World* create_world(int width, int height){
    World* w = malloc(sizeof(World));
    if(!w) return NULL;
    w->width = width;
    w->height = height;
    w->cells = malloc(height*sizeof(char*));
    if(!w->cells){
	free(w);
	return NULL;
    }
    for(int y=0;y<height;y++){
        w->cells[y] = malloc(width);
        for(int x=0;x<width;x++) w->cells[y][x]='*';
    }
    return w;
}

void destroy_world(World *w){
    if(!w) return;
    for(int y=0;y<w->height;y++) free(w->cells[y]);
    free(w->cells);
    free(w);
}

// Pomocná funkcia na overenie dosiahnuteľnosti (BFS)
static void ensure_reachability(World *w){
    int *visited = calloc(w->width * w->height, sizeof(int));
    int queue[w->width*w->height][2];
    int front=0, back=0;

    queue[back][0]=0; queue[back][1]=0; back++;
    visited[0]=1;

    int dirs[4][2]={{0,1},{0,-1},{1,0},{-1,0}};

    while(front<back){
        int x=queue[front][0];
        int y=queue[front][1];
        front++;

        for(int d=0;d<4;d++){
            int nx = x + dirs[d][0];
            int ny = y + dirs[d][1];
            if(nx<0 || nx>=w->width || ny<0 || ny>=w->height) continue;
            if(w->cells[ny][nx]=='#') continue;
            if(visited[ny*w->width+nx]) continue;
            visited[ny*w->width+nx]=1;
            queue[back][0]=nx; queue[back][1]=ny; back++;
        }
    }

    // všetky nedosiahnuteľné políčka zmeniť na prekážky
    for(int y=0;y<w->height;y++){
        for(int x=0;x<w->width;x++){
            if(!visited[y*w->width+x]) w->cells[y][x]='#';
        }
    }
    free(visited);
}

World* create_world_with_obstacles(int width, int height, double obstacle_ratio){
    srand(time(NULL));
    World* w = create_world(width,height);
    if(!w) return NULL;

    // vygenerujeme prekážky
    for(int y=0;y<height;y++){
        for(int x=0;x<width;x++){
            if(x==0 && y==0) continue; // štart [0,0] nikdy nie je prekážka
            double r = (double)rand()/RAND_MAX;
            if(r < obstacle_ratio) w->cells[y][x]='#';
        }
    }

    ensure_reachability(w);
    return w;
}

void print_world(World *w, int walker_x, int walker_y){
    for(int y=0;y<w->height;y++){
        for(int x=0;x<w->width;x++){
            if(x==walker_x && y==walker_y) putchar('C');
            else putchar(w->cells[y][x]);
        }
        putchar('\n');
    }
}