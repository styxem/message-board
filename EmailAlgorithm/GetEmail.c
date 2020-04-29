#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#define PORT 80

int main(int argc, char const *argv[])
{
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\nSOCKET CONNECTION ERROR\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IP addresses
    if(inet_pton(AF_INET, gethostbyname("imap.gmail.com"), &serv_addr.sin_addr) <= 0)
    {
        printf("\nINVALID ADDRESS\n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nCONNECTION FAILED\n");
        return -1;
    }

    send(sock, hello, strlen(hello), 0);
    printf("GET /");
    valread = read(sock, buffer, 1024);
    printf("%s\n", buffer);
    return 0;
}