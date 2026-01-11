#include "../common/protocol.h"
#include "../common/world.h"
#include "../common/simulation.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <termios.h>
#include <string.h>
#include <errno.h>

#define VYSTUPNY_DIR "../vystupny"

int fd_cmd[2];
int fd_state[2];
volatile int running = 0;
volatile SimMode mode = MODE_INTERACTIVE;
volatile int sum_type = 0;
pthread_mutex_t mode_mutex = PTHREAD_MUTEX_INITIALIZER;

Simulation *sim;

// Dynamické world pole, alokované až po prvom WorldState
World w = {0};

int getch(){
    struct termios oldt,newt;
    int ch;
    tcgetattr(STDIN_FILENO,&oldt);
    newt=oldt;
    newt.c_lflag &= ~(ICANON|ECHO);
    tcsetattr(STDIN_FILENO,TCSANOW,&newt);
    ch=getchar();
    tcsetattr(STDIN_FILENO,TCSANOW,&oldt);
    return ch;
}

ssize_t read_full(int fd, void *buf, size_t count) {
    size_t read_bytes = 0;
    char *ptr = buf;
    while (read_bytes < count) {
        ssize_t n = read(fd, ptr + read_bytes, count - read_bytes);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (n == 0) break;
        read_bytes += n;
    }
    return read_bytes;
}

void* input_thread(void* arg){
    while(running){
        char c = getch();
        CommandType cmd;
        if(c=='i') cmd = CMD_MODE_SWITCH;
        else if(c=='t') cmd = CMD_SUM_TYPE_SWITCH;
        else if(c=='q') cmd = CMD_QUIT;
        else continue;

        write(fd_cmd[1],&cmd,sizeof(cmd));
        if(cmd==CMD_QUIT) break;
    }
    return NULL;
}

void* render_thread(void* arg){
    WorldState ws;
    int initialized = 0;
    while(running){
        if(read_full(fd_state[0],&ws,sizeof(ws))!=sizeof(ws)){
            if(!running) break;
            continue;
        }

        // dynamická alokácia world len raz
        if(!initialized){
            w.width = ws.width;
            w.height = ws.height;
            w.cells = malloc(w.height*sizeof(char*));
            for(int y=0;y<w.height;y++)
                w.cells[y] = malloc(w.width);
            initialized = 1;
        }

        // Kopírujeme len presne ws.height x ws.width
        for(int y=0;y<ws.height;y++)
            for(int x=0;x<ws.width;x++)
                w.cells[y][x] = ws.cells[y][x];

        pthread_mutex_lock(&mode_mutex);
        SimMode current_mode = mode;
        int current_sum_type = sum_type;
        pthread_mutex_unlock(&mode_mutex);

        system("clear");
        fflush(stdout);

        printf("Replikacia %d / %d | Krok %d\n",ws.current_rep,ws.total_rep,ws.step);

        if(current_mode==MODE_INTERACTIVE){
            // vykreslenie presne ws.width/ws.height
            for(int y=0;y<ws.height;y++){
                for(int x=0;x<ws.width;x++){
                    if(x==ws.walker_x && y==ws.walker_y) putchar('C');
                    else putchar(w.cells[y][x]);
                }
                putchar('\n');
            }
        } else {
            for(int y=0;y<ws.height;y++){
                for(int x=0;x<ws.width;x++){
                    if(w.cells[y][x]=='#') putchar('#');
                    else if(current_sum_type==0)
                        printf("%3.0f",simulation_get_avg(sim,x,y));
                    else
                        printf("%3.0f",100*simulation_get_prob(sim,x,y));
                }
                putchar('\n');
            }
        }
    }

    // uvoľnenie dynamickej pamäte
    if(initialized){
        for(int y=0;y<w.height;y++) free(w.cells[y]);
        free(w.cells);
        w.cells = NULL;
    }

    return NULL;
}



void create_sim(int use_file){
    ClientConfig client_cfg = {0};
    SimulationConfig cfg;

    if(use_file){
        client_cfg.from_file = 1;
        char fname[128];
        printf("Nazov suboru v vystupny: ");
        scanf("%127s",&fname);
        snprintf(client_cfg.file_name,sizeof(client_cfg.file_name),"%s/%s",VYSTUPNY_DIR,fname);
        printf("Zadaj pocet replikacii: ");
        scanf("%d",&cfg.replikacie);
        client_cfg.cfg.replikacie = cfg.replikacie;
    } else {
        client_cfg.from_file = 0;
        printf("Sirka vyska: "); scanf("%d %d",&cfg.width,&cfg.height);
        printf("Replikacie: "); scanf("%d",&cfg.replikacie);
        printf("K: "); scanf("%d",&cfg.K);
        printf("Pravd UP DOWN LEFT RIGHT: "); scanf("%lf %lf %lf %lf",&cfg.prob_up,&cfg.prob_down,&cfg.prob_left,&cfg.prob_right);
        printf("Typ sveta (0=bez prekazok,1=s prekazkami): "); scanf("%d",&cfg.world_type);
        cfg.obstacle_ratio=0;
        if(cfg.world_type==1){
            printf("Ratio prekazok: "); scanf("%lf",&cfg.obstacle_ratio);
        }
        client_cfg.cfg = cfg;
    }

    pipe(fd_cmd);
    pipe(fd_state);

    pid_t pid = fork();
    if(pid==0){
        dup2(fd_cmd[0],STDIN_FILENO);
        dup2(fd_state[1],STDOUT_FILENO);
        execl("./server/server","server",NULL);
        exit(1);
    }

    // Posielame celý ClientConfig
    write(fd_cmd[1],&client_cfg,sizeof(client_cfg));
    running = 1;

    pthread_t in_th, rd_th;
    pthread_create(&in_th,NULL,input_thread,NULL);
    pthread_create(&rd_th,NULL,render_thread,NULL);

    int status;
    while (running) {
        pid_t r = waitpid(pid, &status, WNOHANG);
        if (r != 0) break;
        usleep(50000);
    }
    running=0;

    pthread_join(in_th,NULL);
    pthread_join(rd_th,NULL);
}

int main(){
    int c;
    while(1){
        printf("\n1 - Nova simulacia\n2 - Simulacia zo suboru\n3 - Koniec\nVolba: ");
        scanf("%d",&c);
        getchar();
        if(c==1) create_sim(0);
        else if(c==2) create_sim(1);
        else break;
    }
    return 0;
}
