#include <stdio.h>
#include <linux/input.h>

// Структура input_event
// struct input_event {
// #if (__BITS_PER_LONG != 32 || !defined(__USE_TIME_BITS64)) && !defined(__KERNEL__)
// 	struct timeval time;
// #define input_event_sec time.tv_sec
// #define input_event_usec time.tv_usec
// 	__u16 type;
// 	__u16 code;
// 	__s32 value;
// };

int main()
{
    FILE *fd = fopen("/dev/input/event4", "rb");
    struct input_event events[6];
    while(1)
    {
        fread(&events, sizeof(struct input_event), 6, fd);
        for(int i = 0; i < 6; i++)
            printf("---%d---\nTime: %ld\nType: %d\n Code: %d\nValue: %d\n",i, events[i].time.tv_sec, events[i].type, events[i].code, events[i].value);
    }
    return 0;
}