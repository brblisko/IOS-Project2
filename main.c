#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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

bool processArgs(char **argv, reindeers *RD, elfs *EF);

int main(int argc, char **argv)
{
    reindeers RD;
    elfs EF;

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
    return 0;
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
