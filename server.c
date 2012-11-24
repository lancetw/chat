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

void timestamp(char*);

int main(int argc, char *argv[]) 
{
    int server_sockfd, client_sockfd, on;
    unsigned int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    int result;
    fd_set readfds, testfds;
    int fdmax;
    char buf[BUFF_LEN]; 
    char tmp[BUFF_LEN]; 
    char msg[BUFF_LEN]; 
    int j;

    /*  建立伺服器 socket  */

    if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket() 呼叫失敗");
        exit(EXIT_FAILURE);
    }
    
    /* "address already in use" 錯誤訊息 */
    
    if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (int)) == -1) {
        perror("setsockopt() 呼叫失敗");
        exit(EXIT_FAILURE);
    }

    /*  設定伺服器 socket  */

    memset(&server_address, 0x00, sizeof (struct sockaddr_in));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(PORT);
    server_len = sizeof (server_address);
    server_len = sizeof (server_address);

    /*  綁定 socket fd 與伺服器位址  */

    if (bind(server_sockfd, (struct sockaddr *)&server_address, server_len) == -1) {
        perror("bind() 呼叫失敗");
        exit(EXIT_FAILURE);

    }

    /*  傾聽 socket fd  */

    if (listen(server_sockfd, 10) == -1) {
        perror("listen() 呼叫失敗");
        exit(EXIT_FAILURE);

    }

    FD_ZERO(&readfds);
    FD_ZERO(&testfds);
    FD_ZERO(&buf);
    FD_ZERO(&tmp);
    FD_ZERO(&msg);
    FD_SET(server_sockfd, &readfds);
    
    /* 紀錄目前的 fd 數量 */
    
    fdmax = server_sockfd;

    DEBUG("除錯模式啟動\n");
    printf("伺服器已啟動，正在等待使用者上線\n");

    while(1) {
        int fd;
        int nread;

        /* 複製編號 */
        testfds = readfds;
        fd = 0;
        
        /* 使用 select() 模擬多人聊天室 */

        result = select(fdmax + 1, &testfds, (fd_set *)0, (fd_set *)0, (struct timeval *) 0);

        if(result < 1) {
            perror("伺服器發生問題");
            exit(EXIT_FAILURE);
        }

        /* 遍歷 fd_set */
        
        for(fd = 0; fd <= fdmax; fd++) {
            if(FD_ISSET(fd, &testfds)) {

                /* 處理伺服器端 */
                
                if(fd == server_sockfd) {
                    client_len = sizeof(client_address);
                    client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
                    FD_SET(client_sockfd, &readfds);
                    
                    /* 紀錄 file descriptor 最大值 */
                    
                    if (client_sockfd > fdmax) { 
                        fdmax = client_sockfd;
                    }
                    DEBUG("%s: 新連線 %s 於 socket#%d\n", argv[0], inet_ntoa(client_address.sin_addr), client_sockfd);
                    
                } else {
                    
                    /* 處理連入的客戶端 */
                    
                    ioctl(fd, FIONREAD, &nread);

                    if(nread == 0) {        /* 客戶端沒資料或是斷線 */
                        close(fd);
                        FD_CLR(fd, &readfds);
                        DEBUG("客戶端#%d離線\n", fd);
                        
                    } else {                /* 處理客戶端資料 */
                        
                        /* 開始接收資料 */
                        
                        if (recv(fd, buf, sizeof (buf), 0)) {
                             DEBUG("訊息已接收\n");
                        }

                        /* 處理取得的資料 */
                        
                        usleep(100);
                        
                        timestamp(tmp);
                        sprintf(msg, "%s %s", tmp, buf);
                        printf("%s\n", msg);
                        
                        for (j = 0; j <= fdmax; j++) {
                            
                            /* 傳給線上所有使用者 */
                            
                            if (FD_ISSET(j, &readfds)) {
                                
                                /* 不要傳給自己 (server_sockfd) */
                                
                                if (j != server_sockfd) {
                                    if (send(j, msg, sizeof (msg), 0)) {
                                        DEBUG("送出訊息\"%s\"給#%d\n", buf, j);
                                    }
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


void timestamp(char* ubuf) {
    
    char            fmt[64], buf[64];
    struct timeval  tv;
    struct tm       *tm;

    gettimeofday(&tv, NULL);
    
    if((tm = localtime(&tv.tv_sec)) != NULL) {
            strftime(fmt, sizeof (fmt), "[%H:%M:%S]", tm);
            snprintf(buf, sizeof (buf), fmt, tv.tv_usec);
            memcpy(ubuf, buf, sizeof (buf));
    }
}
