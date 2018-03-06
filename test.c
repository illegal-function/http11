#include "http11.c"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const char input[] = "GET /testuri.php HTTP/1.1\r\n"
                     "Host: localhost:8080\r\n"
                     "Connection: close\r\n"
                     "\r\n"
                     ;


int main(void) {
  struct ilfHttpState state = {
    .type = 0,
  };

  long rv = ilfHttpParse(&state, input, strlen(input), NULL);

  if (rv == cHttpParseFailed) {
    printf("Failed\n\n");
    exit(EXIT_FAILURE);
  }
  else {
    printf("Success\n\n");
  }
}
