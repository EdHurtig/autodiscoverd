//
//  main.c
//  autodiscoverd
//
//  Created by Edward Hurtig on 2/6/16.
//  Copyright © 2016 Edward Hurtig. All rights reserved.
//

#include <stdio.h>

#include <stdio.h> /* printf, sprintf */
#include <stdlib.h> /* exit */
#include <unistd.h> /* read, write, close */
#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */

void error(const char *msg) { perror(msg); exit(0); }

int main(int argc,char *argv[])
{
    /* first what are we going to send and where are we going to send it? */
    int portno =        80;
    char *host[10] =  { "api.hurtigtechnologies.com", "ip.ht.gs", "ip.hurtigtechnologies.com", NULL };
    char *message_fmt = "GET /hosts/?host=%s HTTP/1.0\r\nHost: %s\r\n\r\n";
    
    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd, bytes, sent, received, total;
    char message[1024],response[4096];
    
    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);
    printf("Hostname: %s\n", hostname);
    struct hostent* h;
    h = gethostbyname(hostname);
    printf("h_name: %s\n", h->h_name);
    

    int i = -1;
    while (host[++i] != NULL) {
        /* fill in the parameters */
        sprintf(message,message_fmt, hostname, host[i]);

        /* create the socket */
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            printf("ERROR opening socket\r\n");
            continue;
        }
        
        /* lookup the ip address */
        server = gethostbyname(host[i]);
        if (server == NULL) {
            printf("Failed: no such host: %s\r\n", host[i]);
            continue;
        }
        
        /* fill in the structure */
        memset(&serv_addr,0,sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(portno);
        memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);
        
        /* connect the socket */
        if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
            printf("ERROR connecting: %s\r\n", host[i]);
            continue;
        }
        
        /* send the request */
        total = strlen(message);
        sent = 0;
        do {
            bytes = write(sockfd,message+sent,total-sent);
            if (bytes < 0) {
                printf("ERROR writing message to socket\r\n");
                continue;
            }
            if (bytes == 0)
                break;
            sent+=bytes;
        } while (sent < total);
        
        /* receive the response */
        memset(response,0,sizeof(response));
        total = sizeof(response)-1;
        received = 0;
        do {
            bytes = read(sockfd,response+received,total-received);
            if (bytes < 0) {
                printf("ERROR reading response from socket\r\n");
                break;
            }
            if (bytes == 0)
                break;
            received+=bytes;
        } while (received < total);
        
        if (received == total) {
            printf("ERROR storing complete response from socket\r\n");
            continue;
        }
        
        /* close the socket */
        close(sockfd);

        /* process response */
        if ( strstr(response, "{\"success\":true}") != NULL ) {
            printf("Published Response to server: %s\r\n", host[i]);
        } else {
            printf("Failed to publish response to server: %s\r\n", host[i]);
        }
        
    }
    
    return 0;
}
