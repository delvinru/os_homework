#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <time.h>

static int x_center = 0;
static int y_center = 0;
static int R = 100;

void draw_circle(char *fb)
{
}

int main()
{
    int fd, bpp, bsize;
    struct fb_var_screeninfo info;
    char *fb;

    if(0 > (fd = open("/dev/fb0", O_RDWR)))
    {
        perror("open");
        return -1;
    }

    if ( (-1) == (ioctl(fd, FBIOGET_VSCREENINFO, &info)))
    {
        perror("ioctl");
        goto err;
    }
    bpp = info.bits_per_pixel >> 3;
    printf("xres = %d\n", info.xres);
    printf("yres = %d\n", info.yres);
    printf("bpp = %d\n", bpp);

    // If something wrong need align xres
    // For example, if 8 + 7 ) &(~7)
    bsize = info.xres * info.yres * bpp;

    if ( MAP_FAILED == ( fb = mmap(NULL, bsize, PROT_WRITE, MAP_SHARED, fd, 0)))
    {
        perror("mmap");
        goto err;
    }

    int R = 100;
    int mvy = 10;
    int mvx = 10;
    x_center = info.xres / 2;
    y_center = info.yres / 2;

     

    while(1)
    {
        memset((void*)fb, 0, bsize);
        
        x_center += mvx;
        y_center += mvy;

        if((y_center + R) >= info.yres)
            mvy = -10;

        if((x_center + R) >= info.xres)
            mvx = -10;
        
        if((y_center - R) <= 0)
            mvy = 10;

        if((x_center - R) <= 0)
            mvx = 10;

        // Draw circle
        for (int y = 0 ; y < info.yres; y++)
        {
            for(int x = 0; x < info.xres; x++)
            {
                if ( ((x - x_center)*(x - x_center) + (y - y_center)*(y - y_center)) < R*R )
                {
                
                    // Offset
                    //memset(fb + ((y * info.xres) + x) * bpp, (x*y)&0xff, bpp);
                    //*((unsigned int*)(fb + ((y * info.xres) + x) * bpp)) = (0xFFFFFFFF);
                    *((unsigned int*)(fb + ((y * info.xres) + x) * bpp)) = (x^y);
                }
            }
        }
        usleep(150000);
    }

err:
    close(fd);
    return 0;
}
