#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include "uv.h"

#define ThreadsNum 10

int open_cliendfd (char *host, int port)
{
    int fd;
    struct hostent *hp;
    struct sockaddr_in server_addr;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return -1;

    if ((hp = gethostbyname(host)) == NULL)
        return -2;

    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)hp->h_addr_list[0],
          (char *)&server_addr.sin_addr.s_addr, hp->h_length);
    server_addr.sin_port = htons(port);

    if (connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        return -1;
    return fd;
}

char *imap_recv(int fd, size_t size) {
    size_t cursor = 0;
    int rc;

    char *buffer = (char *)malloc(size * sizeof(char));
    char *result = (char *)malloc(size * sizeof(char));

    while ((rc = (int)recv(fd, buffer, size, 0))) {

        if (rc == -1)
            continue;

        buffer[rc] = '\0';
        int len = (int)(sizeof(char)*(cursor++)*size) + rc;
        char *temp_str = buffer;
        char *temp_res = (char *)malloc(len);
        memcpy(temp_res, result, len);

        if (result != NULL) {
            strcat(temp_res, temp_str);
            memcpy(result, temp_res, strlen(temp_res)+1);
        } else {
            memcpy(result, buffer, strlen(buffer)+1);
        };
        //free(temp_res);

        // break
        if (rc < size) break;
    };

    return result;
}

void * thread_null(void* args_t);
int check_ok(char* str);

int main(int argc, char *argv[])
{
    printf("test\n");
    int i, rc; char x;

    pthread_t threads[ThreadsNum];

    for(i = 0; i < ThreadsNum; i++) {
        rc = pthread_create(&threads[i], NULL, thread_null, (void*)i);
        if (0 != rc) {
            fprintf(stderr, "pthread_create() failure\r\nMax pthread num is %d\r\n", i);
            exit(-1);
        } else {
            fprintf(stdout, "pthread_create() success\r\nCurrent pthread num is %d\r\n", i);
        }
    }
    fprintf(stdout, "Max pthread num is 65536\r\nYour system is power_full\r\n");
    scanf("blocking thread: %c", &x);
    //exit(0);
}

int check_ok(char* str)
{
    int len = (int)strlen(str), i;
    int is_ok = 0;
    for (i=0; i<len; i++) {
        if (i+4 > len) break;
        if (str[i] == 'O' && str[i+1] == 'K') {
            is_ok = 1;
            break;
        };
    };
    return is_ok;
}

void* thread_null(void* args_t)
{
    char login[50];
    snprintf(login, 50, "A2 LOGIN \"test%d\" \"ping55555\"\n", (int)args_t+1);

    int fd = open_cliendfd("imap.pingkit.com", 143);
    if (fd < 0) {
        fprintf(stdout, "fd = %d, open file failed\n", fd);
        return 0;
    } else {
        fprintf(stderr, "fd = %d\n", fd);
    };

    // init
    char *result = imap_recv(fd, 100);
    printf("len=%lu\n", strlen(result));

    char *buffers[6] = {
            "A1 CAPABILITY\n",
            login,
            "A3 CAPABILITY\n",
            "A4 ID (\"name\" \"inbox\" \"version\" \"1.0.0\" \"support-url\" \"http://yorkiefixer.me\")\n",
            "A5 SELECT \"INBOX\"\n",
            //"A6 FETCH *:* (UID ENVELOPE)\n"
            //"A7 LOGOUT\n"
    };

    int i = 0;
    do {
        printf("S: %s\n", result);
        if (i == 0) {
            printf("C: %s\n", buffers[i]);
            send(fd, buffers[i], strlen(buffers[i]), 0);
            i++;
            continue;
        }

        if (check_ok(result)) {
            if (i >= 5) {
                break;
            } else {
                printf("C: %s\n", buffers[i]);
                send(fd, buffers[i], strlen(buffers[i]), 0);
                i++;
            }
        }
    }
    while ((result = imap_recv(fd, 100)) || 1);

    int cursor = 10;
    while (1) {
        char msg[30];
        snprintf(msg, sizeof(msg), "A%d IDLE\n", cursor++);
        send(fd, msg, strlen(msg), 0);
        printf("C: %s\n", msg);

        // + idling...
        result = imap_recv(fd, 50);
        printf("S: %s\n", result);
        if (result[0] != '+')
            continue;

        // %d Exists...
        EXISTS:
        result = imap_recv(fd, 200);
        printf("S: %s\n", result);
        char seq[3];
        int k = 0;
        while (result[0] == '*') {
            if (result[2] < 48 || result[2] > 57)
                goto EXISTS;

            while (1) {
                printf("k=%d, chars=%c,%c\n", k, result[k], result[k+1]);
                if (result[k] >= 48 && result[k] <= 57 && result[k+1] == ' ') {
                    if (result[k+2] != 'E') break;
                    int start = 2;
                    while (start <= k) {
                        seq[start-2] = result[start];
                        start += 1;
                    };
                    printf("seq=%s\n", seq); break;
                };
                k++;
            };
            send(fd, "DONE\n", 5, 0);
            printf("test%d comming a message\n", (int)args_t+1);
            printf("C: DONE\n");

            // OK IDLE terminated
            result = imap_recv(fd, 100);
            printf("S: %s\n", result);

            snprintf(msg, sizeof(msg), "A%d FETCH %s ALL\n", cursor++, seq);
            printf("C: %s\n", msg);
            send(fd, msg, strlen(msg), 0);

            // loop for FETCH OK Completed.
            while (1) {
                result = imap_recv(fd, 100);
                printf("S: %s\n", result);
                if (check_ok(result))
                    break;
            };

            break;
        };
    }

    printf("done\n");
    return 0;
}