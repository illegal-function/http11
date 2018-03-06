#ifndef ILF_HTTP11_H
#define ILF_HTTP11_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

enum {
  cHttpParseFailed = -1,
};

struct ilfHttpState {
  unsigned type : 1;
  unsigned state : 15;
};

struct ilfHttpVTable {
  int _ignore;
};

long ilfHttpParse( struct ilfHttpState * const restrict,
                   const void * const restrict,
                   const size_t,
                   const struct ilfHttpVTable * const restrict );


enum {
  cHttpUriScheme = 0,
  cHttpUriUsername,
  cHttpUriPassword,
  cHttpUriHostname,
  cHttpUriPort,
  cHttpUriPath,
  cHttpUriQueryString,
  cHttpUriFragment,
  _cHttpUriLast,
};

struct ilfHttpUri {
  struct {
    const char *ptr;
    size_t length;
  } parts[_cHttpUriLast];
};

long ilfHttpUriParse( const char * const restrict,
                      const size_t,
                      struct ilfHttpUri * const );

#endif
