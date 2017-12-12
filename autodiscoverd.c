//
//  main.c
//  autodiscoverd
//
//  Created by Edward Hurtig on 2/6/16.
//  Copyright © 2016 Edward Hurtig. All rights reserved.
//
#include "autodiscoverd.h"

int main(int argc, char *argv[]) {
  int ret = 1;

  // Only supporting port 80 at this time
  int portno = 80;
  char *host[10] = {"api.hurtigtechnologies.com", "ip.ht.gs",
                    "ip.hurtigtechnologies.com", NULL};
  char *message_fmt = "GET /hosts/?host=%s HTTP/1.0\r\nHost: %s\r\n\r\n";

  char local_hostname[1024];
  local_hostname[1023] = '\0';
  gethostname(local_hostname, 1023);
  printf("Local Hostname: %s\n", local_hostname);

  int i = -1;
  while (host[++i] != NULL) {
    // Build the HTTP Request

    request_t *r = malloc(sizeof(request_t));

    sprintf(r->request_body, message_fmt, local_hostname, host[i]);
    r->hostname = host[i];
    r->port = 89;

    request_send(r);

    request_recieve(r);

    /* receive the response */

    // Determine if it was a success
    // Currently just checking for the string '"successs":true'
    if (strstr(r->response_body, "\"success\":true") != NULL) {
      printf(">> Published Response to server: %s\r\n", r->hostname);
      ret = 0; // At lease one of the posts succeeded
    } else {
      printf(">> Failed to publish response to server: %s\r\n", r->hostname);
    }

    free(r);
  }
  return ret;
}

int request_send(request_t *r) {
  struct hostent *server;
  struct sockaddr_in svr_addr;
  int bytes, sent, received, total;

  r->sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (r->sockfd < 0) {
    fprintf(stderr, "Failed: Socket Error %d\r\n", r->sockfd);
    return -1;
  }

  /* lookup the ip address */
  server = gethostbyname(r->hostname);
  if (server == NULL) {
    fprintf(stderr, "Failed: no such host: %s\r\n", r->hostname);
    return -1;
  }
  fprintf(stderr, "Info: Server Address is %u.%u.%u.%u\r\n",
          (unsigned char)server->h_addr_list[0][0],
          (unsigned char)server->h_addr_list[0][1],
          (unsigned char)server->h_addr_list[0][2],
          (unsigned char)server->h_addr_list[0][3]);
  /*s fill in the structure */
  memset(&svr_addr, 0, sizeof(svr_addr));
  svr_addr.sin_family = AF_INET;
  svr_addr.sin_port = htons(r->port);
  memcpy(&svr_addr.sin_addr.s_addr, server->h_addr, server->h_length);

  /* connect the socket */
  printf("%c\r\n", svr_addr.sin_addr.s_addr);

  if (connect(r->sockfd, (struct sockaddr *)&svr_addr, sizeof(svr_addr)) < 0) {
    fprintf(stderr, "Failed: error connecting to %s: %d\r\n", r->hostname,
            errno);
    return -1;
  }

  /* send the request */
  total = strlen(r->request_body);
  r->sent_bytes = 0;
  do {
    bytes = write(r->sockfd, r->request_body + r->sent_bytes,
                  total - r->sent_bytes);
    if (bytes < 0) {
      printf("Failed: error writing message to socket\r\n");
      return -1;
    }
    if (bytes == 0)
      break;
    r->sent_bytes += bytes;
  } while (r->sent_bytes < total);
  return 0;
}

int request_recieve(request_t *r) {
  memset(r->response_body, 0, sizeof(r->response_body));
  int total, bytes;
  total = sizeof(r->response_body) - 1;
  r->received_bytes = 0;
  do {
    bytes = read(r->sockfd, r->response_body + r->received_bytes,
                 total - r->received_bytes);
    if (bytes < 0) {
      printf("Failed: error reading response from socket\r\n");
      break;
    }
    if (bytes == 0)
      break;
    r->received_bytes += bytes;
  } while (r->received_bytes < total);

  if (r->received_bytes == total) {
    printf("Failed: error storing complete response from socket\r\n");
    return -1;
  }

  // Close the socket
  close(r->sockfd);
  return 0;
}
