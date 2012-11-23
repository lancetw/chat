/**
 * client.c : 簡易多人CLI聊天室客戶端
 *
 * Copyright (c) 2012 鄭新霖 aka Hsin-lin Cheng <lancetw@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/* 除錯模式 */

#define _DEBUG        

#include "global.h"

#include <stdio.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

/* 預設緩衝區長度為 1024 bytes */

#define BufferLength 1024

/* 預設開放端口*/

#define PORT 1234

int stdin_ready(int fd);

int main(int argc, char *argv[]) 
{
    int client_sockfd, fd;
    int len;
    struct sockaddr_in address;
    int result;
    char buf[BufferLength]; 
    char message[BufferLength]; 
    fd_set readfds;
    
    FD_ZERO(&buf);
    FD_ZERO(&message);

    /*  建立客戶端 socket  */

    client_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    /*  設定客戶端 socket  */
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(PORT);
    len = sizeof(address);

    /* 與伺服器建立連線 */

    result = connect(client_sockfd, (struct sockaddr *)&address, len);

    if(result == -1) {
        perror("oops: client error");
        exit(EXIT_FAILURE);
    }

    printf("進入聊天室...\n");


    FD_ZERO(&readfds);
    FD_SET(client_sockfd, &readfds);
    
    /* 開始傳送資料 */

    for(;;) {
        /* 開始接收資料 */
        int nread, nwrite;
        
        ioctl(client_sockfd, FIONREAD, &nread);
        
        if (stdin_ready(fileno(stdin))) {
            fscanf(stdin, "%s", buf);
            send(client_sockfd, buf, sizeof (buf), 0);
        }
        
        if(!nread == 0) { /* 處理客戶端資料 */
            
            /* 開始接收資料 */
            if (FD_ISSET(client_sockfd, &readfds)) 
            {
                recv(client_sockfd, buf, sizeof (buf), 0);
                usleep(100);
                printf("%s\n", buf);
            }
            
        }

    }




    /* 關閉 socket */
    
    close(client_sockfd);
    exit(EXIT_SUCCESS);
    
    return EXIT_SUCCESS;
}


/* 以 select() 實作 non-blocking 的輸入 */
int stdin_ready(int fd) {
    fd_set fdset;
    struct timeval timeout;
    int ret;
    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);
    timeout.tv_sec = 0;
    timeout.tv_usec = 1;

    return select(fd + 1, &fdset, NULL, NULL, &timeout) == 1 ? 1 : 0;
}