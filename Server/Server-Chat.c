#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <string.h>
#include <windows.h>
#include <time.h>

#define MAX_Client 10
#define Max_Buffer 512

typedef enum clientstatus
{
    FREE,
    AUTH,
    BUSY,
    REUSABLE
} CLIENTSTATUS;

SOCKET mainSock;
SOCKET ClientSock[MAX_Client];
HANDLE threadServer;
DWORD threadIdServer;
HANDLE threadClients[MAX_Client];
DWORD threadIdClients[MAX_Client];
CLIENTSTATUS status_thr_sock[MAX_Client];

char buffer[512];
int list[255];
char IPserver[255][16];
char select_ip;
//int ClientList[MAX_Client];
//char LENGTH_SEND[512];
int Stop_sever = 0;

int setupSocketMain(int port);
int receiveFrom(SOCKET *sock, char *bufferFrom);
int sendTo(SOCKET *sock, char *data);
void sendToAll(char *nikc, char *tmp_buffer, int not);
char *BinIP2StrIP(long lnIP);
void GetIpAddress();
void get_time();
void printLogo();
DWORD WINAPI Thread_Client(void *data);

/*------------------------ Create Thread Server --------------------*/
DWORD WINAPI Thread_Server(void *data)
{
    int retrn = 0;
    int listPort = -1;
    memset(&buffer, 0x00, 512);
    char userIn[50];
    int round = 0;

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

    GetIpAddress();
    get_time();
    printf("[  INFO ] Local Machine IP Adrees: %s\n", IPserver);
    retrn = setupSocketMain(listPort);

    for (int y = 0; y < MAX_Client; y++)
    {
        status_thr_sock[y] = FREE;
    }

    /* Connect Client */

    if (retrn == 0)
    {
        do
        {
            for (int x = 0; x < MAX_Client; x++)
            {
                if (status_thr_sock[x] == FREE || status_thr_sock[x] == REUSABLE)
                {
                    struct sockaddr_in client_info = {0};
                    int addrsize = sizeof(client_info);
                    if ((ClientSock[x] = accept(mainSock, (struct sockaddr *)&client_info, &addrsize)) == SOCKET_ERROR)
                    {
                        get_time();
                        printf("[ ERROR ] Failed Accept() with error: %d\n", WSAGetLastError());
                    }
                    else
                    {
                        // Print Client IP
                        get_time();
                        getpeername(ClientSock[x], (struct sockaddr *)&client_info, &addrsize);
                        printf("[  INFO ] Client Connect IP: %s:%d\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));

                        threadClients[x] = CreateThread(NULL, 0, Thread_Client, (void *)x, 0x00000004, &threadIdClients[x]);
                        if (threadClients[x] != NULL)
                        {
                            get_time();
                            printf("[  INFO ] ID Thread Clinet %i: %lu\n", x, threadIdClients[x]);
                            ResumeThread(threadClients[x]);
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

/*------------------------ Create Thread Client --------------------*/
DWORD WINAPI Thread_Client(void *data)
{
    int Indice = (int)data;
    int retrn;
    char nickname[30];
    char buffer_thr[Max_Buffer];
    memset(buffer_thr, 0x00, 512);
    status_thr_sock[Indice] = AUTH;

    if (sendTo(&ClientSock[Indice], "/getnick") != 0)
    {
        get_time();
        printf("[ ERROR ] Failed sendTo(ClientSock[%i], \"/getnick\") with error: %d\n", Indice, WSAGetLastError());
        status_thr_sock[Indice] = REUSABLE;
        retrn = 1;
    }
    else
    {
        if ((retrn = receiveFrom(&ClientSock[Indice], buffer_thr)) == 0)
        {
            status_thr_sock[Indice] = BUSY;
            strcpy(nickname, buffer_thr);
            get_time();
            printf("[  INFO ] Client %i Nickname: %s\n", Indice, nickname);
            if (sendTo(&ClientSock[Indice], "Welcom To ChatRoom...") != 0)
            {
                get_time();
                printf("[ ERROR ] Failed sendTo(ClientSock[%i], \"Welcom To ChatRoom...\") with error: %d\n", Indice, WSAGetLastError());
                memset(&buffer_thr, 0x00, 512);
            }
            else
            {
                do
                {
                    if ((retrn = receiveFrom(&ClientSock[Indice], buffer_thr)) == 0)
                    {
                        if (strcmp(buffer_thr, "/exit") == 0)
                        {
                            break;
                        }
                        else
                        {
                            sendToAll(nickname, buffer_thr, Indice);
                        }
                    }
                    else
                    {
                        get_time();
                        printf("[ ERROR ] Failed receiveFrom(&ClientSock[%i], buffer_thr) with error: %d\n", Indice, retrn);
                    }
                    memset(&buffer_thr, 0x00, 512);
                } while (Stop_sever == 0);
            }
        }
        else
        {
            get_time();
            printf("[ ERROR ] Failed receiveFrom(&ClientSock[%i], buffer_thr) with error: %d\n", Indice, retrn);
        }
    }

    closesocket(ClientSock[Indice]);
    WSACleanup();
    get_time();
    printf("[  INFO ] Close Socket Client %i\n\n", Indice);
    status_thr_sock[Indice] = REUSABLE;
    return retrn;
}

int main(int argc, char *argv[])
{
    printLogo();
    threadServer = CreateThread(NULL, 0, Thread_Server, NULL, 0x00000004, &threadIdServer);
    if (threadServer != NULL)
    {
        get_time();
        printf("[  INFO ] ID Thread Server: %lu\n", threadIdServer);
        ResumeThread(threadServer);
    }

    WaitForSingleObject(threadServer, INFINITE);
    WaitForMultipleObjects(0x0000000A, threadClients, TRUE, INFINITE);
    for (int x = 0; x < 10; x++)
    {
        CloseHandle(threadClients[x]);
    }

    CloseHandle(threadServer);

    system("pause");
    return 0;
}

/*------------------------ Get Local IP ----------------------------*/
char *BinIP2StrIP(long lnIP)
{
    static char s_acStrIP[16];

    PUCHAR puc = (unsigned char *)&lnIP;
    unsigned int aun[4];

    for (int i = 0; i < 4; aun[i++] = *puc++)
        ;

    sprintf(s_acStrIP, "%d.%d.%d.%d", aun[0], aun[1], aun[2], aun[3]);
    sprintf(IPserver[0], "%d.%d.%d.%d", aun[0], aun[1], aun[2], aun[3]);

    return (s_acStrIP);
}

void GetIpAddress()
{
    char acClientHost[255];
    WORD wVersionRequested;
    WSADATA wsaData;
    struct hostent *pHostent = NULL;
    int nErr = 0;

    wVersionRequested = MAKEWORD(1, 1);

    if ((nErr = WSAStartup(wVersionRequested, &wsaData)) == -1)
    {
        // error initializing winsck...
    }

    if ((nErr = gethostname((char *)acClientHost, sizeof(acClientHost))) == -1)
    {
        //return NULL;
    }

    if (!(pHostent = gethostbyname(acClientHost)))
    {
        // an error occured...
        //return NULL;
    }

    list[0] = inet_addr(BinIP2StrIP(*((long *)pHostent->h_addr_list[0])));
}

/*----------------------- Setup Socket Main ------------------------*/
int setupSocketMain(int port)
{
    struct sockaddr_in config;
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        get_time();
        printf("[ ERROR ] Failed WSAStartup() with error: %d", WSAGetLastError());
        return 1;
    }
    get_time();
    printf("[  Ok   ] WSAStartup Complete!\n");

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

    config.sin_addr.s_addr = INADDR_ANY;

    config.sin_port = htons(port);

    if (bind(mainSock, (struct sockaddr *)&config, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
    {
        get_time();
        printf("[ ERROR ] Failed main socket binding! %d\n", WSAGetLastError());
        return 2;
    }
    get_time();
    printf("[  OK   ] Bind Success!\n");

    if (listen(mainSock, 10) == SOCKET_ERROR)
    {
        get_time();
        printf("[ERROR] Failed to open listen port!\n");
        perror("listen");
        return 2;
    }
    get_time();
    printf("[  INFO ] Listening Port Open!\n");

    return 0;
}

/*----------------------- Receive / send ---------------------------*/
int receiveFrom(SOCKET *sock, char *bufferFrom)
{
    int countRecv = recv(*sock, bufferFrom, 512, 0);
    if (countRecv > 0)
    {
        get_time();
        printf("[ DEBUG ] Received: %s \n", bufferFrom);
        return 0;
    }
    else if (countRecv == 0)
    {
        get_time();
        printf("[ DEBUG ] Connection closed\n");
        return -1;
    }
    else
    {
        int err = WSAGetLastError();
        get_time();
        printf("[ DEBUG ] recv failed: %d\n", err);
        return err;
    }
}
int sendTo(SOCKET *sock, char *data)
{
    if (send(*sock, data, strlen(data), 0x00) == SOCKET_ERROR)
        return 1;
    return 0;
}

void sendToAll(char *nikc, char *tmp_buffer, int not)
{

    char marc[] = ": ";
    char temp[Max_Buffer];
    strcat(nikc, marc);
    strcat(nikc, tmp_buffer);
    for (int i = 0; i < MAX_Client; i++)
    {
        if (i != not)
        {
            if (status_thr_sock[i] == BUSY)
            {
                sendTo(&ClientSock[i], nikc);
            }
        }
    }
}

/*-------------------------- Colse all Server ----------------------*/
/*
void catch_ctrl_c_and_exit(int sig) {
    ClientList *tmp;
    while (root != NULL) {
        printf("\nClose socketfd: %d\n", root->data);
        close(root->data); // close all socket include server_sockfd
        tmp = root;
        root = root->link;
        free(tmp);
    }
    printf("Bye\n");
    exit(EXIT_SUCCESS);
}*/

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