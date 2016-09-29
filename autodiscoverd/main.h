
#ifndef _AUTODISCOVERD_MAIN_H_
#define _AUTODISCOVERD_MAIN_H_

#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct request {
  char *hostname;
  unsigned int port;
  char request_body[1024];
  int sockfd;
  unsigned int sent_bytes;
  unsigned int received_bytes;
  char response_body[4096];
} request_t;

int request_send(request_t *r);
int request_recieve(request_t *r);

#endif
