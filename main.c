#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "hw2_output.c"
#include <semaphore.h>
#include <time.h>

int** cig_butts;
sem_t** cig_butts_inter;
sem_t** sneakers_inter;
sem_t lock;
sem_t prior;
sem_t sem_break;

int sneaker_cord[8][2] = {{-1,-1},{-1,0},{-1,1},{0,1},{1,1},{1,0},{1,-1},{0,-1}};
enum hw2_actions action;

struct Private_struct
{
    int gid;
    int time;
    int si;
    int sj;
    int ng;
    int** top_corners;
};

struct Sneaker_struct
{
    int sid;
    int time;
    int ns;
    int* cigg_num;
    int** litter_centers;
};

struct Commander
{
    int num_orders;
    int* time_order;
    int* command;
    int num_privates;
    struct Private_struct* privates;
};


void *give_command( void* arguments)
{
    struct Commander *command_list = arguments;
    for(int i = 0; i < command_list->num_orders; i++)
    {
        if(i == 0)
        {
            usleep(command_list->time_order[i] * 1000);
        }
        else
        {
            usleep((command_list->time_order[i] - command_list->time_order[i - 1]) * 1000);
        }
        
        if(command_list->command[i] == 0)
        {

            hw2_notify(action = ORDER_BREAK, 0, 0, 0);
            for(int m = 0; m < command_list->num_privates; m++)
            {
                hw2_notify(action = GATHERER_TOOK_BREAK, command_list->privates[m].gid, 0, 0);
            }
            sem_wait(&sem_break);
        }
        else if(command_list->command[i] == 1)
        {
            hw2_notify(action = ORDER_CONTINUE, 0, 0, 0);
            for(int m = 0; m < command_list->num_privates; m++)
            {
                hw2_notify(action = GATHERER_CONTINUED, command_list->privates[m].gid, 0, 0);
            }
            sem_post(&sem_break);
        }
        else
        {
            hw2_notify(action = ORDER_STOP, 0, 0, 0);
            for(int m = 0; m < command_list->num_privates; m++)
            {
                hw2_notify(action = GATHERER_STOPPED, command_list->privates[m].gid, 0, 0);
            }
        }
        
        
    }
}

struct Commander simplify_orders(struct Commander command_list, int num_privates)
{
    struct Commander new_command_list;
    new_command_list.privates = command_list.privates;
    new_command_list.num_privates = command_list.num_privates;
    new_command_list.num_orders = command_list.num_orders;
    int index = 0;
    int index2 = 0;
    while (command_list.time_order[index])
    {
        if(command_list.command[index] == 2)
        {
            break;
        }
        if(index != 0)
        {
            if (command_list.command[index] != command_list.command[index - 1])
            {
                index2++;
            }
        }
        else
        {
            if(command_list.command[index] == 0)
            {
                index2++;
            }
        }
        index++;
    }
    new_command_list.command = (int*)malloc((index + 1)*sizeof(int));
    new_command_list.time_order = (int*)malloc((index + 1)*sizeof(int));

    int i = 0;
    int index3 = 0;
    while (command_list.time_order[index3])
    {
        if(command_list.command[index3] == 2)
        {
            new_command_list.command[i] = command_list.command[index3];
            new_command_list.time_order[i] = command_list.time_order[index3];
            break;
        }
        if(index3 != 0)
        {
            if (command_list.command[index3] != command_list.command[index3 - 1])
            {
                new_command_list.command[i] = command_list.command[index3];
                new_command_list.time_order[i] = command_list.time_order[index3];
                i++;
            }
        }
        else
        {
            if(command_list.command[index3] == 0)
            {
                new_command_list.command[i] = command_list.command[index3];
                new_command_list.time_order[i] = command_list.time_order[index3];
                i++;
            }
        }
        index3++;
    }
    return new_command_list;

}
void *collect_cigg_butts( void* arguments)
{
    struct Private_struct *private_prop = arguments;
    hw2_notify(action = GATHERER_CREATED, (*private_prop).gid, 0, 0);
    for(int num = 0; num < (*private_prop).ng; num++)
    {
        sem_wait(&prior);
        for(int i = 0; i < (*private_prop).si; i++ )
        {
            for (int j = 0; j < (*private_prop).sj; j++)
            {
                int coord_i = (*private_prop).top_corners[num][0] + i;
                int coord_j = (*private_prop).top_corners[num][1] + j;
                sem_wait(&cig_butts_inter[coord_i][coord_j]);
            }
            
        }
        sem_post(&prior);
        hw2_notify(action = GATHERER_ARRIVED, (*private_prop).gid, (*private_prop).top_corners[num][0],(*private_prop).top_corners[num][1]);
        for(int j = 0; j < (*private_prop).sj; j++)
        {
            for(int i = 0; i < (*private_prop).si; i++ )
            {
                int coord_i = (*private_prop).top_corners[num][0] + i;
                int coord_j = (*private_prop).top_corners[num][1] + j;
                while(cig_butts[coord_i][coord_j] != 0)
                {
                    
                    for(int t = 0; t < (*private_prop).time / 5; t++)
                    {
                        sem_wait(&sem_break);
                        sem_post(&sem_break);
                        usleep(5000);
                    }


                    sem_wait(&lock);
                    cig_butts[coord_i][coord_j]--;
                    hw2_notify(action = GATHERER_GATHERED, (*private_prop).gid, coord_i,coord_j);
                    sem_post(&lock);
                }
            }
        }

        for(int i = 0; i < (*private_prop).si; i++ )
        {
            for (int j = 0; j < (*private_prop).sj; j++)
            {
                int coord_i = (*private_prop).top_corners[num][0] + i;
                int coord_j = (*private_prop).top_corners[num][1] + j;
                sem_post(&cig_butts_inter[coord_i][coord_j]);
            }
            
        }
        hw2_notify(action = GATHERER_CLEARED, (*private_prop).gid, 0, 0);
    }
    hw2_notify(action = GATHERER_EXITED, (*private_prop).gid, 0, 0);
}

void *litter_cigg_butts( void* arguments)
{
    struct Sneaker_struct *sneaker_prop = arguments;
    hw2_notify(action = SNEAKY_SMOKER_CREATED, (*sneaker_prop).sid, 0, 0);
    for(int num = 0; num < (*sneaker_prop).ns; num++)
    {
        sem_wait(&prior);
        for(int i = 0; i < 3; i++ )
        {
            for (int j = 0; j < 3; j++)
            {
                int coord_i = (*sneaker_prop).litter_centers[num][0] - 1 + i;
                int coord_j = (*sneaker_prop).litter_centers[num][1] - 1 + j;
                sem_wait(&cig_butts_inter[coord_i][coord_j]);
            }
            
        }
        sem_wait(&sneakers_inter[(*sneaker_prop).litter_centers[num][0]][(*sneaker_prop).litter_centers[num][1]]);
        sem_post(&prior);
        hw2_notify(action = SNEAKY_SMOKER_ARRIVED, (*sneaker_prop).sid, (*sneaker_prop).litter_centers[num][0],(*sneaker_prop).litter_centers[num][1]);
        for(int i = 0; i < (*sneaker_prop).cigg_num[num]; i++ )
        {
            int coord_i = (*sneaker_prop).litter_centers[num][0] + sneaker_cord[i % 8][0];
            int coord_j = (*sneaker_prop).litter_centers[num][1] + sneaker_cord[i % 8][1];

                
            for(int t = 0; t < (*sneaker_prop).time / 5; t++)
            {
                usleep(5000);
            }

            sem_wait(&lock);
            cig_butts[coord_i][coord_j]++;
            hw2_notify(action = SNEAKY_SMOKER_FLICKED, (*sneaker_prop).sid, coord_i,coord_j);
            sem_post(&lock);
        }
        
        for(int i = 0; i < 3; i++ )
        {
            for (int j = 0; j < 3; j++)
            {
                int coord_i = (*sneaker_prop).litter_centers[num][0] - 1 + i;
                int coord_j = (*sneaker_prop).litter_centers[num][1] - 1 + j;
                sem_post(&cig_butts_inter[coord_i][coord_j]);
            }
            
        }
        sem_post(&sneakers_inter[(*sneaker_prop).litter_centers[num][0]][(*sneaker_prop).litter_centers[num][1]]);

    }
}


int main()
{
    int gridx, gridy;
    int num_privates;
    int num_orders;
    int num_sneakers;
    char cmd[256]; 
    scanf("%d %d", &gridx, &gridy);
    cig_butts = (int**)malloc(gridx * sizeof(int*));
    cig_butts_inter = (sem_t**)malloc(gridx * sizeof(sem_t*));
    sneakers_inter = (sem_t**)malloc(gridx * sizeof(sem_t*));
    for(int i = 0; i < gridx; i++)
    {
        cig_butts[i] = (int*)malloc(gridy * sizeof(int));
        cig_butts_inter[i] = (sem_t*)malloc(gridy * sizeof(sem_t));
        sneakers_inter[i] = (sem_t*)malloc(gridy * sizeof(sem_t));
        for(int j = 0; j < gridy; j++)
        {
            scanf("%d", &cig_butts[i][j]);
            sem_init(&cig_butts_inter[i][j], 0, 1);
        }
    }
    sem_init(&lock, 0, 1);
    sem_init(&prior, 0, 1);
    sem_init(&sem_break, 0, 1);
    scanf("%d", &num_privates);
    struct Private_struct* privates;
    struct Commander commander_orders;
    struct Sneaker_struct* sneakers;
    commander_orders.num_privates = num_privates;
    pthread_t private_thread[num_privates];
    pthread_t commander_thread;
    privates = (struct Private_struct*)malloc(sizeof(struct Private_struct)* num_privates);
    for(int i = 0; i < num_privates; i++)
    {
        scanf("%d %d %d %d %d", &privates[i].gid, &privates[i].si, &privates[i].sj, &privates[i].time, &privates[i].ng);
        privates[i].top_corners = (int**)malloc(privates[i].ng * sizeof(int*));
        for(int j= 0; j < privates[i].ng; j++)
        {
            privates[i].top_corners[j] = (int*)malloc(2 * sizeof(int));
            scanf("%d %d", &privates[i].top_corners[j][0], &privates[i].top_corners[j][1]);
        }
    }
    commander_orders.privates = privates;
    scanf("%d", &num_orders);
    commander_orders.command = (int*)malloc(num_orders*sizeof(int));
    commander_orders.time_order = (int*)malloc(num_orders*sizeof(int));
    commander_orders.num_orders = num_orders;
    for(int i = 0; i < num_orders; i++)
    {
        scanf("%d", &commander_orders.time_order[i]);
        fgets(cmd, sizeof(cmd), stdin);
        if(cmd[1] == 'b')
        {
            commander_orders.command[i] = 0;
        }
        else if(cmd[1] == 'c')
        {
            commander_orders.command[i] = 1;
        }
        else if(cmd[1] == 's')
        {
            commander_orders.command[i] = 2;
        }
    }

    scanf("%d", &num_sneakers);
    pthread_t sneakers_thread[num_sneakers];
    sneakers = (struct Sneaker_struct*)malloc(sizeof(struct Sneaker_struct)* num_sneakers);
    for(int i = 0; i < num_sneakers; i++)
    {
        scanf("%d %d %d", &sneakers[i].sid, &sneakers[i].time, &sneakers[i].ns);
        sneakers[i].litter_centers = (int**)malloc(sneakers[i].ns * sizeof(int*));
        sneakers[i].cigg_num = (int*)malloc(sneakers[i].ns * sizeof(int));
        for(int j= 0; j < sneakers[i].ns; j++)
        {
            sneakers[i].litter_centers[j] = (int*)malloc(2 * sizeof(int));
            scanf("%d %d %d", &sneakers[i].cigg_num[j], &sneakers[i].litter_centers[j][0], &sneakers[i].litter_centers[j][1]);
        }
    }
    struct Commander simple_orders = simplify_orders(commander_orders, num_privates);
    pthread_create( &commander_thread, NULL, &give_command, (void*)&simple_orders);

    hw2_init_notifier();
    for(int i = 0; i < num_privates; i++)
    {
        pthread_create( &private_thread[i], NULL, &collect_cigg_butts, (void*)&privates[i]);
    }

    for(int i = 0; i < num_sneakers; i++)
    {
        pthread_create( &sneakers_thread[i], NULL, &litter_cigg_butts, (void*)&sneakers[i]);
    }


    pthread_join(commander_thread, NULL);
    for(int i = 0; i < num_privates; i++)
    {
        pthread_join( private_thread[i], NULL);
    }

    return 0;

}