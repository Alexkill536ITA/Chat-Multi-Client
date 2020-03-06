#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <string.h>
#include <windows.h>
#include <time.h>

SOCKET mainSock;
HANDLE threadMain;
DWORD threadIdMain;
char buffer[512];
char bufferRecv[512];
char name[30];
int retrn;
int Ready_write = 0;

int receiveFrom(SOCKET *sock, char *buffer);
int sendTo(SOCKET *sock, char *data);
void get_time();
void printLogo();

DWORD WINAPI Thread_Main(void *data)
{
    unsigned long ipServer;
    int portServer;
    int recvMsgSize;
    char userIn[512];
    int sret;
    int fd = 0;

    fd_set recvMsgTime;
    struct timeval timeout;

    printLogo();
    do
    {
        do
        {
            printf("Enter Nikname [MAX 30 CHAR]: ");
            gets(name);
            printf("\n");
        } while (strlen(name) <= 0 || strlen(name) >= 30);
        printf("Gimme the IP of server: ");
        memset(userIn, 0x00, sizeof(userIn));
        gets(userIn);
        ipServer = inet_addr(userIn);
        char tmp[512];
        memcpy(tmp, userIn, sizeof(userIn));
        do
        {
            printf("Gimme the Port of server: ");
            memset(userIn, 0x00, sizeof(userIn));
            gets(userIn);
            portServer = atoi(userIn);
            if (portServer <= 0 || portServer >= 65536)
                printf("Port not valid! Retry\n");
        } while (portServer <= 0 || portServer >= 65536);
        printf("Server IP: %s\nServer Port: %d\nContinue? [Y/n] ", tmp, portServer);
        gets(userIn);
        if (userIn[0] == '\n' || userIn[0] == '\r')
            userIn[0] = 'Y';
    } while (userIn[1] == '\0' && userIn[0] != 'Y' && userIn[0] != 'y');

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

    mainSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (mainSock == INVALID_SOCKET)
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

    if ((retrn = connect(mainSock, (struct sockaddr *)&config, sizeof(struct sockaddr_in))) == SOCKET_ERROR)
    {
        get_time();
        printf("[ ERROR ] Failed Connect() with error: %d\n", WSAGetLastError());
        closesocket(mainSock);
        WSACleanup();
        exit(1);
    }
    else
    {
        if ((recvMsgSize = recv(mainSock, bufferRecv, sizeof(bufferRecv), 0)) < 0)
        {
            get_time();
            printf("[ERRORE] recv() 1 Failed %u\n", WSAGetLastError());
            closesocket(mainSock);
            WSACleanup();
            exit(1);
        }
        else
        {
            if (strcmp(bufferRecv, "/getnick") == 0)
            {
                if (sendTo(&mainSock, name) != 0)
                {
                    get_time();
                    printf("[ ERROR ] Failed sendTo(CmainSock, name) with error: %d\n", WSAGetLastError());
                    closesocket(mainSock);
                    WSACleanup();
                    exit(1);
                }
                else
                {
                    if ((recvMsgSize = recv(mainSock, bufferRecv, sizeof(bufferRecv), 0)) < 0)
                    {
                        get_time();
                        printf("[ERRORE] recv() 1 Failed %u\n", WSAGetLastError());
                        closesocket(mainSock);
                        WSACleanup();
                        exit(1);
                    }
                    else
                    {
                        get_time();
                        printf("[  OK   ] Connected!\n");
                        system("cls");
                        printLogo();
                        printf("%s\n", bufferRecv);
                        memset(bufferRecv, 0, 512);
                        do
                        {
                            /* Select */
                            FD_ZERO(&recvMsgTime);
                            FD_SET(fd, &recvMsgTime);

                            timeout.tv_sec = 0;
                            timeout.tv_usec = 500;

                            sret = select(0, &recvMsgTime, NULL, NULL, &timeout);


                            if (sret == 0)
                            {
                                if ((recvMsgSize = recv(mainSock, bufferRecv, sizeof(bufferRecv), 0)) < 0)
                                {
                                    get_time();
                                    printf("[ERRORE] recv() 1 Failed %u\n", WSAGetLastError());
                                    memset(bufferRecv, 0, 512);
                                }
                                else
                                {
                                    get_time();
                                    printf("%s\n", bufferRecv);
                                    memset(bufferRecv, 0, 512);
                                }
                            }
                            else
                            {
                                memset(buffer, 0, 512);
                                printf("\n> ");
                                gets(buffer);
                                if (sendTo(&mainSock, buffer) != 0)
                                {
                                    get_time();
                                    printf("[ ERROR ] Failed sendTo(CmainSock, name) with error: %d\n", WSAGetLastError());
                                    memset(buffer, 0, 512);
                                }
                                else
                                {
                                    get_time();
                                    printf("%s: %s", name, buffer);
                                    memset(buffer, 0, 512);
                                }
                            }
                        } while (strcmp(buffer, "/Exit") != 0 && strcmp(buffer, "/exit") != 0);
                    }
                }
            }
        }
        closesocket(mainSock);
        WSACleanup();
        printf("\n");
        return 0;
    }
}

int main(int argc, char *argv[])
{
    printLogo();
    threadMain = CreateThread(NULL, 0, Thread_Main, NULL, 0x00000004, &threadIdMain);
    if (threadMain != NULL)
    {
        get_time();
        printf("[  INFO ] ID Thread Server: %lu\n", threadIdMain);
        ResumeThread(threadMain);
    }

    /*    do
    {
        if (Ready_write == 1)
        {
            printf("> ");
            gets(buffer);
        }
    } while (strcmp(buffer, "/Exit") != 0 && strcmp(buffer, "/exit") != 0);*/

    WaitForSingleObject(threadMain, INFINITE);
    CloseHandle(threadMain);
    system("pause");
    return 0;
}

/*----------------------- Receive / send ---------------------------*/
int receiveFrom(SOCKET *sock, char *bufferR)
{
    int countRecv = recv(*sock, bufferR, 512, 0);
    if (countRecv > 0)
    {
        //printf("[DEBUG] Received: %s\n", buffer);
        return 0;
    }
    else if (countRecv == 0)
    {
        printf("[DEBUG] Connection closed\n");
        return -1;
    }
    else
    {
        int err = WSAGetLastError();
        printf("[DEBUG] recv failed: %d\n", err);
        return err;
    }
}
int sendTo(SOCKET *sock, char *data)
{
    if (send(*sock, data, strlen(data), 0x00) == SOCKET_ERROR)
        return 1;
    return 0;
}

/*---------------------------- GET TIME ----------------------------*/
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