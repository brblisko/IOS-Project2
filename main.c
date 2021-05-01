#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <time.h>

typedef struct reindeers
{
    int cnt;
    int time;
} reindeers;

typedef struct elfs
{
    int cnt;
    int time;
} elfs;

typedef struct shared
{
    sem_t *mainSem;
    sem_t *mutexSem;
    int *processCnt;
} shared;

bool processArgs(char **argv, reindeers *RD, elfs *EF);
void semOpener(shared *SM);
void memClean(shared *SM);
void santa(shared *SM);
void elf(shared *SM, int efID);
void raindeer(shared *SM, int rdID);

int main(int argc, char **argv)
{
    setbuf(stdout, NULL);
    reindeers RD;
    elfs EF;
    shared SM;

    if (argc == 5)
    {
        if (!processArgs(argv, &RD, &EF))
        {
            return 1;
        }
    }
    else
    {
        printf("To use this program please enter 4 numbers as arguments\n");
        return 0;
    }
    sem_t *mainSem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_t *mutexSem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int *processCnt = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *processCnt = EF.cnt + RD.cnt + 1; // plus one santa

    SM.mainSem = mainSem;
    SM.mutexSem = mutexSem;
    SM.processCnt = processCnt;

    sem_init(mainSem, 1, 0);
    sem_init(mutexSem, 1, 1);
    printf("%d\n", *processCnt);

    int pid = fork(); //vytvorime santu

    if (pid < 0)
    {
        semOpener(&SM);
        memClean(&SM);
        return 1;
    }
    if (pid == 0)
    {
        santa(&SM);
        exit(0);
    }

    for (int i = 0; i < EF.cnt; i++)
    {
        int pid = fork();
        if (pid < 0)
        {
            semOpener(&SM);
            memClean(&SM);
            return 1;
        }
        if (pid == 0)
        {
            elf(&SM, i + 1);
            exit(0);
        }
    }

    for (int i = 0; i < RD.cnt; i++)
    {
        pid = fork();
        if (pid < 0)
        {
            semOpener(&SM);
            memClean(&SM);
            return 1;
        }
        if (pid == 0)
        {
            raindeer(&SM, i + 1);
            exit(0);
        }
    }

    sem_wait(mainSem);
    printf("zavrel sa main\n");
    memClean(&SM);
    return 0;
}

void santa(shared *SM)
{
    printf("haha hohoho som santa teraz budem 1sec cakat\n");
    printf("zavrel sa santa\n");
    (*SM->processCnt)--;
    printf("%d\n", *SM->processCnt);
    if (*SM->processCnt == 0)
    {
        sem_post(SM->mainSem);
    }
}

void elf(shared *SM, int efID)
{
    (*SM->processCnt)--;
    printf("zavrel sa elf\n");
    printf("%d\n", *SM->processCnt);
    if (*SM->processCnt == 0)
    {
        sem_post(SM->mainSem);
    }
}

void raindeer(shared *SM, int rdID)
{
    (*SM->processCnt)--;
    printf("zavrel sa raindeer\n");
    printf("%d\n", *SM->processCnt);
    if (*SM->processCnt == 0)
    {
        sem_post(SM->mainSem);
    }
}

void memClean(shared *SM)
{
    sem_destroy(SM->mainSem);
    sem_destroy(SM->mutexSem);
    munmap(SM->mainSem, sizeof(sem_t));
    munmap(SM->mutexSem, sizeof(sem_t));
    munmap(SM->processCnt, sizeof(int));
}

void semOpener(shared *SM)
{
    for (int i = 0; i < *SM->processCnt; i++)
    {
        sem_post(SM->mainSem);
        sem_post(SM->mutexSem);
    }
}

bool processArgs(char **argv, reindeers *RD, elfs *EF)
{
    char *endPtr;
    EF->cnt = strtol(argv[1], &endPtr, 10);
    if (*endPtr != '\0')
    {
        fprintf(stderr, "ERROR - could not convert \"%s\" to integer\nPlease enter integer\n", argv[1]);
        return false;
    }

    if (EF->cnt <= 0)
    {
        fprintf(stderr, "ERROR - elf counter can't be less or equal to 0\n");
        return false;
    }
    else if (EF->cnt >= 1000)
    {
        fprintf(stderr, "ERROR - elf counter can't be more or equal to 1000\n");
        return false;
    }

    EF->time = strtol(argv[3], &endPtr, 10);
    if (*endPtr != '\0')
    {
        fprintf(stderr, "ERROR - could not convert \"%s\" to integer\nPlease enter integer\n", argv[3]);
        return false;
    }

    if (EF->time < 0)
    {
        fprintf(stderr, "ERROR - elf time can't be less then 0\n");
        return false;
    }
    else if (EF->time > 1000)
    {
        fprintf(stderr, "ERROR - elf time can't be more then 1000\n");
        return false;
    }

    RD->cnt = strtol(argv[2], &endPtr, 10);
    if (*endPtr != '\0')
    {
        fprintf(stderr, "ERROR - could not convert \"%s\" to integer\nPlease enter integer\n", argv[2]);
        return false;
    }
    if (RD->cnt <= 0)
    {
        fprintf(stderr, "ERROR - reindeer counter can't be less or equal to 0\n");
        return false;
    }
    else if (RD->cnt >= 20)
    {
        fprintf(stderr, "ERROR - reindeer counter can't be more or equal to 20\n");
        return false;
    }

    RD->time = strtol(argv[4], &endPtr, 10);
    if (*endPtr != '\0')
    {
        fprintf(stderr, "ERROR - could not convert \"%s\" to integer\nPlease enter integer\n", argv[4]);
        return false;
    }

    if (RD->time < 0)
    {
        fprintf(stderr, "ERROR - reindeer time can't be less then 0\n");
        return false;
    }
    else if (RD->time > 1000)
    {
        fprintf(stderr, "ERROR - reindeer time can't be more then 1000\n");
        return false;
    }
    return true;
}
