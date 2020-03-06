#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <windows.h>
#include <time.h>

#define MAX_Buffer 512

SOCKET MainSock;
HANDLE ThreadMain;
DWORD ThreadIdMain;
char Nickname[30];
char BufferRecv[MAX_Buffer];
char Buffer[MAX_Buffer];
int retrn;

/*---------------------------- MAIN --------------------------------*/
int main(int argc, char *argv[])
{
    printLogo();
    ThreadMain = CreateThread(NULL, 0, Thread_Main, NULL, 0x00000004, &ThreadIdMain);
    if (ThreadMain != NULL)
    {
        get_time();
        printf("[  INFO ] ID Thread Server: %lu\n", ThreadIdMain);
        ResumeThread(ThreadMain);
    }

    WaitForSingleObject(ThreadMain, INFINITE);
    CloseHandle(ThreadMain);
    system("pause");
    return 0;
}

DWORD WINAPI Thread_Main(void *data)
{
    unsigned long ipServer;
    char UserIn[MAX_Buffer];
    char tmp[MAX_Buffer];
    int portServer;

    fd_set recvMsgTime;
    struct timeval timeout;
    int fd = 0;
    int sret = 0;

    printLogo();
    do
    {
        do
        {
            printf("Enter Nickname [MAX 30 CHAR]: ");
            gets(Nickname);
            printf("\n");
        } while (strlen(Nickname) <= 0 || strlen(Nickname) >= 30);
        printf("Gimme the IP of Server: ");
        memset(UserIn, 0x00, sizeof(UserIn));
        gets(UserIn);
        ipServer = inet_addr(UserIn);
        memcpy(tmp, UserIn, sizeof(UserIn));
        do
        {
            printf("Gimme the IP of Server: ");
            memset(UserIn, 0x00, sizeof(UserIn));
            gets(UserIn);
            portServer = atoi(UserIn);
            if (portServer <= 0 || portServer >= 65535)
            {
                printf("Port not valid! Retry\n");
            }
        } while (portServer <= 0 || portServer >= 65536);
        printf("Server IP: %s\nServer Port: %d\nContinue? [Y/n] ", tmp, portServer);
        gets(UserIn);
        if (UserIn[0] == '\n' || UserIn[0] == '\r')
        {
            UserIn[0] = 'Y';
        }
    } while (UserIn[1] == '\0' && UserIn[0] != 'Y' && UserIn[0] != 'y');

    struct sockaddr_in config;
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        get_time();
        printf("[ ERROR ] Failed WSAStartup() with error: %d\n", WSAGetLastError());
        return 1;
    }
    get_time();
    printf("[  OK   ] WSAStartup Complete!\n");

    memset(&config, 0x00, sizeof(struct sockaddr_in));

    MainSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (MainSock == INVALID_SOCKET)
    {
        get_time();
        printf("[ ERROR ] Failed main socket creation!\n");
        perror("socket");
        return 2;
    }
    get_time();
    printf("[  OK   ] Socket created!\n");

    config.sin_family = AF_INET;
    config.sin_addr.s_addr = ipServer;
    config.sin_port = htons(portServer);

    if ((retrn = connect(MainSock, (struct sockaddr *)&config, sizeof(struct sockaddr_in))) == SOCKET_ERROR)
    {
        get_time();
        printf("[ ERROR ] Failed Connect() with error: %d\n", WSAGetLastError());
        closesocket(MainSock);
        WSACleanup();
        exit(1);
    }
    else
    {
        memset(BufferRecv, 0, 512);
        recv(MainSock, BufferRecv, sizeof(BufferRecv), 0);
        if (strcmp(BufferRecv, "/getnick") == 0)
        {
            if (send(MainSock, Nickname, strlen(Nickname), 0x00) == SOCKET_ERROR)
            {
                get_time();
                printf("[ ERROR ] Failed sendTo(MainSock, Nickname) with error: %d\n", WSAGetLastError());
                closesocket(MainSock);
                WSACleanup();
                exit(1);
            }
            else
            {
                recv(MainSock, BufferRecv, sizeof(BufferRecv), 0);
                get_time();
                printf("[  OK   ] Connected!\n");
                system("cls");
                printLogo();
                printf("%s\n", BufferRecv);
                memset(BufferRecv, 0, 512);
                do
                {
                    FD_ZERO(&recvMsgTime);
                    FD_SET(fd, &recvMsgTime);

                    timeout.tv_sec = 0;
                    timeout.tv_usec = 500;

                    sret = select(1, &recvMsgTime, NULL, NULL, &timeout);

                    if (sret == )
                    {
                        /* code */
                    }
                    


                } while (strcmp(Buffer, "/Exit") != 0 && strcmp(Buffer, "/exit") != 0);
            }
        }
    }
}

/*---------------------------- GET TIME   ---------------------------*/
void get_time()
{
    char s[100];
    int dim;
    time_t t = time(NULL);
    struct tm *tp = localtime(&t);

    dim = strftime(s, 100, "%H:%M:%S", tp);
    printf("%s ", s);
}

/*---------------------------- Print Logo ---------------------------*/
void printLogo()
{
    system("cls");
    printf("_______________________________________________________________________________________________________________\n\n");
    printf("   /$$$$$$  /$$                   /$$           /$$$$$$$                                   \n");
    printf("  /$$__  $$| $$                  | $$          | $$__  $$                                  \n");
    printf(" | $$  \\__/| $$$$$$$   /$$$$$$  /$$$$$$        | $$  \\ $$  /$$$$$$   /$$$$$$  /$$$$$$/$$$$ \n");
    printf(" | $$      | $$__  $$ |____  $$|_  $$_/        | $$$$$$$/ /$$__  $$ /$$__  $$| $$_  $$_  $$\n");
    printf(" | $$      | $$  \\ $$  /$$$$$$$  | $$          | $$__  $$| $$  \\ $$| $$  \\ $$| $$ \\ $$ \\ $$\n");
    printf(" | $$    $$| $$  | $$ /$$__  $$  | $$ /$$      | $$  \\ $$| $$  | $$| $$  | $$| $$ | $$ | $$\n");
    printf(" |  $$$$$$/| $$  | $$|  $$$$$$$  |  $$$$/      | $$  | $$|  $$$$$$/|  $$$$$$/| $$ | $$ | $$\n");
    printf("  \\______/ |__/  |__/ \\_______/   \\___/        |__/  |__/ \\______/  \\______/ |__/ |__/ |__/ By Alexkill536ITA\n\n");
    printf("_______________________________________________________________________________________________________________\n\n");
}