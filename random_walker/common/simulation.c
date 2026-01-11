#include "simulation.h"
#include <stdlib.h>

Simulation* create_simulation(int width, int height, int K, int reps){
    Simulation* s = malloc(sizeof(Simulation));
    s->world = create_world(width,height);
    s->walker.x = width/2;
    s->walker.y = height/2;
    s->replikacie = reps;
    s->K = K;

    s->avg_steps = malloc(height*sizeof(double*));
    s->prob_success = malloc(height*sizeof(double*));
    for(int y=0;y<height;y++){
        s->avg_steps[y] = malloc(width*sizeof(double));
        s->prob_success[y] = malloc(width*sizeof(double));
        for(int x=0;x<width;x++){
            s->avg_steps[y][x] = 0;
            s->prob_success[y][x] = 0;
        }
    }
    return s;
}

Simulation* create_simulation_with_obstacles(int width, int height, int K, int reps, double ratio){
    Simulation* s = malloc(sizeof(Simulation));
    s->world = create_world_with_obstacles(width,height,ratio);
    s->walker.x = width/2;
    s->walker.y = height/2;
    s->replikacie = reps;
    s->K = K;

    s->avg_steps = malloc(height*sizeof(double*));
    s->prob_success = malloc(height*sizeof(double*));
    for(int y=0;y<height;y++){
        s->avg_steps[y] = malloc(width*sizeof(double));
        s->prob_success[y] = malloc(width*sizeof(double));
        for(int x=0;x<width;x++){
            s->avg_steps[y][x] = 0;
            s->prob_success[y][x] = 0;
        }
    }
    return s;
}

Simulation* create_simulation_from_file(const char* filename, int replikacie) {
    FILE* f = fopen(filename, "r");
    if(!f) return NULL;

    int width, height, K;
    double prob_up, prob_down, prob_left, prob_right;

    // Prvy riadok: rozmer + K + sance
    if(fscanf(f, "%d %d %d %lf %lf %lf %lf\n", &width, &height, &K,
              &prob_up, &prob_down, &prob_left, &prob_right) != 7){
        fclose(f);
        return NULL;
    }

    Simulation* s = malloc(sizeof(Simulation));
    s->replikacie = replikacie;
    s->K = K;
    s->walker.x = width/2;
    s->walker.y = height/2;
    s->walker.prob_up = prob_up;
    s->walker.prob_down = prob_down;
    s->walker.prob_left = prob_left;
    s->walker.prob_right = prob_right;

    s->world = malloc(sizeof(World));
    s->world->width = width;
    s->world->height = height;
    s->world->cells = malloc(height * sizeof(char*));
    for(int y=0;y<height;y++){
        s->world->cells[y] = malloc(width);
        for(int x=0;x<width;x++){
            int c = fgetc(f);
            // preskoc nove riadky
            while(c == '\n' || c == '\r') c = fgetc(f);
            if(c == EOF) c = '*'; // fallback
            s->world->cells[y][x] = (char)c;
        }
    }

    // alokacia sumárnych polí
    s->avg_steps = malloc(height*sizeof(double*));
    s->prob_success = malloc(height*sizeof(double*));
    for(int y=0;y<height;y++){
        s->avg_steps[y] = calloc(width,sizeof(double));
        s->prob_success[y] = calloc(width,sizeof(double));
    }

    fclose(f);
    return s;
}

void destroy_simulation(Simulation* s){
    destroy_world(s->world);
    for(int y=0;y<s->world->height;y++){
        free(s->avg_steps[y]);
        free(s->prob_success[y]);
    }
    free(s->avg_steps);
    free(s->prob_success);
    free(s);
}

// Aktualizujeme len bunku, kde walker stojí
void simulation_update_summary(Simulation* s, int x, int y, int step){
    s->avg_steps[y][x] += step;
    s->prob_success[y][x] += 1.0 / s->replikacie;
}

double simulation_get_avg(Simulation* s,int x,int y){
    return s->avg_steps[y][x];
}

double simulation_get_prob(Simulation* s,int x,int y){
    return s->prob_success[y][x];
}
