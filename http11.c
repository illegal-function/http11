#include "http11.h"

#include <stdio.h>

/*
 * The following core rules are included by reference, as defined in
 * [RFC5234], Appendix B.1: ALPHA (letters), CR (carriage return), CRLF
 * (CR LF), CTL (controls), DIGIT (decimal 0-9), DQUOTE (double quote),
 * HEXDIG (hexadecimal 0-9/A-F/a-f), HTAB (horizontal tab), LF (line
 * feed), OCTET (any 8-bit sequence of data), SP (space), and VCHAR (any
 * visible [USASCII] character).
 */

enum {
  s_nostate = 0,
  s_method_tchar,
  s_method_sp,
  s_uri_char,

  s_req_proto_h,
  s_req_proto_ht,
  s_req_proto_htt,
  s_req_proto_http,
  s_req_require_slash,
  s_req_require_1,
  s_req_require_dot,
  s_req_require_dot_1,


  s_req_require_cr,
  s_req_require_lf,

  s_hfield_fchar,
  s_hfield_char,
  s_hfield_hvalue_fchar,

  s_is_double_crlf,
  s_have_2cr,

  s_hvalue_have_cr,
  s_hvalue_have_crlf,

  s_status_1,
};

static inline bool is_tchar(const unsigned char i) {
  static const unsigned char table[0x100] = {
    ['!'] = 1, ['#'] = 1, ['$'] = 1, ['%'] = 1, ['&'] = 1,
   ['\''] = 1, ['*'] = 1, ['+'] = 1, ['-'] = 1, ['.'] = 1,
    ['^'] = 1, ['_'] = 1, ['`'] = 1, ['|'] = 1, ['~'] = 1,
    ['0'] = 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    ['A'] = 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    ['a'] = 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  };

  return table[i];
}

static inline bool is_urichar(const unsigned char i) {
  static const unsigned char table[0x100] = {
    ['-'] = 1, ['.'] = 1, ['_'] = 1, ['~'] = 1, [':'] = 1,
    ['/'] = 1, ['?'] = 1, ['#'] = 1, ['['] = 1, [']'] = 1,
    ['@'] = 1, ['!'] = 1, ['$'] = 1, ['&'] = 1, ['\''] = 1,
    ['('] = 1, [')'] = 1, ['*'] = 1, ['+'] = 1, [','] = 1,
    [';'] = 1, ['='] = 1, ['`'] = 1, ['%'] = 1,

    ['0'] = 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    ['A'] = 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    ['a'] = 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  };

  return table[i];
}

long ilfHttpParse(struct ilfHttpState * const restrict state,
                  const void * const restrict input,
                  const size_t size,
                  const struct ilfHttpVTable * const restrict vtable) {
  // --
  const unsigned char *p = input;
  const unsigned char *endptr = p + size;

  while (p < endptr) {
    printf("%c\n", *p);
    switch (state->state) {
      case s_nostate:
        // request
        if (state->type == 0) {
          if (!is_tchar(*p) ) return cHttpParseFailed;
          ++p;
          state->state = s_method_tchar;
        } else {
          if (*p >= '0' && *p <= '9') {
            state->state = s_status_1;
            ++p;
          } else {
            return cHttpParseFailed;
          }
        }

        break;

      case s_method_tchar:
        if (is_tchar(*p) ) {
          ++p;
        } else if (*p == ' ') {
          state->state = s_method_sp;
          ++p;
        }
        else {
          return cHttpParseFailed;
        }

        break;

      case s_method_sp:
        //
        if (!is_urichar(*p) ) return cHttpParseFailed;
        state->state = s_uri_char;
        ++p;
        break;

      case s_uri_char:
        if (is_urichar(*p) ) {
          ++p;
          break;
        }
        else if (*p == ' ') {
          ++p;
          state->state = s_req_proto_h;
          break;
        }
        else {
          return cHttpParseFailed;
        }

      case s_req_proto_h:
        if (*p != 'H') return cHttpParseFailed;
        p++;
        state->state = s_req_proto_ht;
        break;

      case s_req_proto_ht:
        if (*p != 'T') return cHttpParseFailed;
        p++;
        state->state = s_req_proto_htt;
        break;

      case s_req_proto_htt:
        if (*p != 'T') return cHttpParseFailed;
        p++;
        state->state = s_req_proto_http;
        break;

      case s_req_proto_http:
        if (*p != 'P') return cHttpParseFailed;
        p++;
        state->state = s_req_require_slash;
        break;

      case s_req_require_slash:
        if (*p != '/') return cHttpParseFailed;
        p++;
        state->state = s_req_require_1;
        break;

      case s_req_require_1:
        if (*p != '1') return cHttpParseFailed;
        p++;
        state->state = s_req_require_dot;
        break;

      case s_req_require_dot:
        if (*p != '.') return cHttpParseFailed;
        p++;
        state->state = s_req_require_dot_1;
        break;

        case s_req_require_dot_1:
          if (*p != '1') return cHttpParseFailed;
          p++;
          state->state = s_req_require_cr;
          break;

        case s_req_require_cr:
          if (*p != '\r') return cHttpParseFailed;
          p++;
          state->state = s_req_require_lf;
          break;

        case s_req_require_lf:
          if (*p != '\n') return cHttpParseFailed;
          p++;
          state->state = s_hfield_fchar;
          break;

        // header fields (tokens)
        case s_hfield_fchar:
          if (!is_tchar(*p) ) return cHttpParseFailed;
          p++;
          state->state = s_hfield_char;
          break;

        case s_hfield_char:
          if (is_tchar(*p) ) {
            p++;
            break;
          } else if (*p == ':') {
            p++;
            state->state = s_hfield_hvalue_fchar;
            break;
          } else {
            return cHttpParseFailed;
          }

        // accept pre whitespace
        // OWS == optional whitespace
        case s_hfield_hvalue_fchar:
          if (*p == ' ' || *p == '\t') {
            p++;
            break;
          } else if (*p == '\r') {
            state->state = s_hvalue_have_cr;
          }
          p++;
          break;

        case s_hvalue_have_cr:
          if (*p != '\n') return cHttpParseFailed;
          p++;
          state->state = s_is_double_crlf;
          break;

        case s_is_double_crlf:
          if (*p == '\r') {
            p++;
            state->state = s_have_2cr;
            break;
            // have header end
          } else {
            state->state = s_hfield_fchar;
            break;
          }

        case s_have_2cr:
          if (*p != '\n') return cHttpParseFailed;
          p++;
          // headers parsed
          return 0;

      default:
        return cHttpParseFailed;

    }
  }

  return 0;
}
