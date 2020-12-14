#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

#include <sys/time.h>
#include <linux/input.h>

static short verbose = 0;

void start(char *file);
void choose_event(char *filename);

int main(int argc, char** argv)
{
    // show help message
    if(argv[1] != NULL && !strncmp(argv[1], "-h", 2))
    {
        printf("\e[1;33mUsage:\e[0m\n\t\e[1;33mPress F12\e[0m - start/stop capturing\n\t\e[1;33m-v\e[0m - enable verbose mode for sysadmins :)\n");
        exit(EXIT_SUCCESS);
    }

    if(argv[1] != NULL && !strncmp(argv[1], "-v", 2))
    {
        puts("\e[1;32m[+]\e[0m Verbose mode enabled!\n");
        verbose = !verbose;
    }

    // check prvilige for read file /dev/input/eventX
    int real = getuid();
    if(real != 0)
    {
        printf("\e[1;31m[-]\e[0m You should be \e[1;31mroot\e[0m for run this program!\n");
        exit(EXIT_SUCCESS);
    }

    char filename[256];
    memset(filename, 0, 256);
    choose_event(filename);
    start(filename);

    return 0;
}

void start(char *file)
{
    // prepare for analyze
    chdir("/dev/input/by-path");
    FILE* fd = fopen(file, "rb");
    if(fd == NULL)
    {
        puts("\e[1;31m[-]\e[0m Can't access to system dir: /dev/input/by-path");
        exit(EXIT_FAILURE);
    }
    struct input_event events[6];
    struct timeval times[2];
    int check = 0;

    // counter for symbols
    long int symbols = -1;
    float result = 0.0;

    puts("\e[1;35m[+]\e[0m Press F12 to start");

    while(1)
    {
        fread(&events, sizeof(struct input_event), 6, fd);

        if(events[4].code == 70)
            check++;

        // if input F12 start recording
        if(events[4].code == 70 && (check % 2))
        {
            puts("\n\e[1;32m[+]\e[0m Start analyzing!");
            times[0].tv_sec = events[4].time.tv_sec;

            if(verbose)
                printf("\e[1;35m[+]\e[0m Begin time: %ld\n", times[0].tv_sec);
        }

        if(check % 2)
        {
            symbols += 1;
            if(verbose)
                printf("\e[1;35m[+]\e[0m Pressed keys: %ld\n", symbols);
        }

        // if F12 pressed again stop analyze
        if(events[4].code == 70 && !(check % 2))
        {
            puts("\n\e[1;32m[+]\e[0m Stop analyzing!");

            times[1].tv_sec = events[4].time.tv_sec;
            // verbose output
            if(verbose)
                printf("\e[1;35m[+]\e[0m Stop time: %ld\n", times[1].tv_sec);

            long amount_time = times[1].tv_sec - times[0].tv_sec;
            printf("\e[1;32m[+]\e[0m Amount time: %ld\n", amount_time);

            result = (float)symbols / amount_time;
            printf("\e[1;32mRESULT:\e[0m \e[1;33m%.2f\e[0m symbols/second\n", result);

            // symbols to 0 if user want test again
            symbols = 0;
        }
    }
}

void choose_event(char *filename)
{
    // read dir
    DIR *dp;
    struct dirent *file;
    dp = opendir("/dev/input/by-path");
    short index = 0;

    // allocate memory for keyboard list
    char *files[5];
    for(int i = 0; i < 5; i++)
        files[i] = (char *)calloc(256, sizeof(char));

    puts("\e[1;32m[+]\e[0m Keyboards list:");
    if(dp != NULL)
    {
        while((file = readdir(dp)) != NULL)
        {
            if(strstr(file->d_name, "kbd") != NULL)
            {
                printf("\t\e[1;33m[%d]\e[0m %s\n", index, file->d_name);
                strncpy(files[index++], file->d_name, strlen(file->d_name));
            }
        }
        closedir(dp);
    }
    
    // get user input
    short int choice;
    printf("\e[1;32m[+]\e[0m Choose keyboard index: ");
    scanf("%1hd", &choice);
    strncpy(filename, files[choice], strlen(files[choice]));

    // free the memory
    for(int i = 0; i < 5; i++)
        free(files[i]);
}
