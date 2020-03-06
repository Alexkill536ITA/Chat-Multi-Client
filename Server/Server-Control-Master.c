#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <string.h>
#include <windows.h>
#include <time.h>

SOCKET mainSock;
char buffer[512];
int retrn;

int receiveFrom(SOCKET *sock, char *buffer);
int sendTo(SOCKET *sock, char *data);
void get_time();
void printLogo();

int main(int argc, char *argv[])
{
    unsigned long ipServer;
    int portServer;
    int recvMsgSize;
    char userIn[512];

    printLogo();
    do
    {
        get_time();
        printf("[  OK   ] Enter You Master Control Chat Room\n");
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
    }
    else
    {
        get_time();
        printf("[  OK   ] Connected!\n");
        do
        {
            system("cls");
            printLogo();
            printf("#-----------------------------------#\n");
            printf("|   1 - List All Client Connect     |\n");
            printf("|   2 - Send Message To Client      |\n");
            printf("|   3 - Kill Connection Client      |\n");
            printf("|   4 - Kill All Connection Client  |\n");
            printf("|   5 - Close To Server Chat Room   |\n");
            printf("|   6 - Close Control Chat Room     |\n");
            printf("#-----------------------------------#\n\n");
            printf("> ");
            gets(buffer);

            if (strcmp(buffer, "1") == 0)
            {
                if ((retrn = sendTo(&mainSock, "/list")) != 0)
                    ;
                {
                    get_time();
                    printf("[ ERRORE ] Failed receiveFrom(&mainSock, '/list') with error: %d\n", retrn);
                    break;
                }
            }

            if (strcmp(buffer, "2") == 0)
            {
                memset(userIn, 0x00, sizeof(userIn));
                printf("Select Client Send Message: ");
                gets(userIn);
                memset(buffer, 0x00, sizeof(buffer));
                printf("Message Text: \n");
                gets(buffer);
                if ((retrn = sendTo(&mainSock, buffer)) != 0)
                {
                    get_time();
                    printf("[ ERRORE ] Failed receiveFrom(&mainSock, '/Message') with error: %d\n", retrn);
                    break;
                }
            }

            if (strcmp(buffer, "3") == 0)
            {
                memset(userIn, 0x00, sizeof(userIn));
                printf("Select Client To Kill: ");
                gets(userIn);
                if ((retrn = sendTo(&mainSock, userIn)) != 0)
                {
                    get_time();
                    printf("[ ERRORE ] Failed receiveFrom(&mainSock, '/Kill User') with error: %d\n", retrn);
                    break;
                }
            }

            if (strcmp(buffer, "4") == 0)
            {
                memset(userIn, 0x00, sizeof(userIn));
                printf("Are You Sure Kill All Client [Y/n]: ");
                gets(userIn);
                if (userIn[0] == '\n' || userIn[0] == '\r')
                {
                    userIn[0] = 'Y';
                }
                if (userIn[0] == 'y' && userIn[0] == 'Y')
                {
                    if ((retrn = sendTo(&mainSock, userIn)) != 0)
                    {
                        get_time();
                        printf("[ ERRORE ] Failed receiveFrom(&mainSock, '/Kill All User') with error: %d\n", retrn);
                        break;
                    }
                }
            }



            if (send(mainSock, buffer, sizeof(buffer), 0) < 0)
            {
                printf("[ERRORE] send() Failed %u\n", WSAGetLastError());
                closesocket(mainSock);
                WSACleanup();
                exit(1);
            }

            /* Receive Message from Server */
            //echoBuffer[BufferSize] = "/0";
            if ((recvMsgSize = recv(mainSock, buffer, sizeof(buffer), 0)) < 0)
            {
                printf("[ERRORE] recv() 1 Failed %u\n", WSAGetLastError());
                closesocket(mainSock);
                WSACleanup();
                exit(1);
            }
            else
                printf("[......] FROM SERVER %d: %s\n", buffer);

            if (strncmp("exit", buffer, 4) == 0)
            {
                printf("[ INFO ] BYE BYE From Server \n");
                closesocket(mainSock);
                WSACleanup();
                exit(1);
            }
        } while (strcmp(buffer, "Exit") != 0 && strcmp(buffer, "exit") != 0);
    }

    closesocket(mainSock);
    WSACleanup();
    printf("\n");
    system("pause");
    return 0;
}

/*----------------------- Receive / send ---------------------------*/
int receiveFrom(SOCKET *sock, char *buffer)
{
    int countRecv = recv(*sock, buffer, 512, 0);
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
    printf("                                    Server Control Master\n");
    printf("_______________________________________________________________________________________________________________\n\n");
}