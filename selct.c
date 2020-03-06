#include <stdio.h>
#include <time.h>
#include <winsock2.h>
#include <windows.h>

int main(void)
{
    fd_set rfds;
    struct timeval timeout;
    int retval;
    int fd = 0;
    int ret;
    char buf[11];

    while (1)
    {
        /* Watch stdin (fd 0) to see when it has input. */
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        /* Wait up to five seconds. */
        timeout.tv_sec = 2;
        timeout.tv_usec = 0;

        /* code */

        retval = select(8, &rfds, NULL, NULL, &timeout);
        /* Don’t rely on the value of tv now! */

        if (retval < 0)
        {
            perror("select()");
            break;
        }
        else if (retval == 0)
        {
            printf("sret = %d\n", retval);
            printf("Timeout\n");
        }
        else
        {
            printf("sret = %d\n", retval);
            memset((void *) buf, 0, 11);
            ret = read(fd, (void *) buf, 10);
            printf("ret = %d\n", ret);
            if (ret != -1)
            {
                printf("buf = %s\n", buf);
            }
            
        }
    }
    system("pause");
    return 0;
}