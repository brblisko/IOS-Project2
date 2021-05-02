#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <stdarg.h>

typedef struct reindeers
{
    int cnt;
    int time;
} reindeers;

typedef struct elves
{
    int cnt;
    int time;
} elves;

typedef struct shared
{
    sem_t *mainSem;
    sem_t *mutexSem;
    sem_t *elfSem;
    sem_t *rdSem;
    sem_t *santaSem;
    sem_t *santaElfSem;
    sem_t *santaRdSem;
    int *processCnt;
    int *lineCnt;
    int *elfLine;
    int *rdCnt;
    int *elfHelped;
    bool *xmas;
} shared;

#define mineRand(x) (rand() % (x + 1)) * 1000

bool processArgs(char **argv, reindeers *RD, elves *EF);
void semOpener(shared *SM);
void memClean(shared *SM, FILE *file);
void santa(shared *SM, reindeers *RD, elves *EF, FILE *file);
void elf(shared *SM, int efID, int slpTime, FILE *file);
void reindeer(shared *SM, int rdID, int slpTime, FILE *file);
void my_print(shared *SM, FILE *file, const char *format, ...);

int main(int argc, char **argv)
{
    reindeers RD;
    elves EF;
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

    FILE *file = fopen("proj2.out", "w");
    if (file == NULL)
    {
        fprintf(stderr, "ERROR - couldn't open file \"proj2.out\"");
        return 1;
    }

    sem_t *mainSem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_t *mutexSem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_t *elfSem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_t *rdSem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_t *santaSem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_t *santaElfSem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_t *santaRdSem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int *processCnt = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int *lineCnt = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int *elfLine = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int *rdCnt = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int *elfHelped = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    bool *xmas = (bool *)mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    *elfHelped = 0;
    *rdCnt = RD.cnt;
    *elfLine = 0;
    *lineCnt = 1;
    *xmas = false;
    *processCnt = EF.cnt + RD.cnt + 1; // plus one santa

    SM.mainSem = mainSem;
    SM.mutexSem = mutexSem;
    SM.elfSem = elfSem;
    SM.processCnt = processCnt;
    SM.lineCnt = lineCnt;
    SM.xmas = xmas;
    SM.elfLine = elfLine;
    SM.rdCnt = rdCnt;
    SM.rdSem = rdSem;
    SM.santaSem = santaSem;
    SM.santaElfSem = santaElfSem;
    SM.santaRdSem = santaRdSem;
    SM.elfHelped = elfHelped;

    sem_init(mainSem, 1, 0);
    sem_init(elfSem, 1, 0);
    sem_init(mutexSem, 1, 1);
    sem_init(rdSem, 1, 0);
    sem_init(santaSem, 1, 0);
    sem_init(santaElfSem, 1, 0);
    sem_init(santaRdSem, 1, 0);

    int pid = fork(); //vytvorime santu

    if (pid < 0)
    { // error checkume fork
        semOpener(&SM);
        memClean(&SM, file);
        return 1;
    }
    if (pid == 0)
    { // procesujeme santu
        santa(&SM, &RD, &EF, file);
        exit(0);
    }

    for (int i = 0; i < EF.cnt; i++)
    { // vytvarama n pocet elfov
        int pid = fork();
        if (pid < 0)
        { // erorr checkujeme fork
            semOpener(&SM);
            memClean(&SM, file);
            return 1;
        }
        if (pid == 0)
        { // procesujeme elfa
            elf(&SM, i + 1, RD.time, file);
            exit(0);
        }
    }

    for (int i = 0; i < RD.cnt; i++)
    { // vytvarame reindeerov
        pid = fork();
        if (pid < 0)
        { // error checkujeme fork
            semOpener(&SM);
            memClean(&SM, file);
            return 1;
        }
        if (pid == 0)
        { // procesujeme reindeera
            reindeer(&SM, i + 1, RD.time, file);
            exit(0);
        }
    }

    sem_wait(mainSem);
    memClean(&SM, file); // koncime
    return 0;
}

void santa(shared *SM, reindeers *RD, elves *EF, FILE *file)
{
    sem_wait(SM->mutexSem);
    my_print(SM, file, "Santa: going to sleep\n");
    sem_post(SM->mutexSem);

    while (1)
    {
        sem_wait(SM->santaSem);
        if (*SM->rdCnt == 0)
        {
            sem_wait(SM->mutexSem);
            my_print(SM, file, "Santa: closing workshop\n");
            *SM->xmas = true;
            *SM->rdCnt = RD->cnt;
            for (int i = 0; i < RD->cnt; i++)
            {
                sem_post(SM->rdSem);
            }

            for (int i = 0; i < EF->cnt; i++)
            {
                sem_post(SM->elfSem);
            }

            sem_post(SM->mutexSem);
            break;
        }
        else if (*SM->elfLine >= 3)
        {
            sem_wait(SM->mutexSem);
            my_print(SM, file, "Santa: helping elves\n");
            *SM->elfLine -= 3;
            *SM->elfHelped = 3;
            sem_post(SM->elfSem);
            sem_post(SM->elfSem);
            sem_post(SM->elfSem);
            sem_post(SM->mutexSem);
        }
        sem_wait(SM->santaElfSem);
        sem_wait(SM->mutexSem);
        my_print(SM, file, "Santa: going to sleep\n");
        sem_post(SM->mutexSem);
    }
    sem_wait(SM->santaRdSem);
    sem_wait(SM->mutexSem);
    my_print(SM, file, "Santa: Christmas started\n");
    (*SM->processCnt)--;
    if (*SM->processCnt == 0)
    {
        sem_post(SM->mainSem);
    }
    sem_post(SM->mutexSem);
}

void elf(shared *SM, int efID, int slpTime, FILE *file)
{
    sem_wait(SM->mutexSem);
    my_print(SM, file, "Elf %d: started\n", efID);
    sem_post(SM->mutexSem);

    while (1)
    {
        usleep(mineRand(slpTime));

        sem_wait(SM->mutexSem);
        my_print(SM, file, "Elf %d: need help\n", efID);
        (*SM->elfLine)++;
        if (*SM->elfLine == 3)
        {
            sem_post(SM->santaSem);
        }
        sem_post(SM->mutexSem);

        sem_wait(SM->elfSem);
        if (*SM->xmas)
        {
            break;
        }

        sem_wait(SM->mutexSem);
        my_print(SM, file, "Elf %d: get help\n", efID);
        (*SM->elfHelped)--;
        if (*SM->elfHelped == 0)
        {
            sem_post(SM->santaElfSem);
        }
        sem_post(SM->mutexSem);
    }

    sem_wait(SM->mutexSem);
    my_print(SM, file, "Elf %d: taking holidays\n", efID);
    (*SM->processCnt)--;
    if (*SM->processCnt == 0)
    {
        sem_post(SM->mainSem);
    }
    sem_post(SM->mutexSem);
}

void reindeer(shared *SM, int rdID, int slpTime, FILE *file)
{
    sem_wait(SM->mutexSem);
    my_print(SM, file, "RD %d: rstarted\n", rdID);
    sem_post(SM->mutexSem);

    usleep(mineRand(slpTime));

    sem_wait(SM->mutexSem);
    my_print(SM, file, "RD %d: return home\n", rdID);
    (*SM->rdCnt)--;
    if (*SM->rdCnt == 0)
    {
        sem_post(SM->santaSem);
    }
    sem_post(SM->mutexSem);

    sem_wait(SM->rdSem);
    sem_wait(SM->mutexSem);
    (*SM->rdCnt)--;
    my_print(SM, file, "RD %d: get hitched\n", rdID);
    if (*SM->rdCnt == 0)
    {
        sem_post(SM->santaRdSem);
    }
    (*SM->processCnt)--;
    if (*SM->processCnt == 0)
    {
        sem_post(SM->mainSem);
    }
    sem_post(SM->mutexSem);
}

void my_print(shared *SM, FILE *file, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    fprintf(file, "%d: ", *SM->lineCnt);
    vfprintf(file, format, args);
    (*SM->lineCnt)++;
    fflush(file);

    va_end(args);
}

void memClean(shared *SM, FILE *file)
{
    fclose(file);
    sem_destroy(SM->mainSem);
    sem_destroy(SM->mutexSem);
    sem_destroy(SM->elfSem);
    sem_destroy(SM->rdSem);
    sem_destroy(SM->santaSem);
    sem_destroy(SM->santaElfSem);
    sem_destroy(SM->santaRdSem);
    munmap(SM->mainSem, sizeof(sem_t));
    munmap(SM->elfSem, sizeof(sem_t));
    munmap(SM->mutexSem, sizeof(sem_t));
    munmap(SM->rdSem, sizeof(sem_t));
    munmap(SM->santaSem, sizeof(sem_t));
    munmap(SM->santaElfSem, sizeof(sem_t));
    munmap(SM->santaRdSem, sizeof(sem_t));
    munmap(SM->processCnt, sizeof(int));
    munmap(SM->lineCnt, sizeof(int));
    munmap(SM->elfLine, sizeof(int));
    munmap(SM->rdCnt, sizeof(int));
    munmap(SM->elfHelped, sizeof(int));
    munmap(SM->xmas, sizeof(bool));
}

void semOpener(shared *SM)
{
    for (int i = 0; i < *SM->processCnt; i++)
    {
        sem_post(SM->mainSem);
        sem_post(SM->mutexSem);
        sem_post(SM->elfSem);
        sem_post(SM->rdSem);
        sem_post(SM->santaSem);
        sem_post(SM->santaRdSem);
        sem_post(SM->santaElfSem);
    }
}

bool processArgs(char **argv, reindeers *RD, elves *EF)
{
    char *endPtr;
    EF->cnt = strtol(argv[1], &endPtr, 10);
    if (*endPtr != '\0')
    {
        fprintf(stderr, "ERROR - could not convert \"%s\" to integer\nPlease enter integer\n", argv[1]);
        return false;
    }

    if (EF->cnt <= 0 || EF->cnt >= 1000)
    {
        fprintf(stderr, "ERROR - elf counter must be from interval (0, 1000)\n");
        return false;
    }

    EF->time = strtol(argv[3], &endPtr, 10);
    if (*endPtr != '\0')
    {
        fprintf(stderr, "ERROR - could not convert \"%s\" to integer\nPlease enter integer\n", argv[3]);
        return false;
    }

    if (EF->time < 0 || EF->time > 1000)
    {
        fprintf(stderr, "ERROR - elf time must be from interval <0,1000>\n");
        return false;
    }

    RD->cnt = strtol(argv[2], &endPtr, 10);
    if (*endPtr != '\0')
    {
        fprintf(stderr, "ERROR - could not convert \"%s\" to integer\nPlease enter integer\n", argv[2]);
        return false;
    }
    if (RD->cnt <= 0 || RD->cnt >= 20)
    {
        fprintf(stderr, "ERROR - reindeer counter must be from interval (0,20)\n");
        return false;
    }

    RD->time = strtol(argv[4], &endPtr, 10);
    if (*endPtr != '\0')
    {
        fprintf(stderr, "ERROR - could not convert \"%s\" to integer\nPlease enter integer\n", argv[4]);
        return false;
    }

    if (RD->time < 0 || RD->time > 1000)
    {
        fprintf(stderr, "ERROR - reindeer time must be from interval <0,1000>\n");
        return false;
    }
    return true;
}
