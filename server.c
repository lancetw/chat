/**
 * server.c : 簡易多人CLI聊天室伺服器
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

#define BUFF_LEN 1024

/* 預設開放端口*/

#define PORT 1234

int main(int argc, char *argv[]) 
{
    int server_sockfd, client_sockfd;
    unsigned int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    int result;
    fd_set readfds, testfds;
    int fdmax;
    char buf[BUFF_LEN]; 
    char message[BUFF_LEN]; 
    int j;
    
    FD_ZERO(&buf);
    FD_ZERO(&message);

    /*  設定 Server socket  */

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(PORT);
    server_len = sizeof(server_address);

    /*  綁定 socket fd 與 server 位址  */

    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);

    /*  傾聽 socket fd  */

    listen(server_sockfd, 10);

    FD_ZERO(&readfds);
    FD_SET(server_sockfd, &readfds);
    
    /* 紀錄目前的 fd 數量 */
    
    fdmax = server_sockfd;

    DEBUG("除錯模式啟動\n");
    printf("伺服器已啟動，正在等待使用者上線\n");

    while(1) {
        int fd;
        int nread;

        testfds = readfds;
        fd = 0;
        
        /* 使用 select() 模擬多人聊天室 */

        result = select(fdmax + 1, &testfds, (fd_set *)0, (fd_set *)0, (struct timeval *) 0);

        if(result < 1) {
            perror("server 發生錯誤");
            exit(EXIT_FAILURE);
        }

        /* 遍歷 FD_LIST */
        
        for(fd = 0; fd <= fdmax; fd++) {
            if(FD_ISSET(fd, &testfds)) {

                /* 處理伺服器端 */
                
                if(fd == server_sockfd) {
                    client_len = sizeof(client_address);
                    client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
                    FD_SET(client_sockfd, &readfds);
                    
                    /* 紀錄 file descriptor 最大值 */
                    
                    if (client_sockfd > fdmax) 
                    { 
                        fdmax = client_sockfd;
                    }
                    
                    DEBUG("%s: 新連線 %s 於 socket %d\n", argv[0], inet_ntoa(client_address.sin_addr), client_sockfd);
                    
                } else {
                    
                    /* 處理連入的客戶端 */
                    
                    ioctl(fd, FIONREAD, &nread);

                    if(nread == 0) {        /* 客戶端沒資料或是斷線 */
                        close(fd);
                        FD_CLR(fd, &readfds);
                        DEBUG("客戶端#%d離線\n", fd);
                        
                    } else {                /* 處理客戶端資料 */
                        
                        /* 開始接收資料 */
                        
                        recv(fd, buf, sizeof (buf), 0);
                        usleep(100);
                        printf("%s\n", buf);

                        /* 處理取得的資料 */
                        for (j = 0; j <= fdmax; j++)
                        {
                            /* 傳給線上所有使用者 */
                            if (FD_ISSET(j, &readfds)) 
                            {
                                /* 不要傳給自己 (server_sockfd) 和原始發送端 (fd) */
                                if (j != server_sockfd && j != fd) 
                                {
                                    DEBUG("送出訊息\"%s\"給#%d\n", buf, j);
                                    send(j, buf, sizeof (buf), 0);
                                }
                            }
                        }                        



                    }
                }
            }
        }
    }
    
    /* 關閉 socket */
    
    close(server_sockfd);
    exit(EXIT_SUCCESS);
    
    return EXIT_SUCCESS;
}
