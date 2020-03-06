#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <windows.h>
#include <time.h>

#define MAX_Client 10
#define MAX_Buffer 512

typedef enum clientstatus
{
    FREE,
    AUTH,
    BUSY,
    REUSABLE
} CLIENTSTATUS;

SOCKET mainSock;
SOCKET ClientSock[MAX_Client];
HANDLE ThreadServer;
DWORD ThreadIdServer;
HANDLE ThreadClient[MAX_Client];
DWORD ThreadIdClient[MAX_Client];
CLIENTSTATUS status_thr_sock[MAX_Client];

char buffer[512];
int Stop_sever = 0;

int setupSocketMain(int port);
int receiveFrom(SOCKET *sock, char *bufferFrom);
int sendTo(SOCKET *sock, char *data);
int sendToAll(char *nikc, char *temp_buffer, int not);
void get_time();
void printLogo();
DWORD WINAPI Thread_Client(void *data);
DWORD WINAPI Thread_Sever(void *data);

/*------------------------------- MAIN -----------------------------*/
int main(int argc, char *argv[])
{
    printLogo();
    ThreadServer = CreateThread(NULL, 0, Thread_Sever, NULL, 0x00000004, &ThreadIdServer);
    if (ThreadServer != NULL)
    {
        get_time();
        printf("[  INFO ] ID Thread Server: %lu\n", ThreadServer);
        ResumeThread(ThreadServer);
    }

    WaitForSingleObject(ThreadServer, INFINITE);
    WaitForMultipleObjects(0x0000000A, ThreadClient, TRUE, INFINITE);
    for (int i = 0; i < 10; i++)
    {
        CloseHandle(ThreadClient[i]);
    }
    CloseHandle(ThreadServer);
    system("pause");
    return 0;
}

/*------------------------ Create Thread Server --------------------*/
DWORD WINAPI Thread_Sever(void *data)
{
    int retrn = 0;
    int listPort = -1;
    char userIn[50];
    int round = 0;
    //int pt1 = 0;

    get_time();
    printf("[  INFO ] Start Server Manager\n");
    do
    {
        get_time();
        printf("[  GET  ] Gimme the port for wait the connections from clients: ");
        memset(userIn, 0x00, sizeof(userIn));
        gets(userIn);
        listPort = atoi(userIn);
        if (listPort <= 0 || listPort >= 65536)
        {
            get_time();
            printf("[WARNING] Port not valid! Retry\n");
        }
        else if (listPort <= 1024)
        {
            get_time();
            printf("[WARNING] Ports under 1024 aren't usable! Retry\n");
        }
    } while (listPort <= 1024 || listPort >= 65536);

    get_time();
    printf("[  INFO ] Local Machine IP Adrees: INADDR_ANY\n");
    retrn = setupSocketMain(listPort);

    if (retrn == 0)
    {
        do
        {
            for (int x = 0; x < MAX_Client; x++)
            {
                if (status_thr_sock[x] == FREE || status_thr_sock[x] == REUSABLE)
                {
                    struct sockaddr_in Client_info = {0};
                    int addrsize = sizeof(Client_info);
                    if ((ClientSock[x] = accept(mainSock, (struct sockaddr *)&Client_info, &addrsize)) == SOCKET_ERROR)
                    {
                        /*
                        if (pt1 == 0)
                        {
                            get_time();
                            printf("[ ERROR ] Failed Accept() with error: %d\n", WSAGetLastError());
                        }
                        */
                        get_time();
                        printf("[ ERROR ] Failed Accept() with error: %d\n", WSAGetLastError());
                    }
                    else
                    {
                        // pt1 = 1;
                        get_time();
                        getpeername(ClientSock[x], (struct sockaddr *)&Client_info, &addrsize);
                        printf("[  INFO ] Client Connect IP: %s:%d\n", inet_ntoa(Client_info.sin_addr), ntohs(Client_info.sin_port));

                        ThreadClient[x] = CreateThread(NULL, 0, Thread_Client, (void *)x, 0x00000004, &ThreadIdClient[x]);
                        if (ThreadClient[x] != NULL)
                        {
                            get_time();
                            printf("[  INFO ] ID Thread Clinet %i: %lu\n", x, ThreadIdClient[x]);
                            ResumeThread(ThreadClient[x]);
                        }
                    }
                }
            }

        } while (Stop_sever == 0);
    }

    closesocket(mainSock);
    WSACleanup();
    get_time();
    printf("[  INFO ] Stop Server Manager\n\n");
    return retrn;
}

/*----------------------- Create Thread Client ---------------------*/
DWORD WINAPI Thread_Client(void *data)
{
    int Indice = (int)data;
    int session_exit = 0;
    char nickname[30];
    char marc[] = ": ";
    char buffer_thr[MAX_Buffer];
    char message_send[MAX_Buffer];
    memset(buffer_thr, 0x00, 512);
    memset(message_send, 0x00, 512);
    status_thr_sock[Indice] = AUTH;

    if (send(ClientSock[Indice], "/getnick", strlen("/getnick"), 0x00) == SOCKET_ERROR)
    {
        get_time();
        printf("[ ERROR ] Failed send(ClientSock[%i], \"/getnick\") with error: %d\n", Indice, WSAGetLastError());
        memset(&buffer_thr, 0x00, 512);
        return 1;
    }
    else
    {
        recv(ClientSock[Indice], buffer_thr, MAX_Buffer, 0);
        strcpy(nickname, buffer_thr);
        get_time();
        printf("[  INFO ] Client %i Nickname: %s\n", Indice, nickname);
        strcat(nickname, marc);

        if (send(ClientSock[Indice], "Welcom To ChatRoom...", strlen("Welcom To ChatRoom..."), 0x00) == SOCKET_ERROR)
        {
            get_time();
            printf("[ ERROR ] Failed send(ClientSock[%i], \"Welcom To ChatRoom...\") with error: %d\n", Indice, WSAGetLastError());
            memset(&buffer_thr, 0x00, 512);
        }
        else
        {
            memset(buffer_thr, 0x00, 512);
            status_thr_sock[Indice] = BUSY;

            do
            {
                int session = recv(ClientSock[Indice], buffer_thr, MAX_Buffer, 0);
                if (session > 0)
                {
                    get_time();
                    printf("[ DEBUG ] Server Receve: %s\n", buffer_thr);
                    if (strcmp(buffer_thr, "/exit") == 0)
                    {
                        get_time();
                        printf("[ DEBUG ] Connection closed\n");
                        session_exit = 1;
                    }
                    else
                    {

                        strcat(message_send, nickname);
                        strcat(message_send, buffer_thr);
                        for (int i = 0; i < 10; i++)
                        {
                            if (i != Indice)
                            {
                                if (status_thr_sock[i] == BUSY)
                                {
                                    send(ClientSock[i], buffer_thr, strlen(buffer_thr), 0x00);
                                }
                            }
                        }
                    }
                }
                else if (session == 0)
                {
                    get_time();
                    printf("[ DEBUG ] Connection closed\n");
                    session_exit = 1;
                }

                memset(message_send, 0x00, 512);
                memset(buffer_thr, 0x00, 512);
            } while (Stop_sever == 0 && session_exit == 0);
        }
    }
    closesocket(ClientSock[Indice]);
    WSACleanup();
    get_time();
    printf("[  INFO ] Close Socket Client %i\n\n", Indice);
    status_thr_sock[Indice] = REUSABLE;
    return 0;
}

/*----------------------- Setup Socket Main ------------------------*/
int setupSocketMain(int port)
{
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
        printf("[ ERROR ] Failed main Socket creation!\n");
        perror("socket");
        return 2;
    }
    get_time();
    printf("[  OK   ] Socket created!\n");

    config.sin_family = AF_INET;
    config.sin_addr.s_addr = INADDR_ANY;
    config.sin_port = htons(port);

    if (bind(mainSock, (struct sockaddr *)&config, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
    {
        get_time();
        printf("[ ERROR ] Failed main Socket binding! %d\n", WSAGetLastError());
        return 2;
    }
    get_time();
    printf("[  OK   ] Bind Success!\n");

    if (listen(mainSock, 10) == SOCKET_ERROR)
    {
        get_time();
        printf("[ ERROR ] Failed to open listen port!\n");
        perror("listen");
        return 2;
    }
    get_time();
    printf("[  INFO ] Listening Port Open!\n");

    return 0;
}

/*---------------------------- GET TIME ----------------------------*/
void get_time()
{
    char s[100];
    time_t t = time(NULL);
    struct tm *tp = localtime(&t);
    strftime(s, 100, "%H:%M:%S", tp);
    printf("%s ", s);
}

/*---------------------------- Print Logo --------------------------*/
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
    printf("                                         Server Master\n");
    printf("_______________________________________________________________________________________________________________\n\n");
}