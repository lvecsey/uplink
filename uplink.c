/*
    Keep a hit counter in memory using FastCGI based on unique URL's
    Copyright (C) 2020  Lester Vecsey

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <stdint.h>
#include <string.h>

#include <fcgiapp.h>
#include <errno.h>

#include "critbit.h"

#define max_urlsz 1024

int crit_getcount(const char *str, void *extra) {

  uint64_t *uplink_counter;

  char *match = "->";

  char *rptr;

  char *mp, *me;

  if (str != NULL) {

    rptr = str;

    mp = match;
    me = match + 2;

    while (rptr[0]) {

      if (mp >= me) {

	uplink_counter = (uint64_t*) extra;
	
	*uplink_counter = strtol(rptr, NULL, 10);

	return 0;
	
      }

      if (rptr[0] == mp[0]) {
	mp++;
      }

      else {
	mp = match;
      }

      rptr++;
      
    }

  }
  
  return 1;
  
}

int main(int argc, char *argv[]) {

  FCGX_Request request;

  uint64_t uplink_counter;

  char *query_string;

  char *client_url;

  int retval_qs;

  int retval_ap;
  
  int retval;

  critbit0_tree url_counters;

  char *critbuf;

  long int noincr;
  
  FCGX_Init();
  FCGX_InitRequest(&request, 0, 0);

  uplink_counter = 0;

  client_url = malloc(max_urlsz);
  if (client_url == NULL) {
    perror("malloc");
    return -1;
  }

  critbuf = malloc(max_urlsz * 2);
  if (critbuf == NULL) {
    perror("malloc");
    return -1;
  }
  
  url_counters = (critbit0_tree) { .root = NULL };
  
  while (FCGX_Accept_r(&request) == 0) {

    query_string = FCGX_GetParam("QUERY_STRING", request.envp);

    FCGX_PutS("Content-type: text/html\r\n\r\n", request.out);
    
    if (strlen(query_string) > max_urlsz) {
      continue;
    }

    retval_qs = sscanf(query_string, "url=%[^&]&noincr=%ld", client_url, &noincr);

    if (retval_qs < 1) {
      continue;
    }

    {
      int len_url;
      len_url = strlen(client_url);
      if (len_url > 2 && client_url[len_url-2] == '-' && client_url[len_url-1] == '>') {
	printf("Rejecting URL.\n");
	continue;
      }
    }
    
    printf("client_url %s\n", client_url);
    
    retval = sprintf(critbuf, "%s->", client_url);
    
    retval_ap = critbit0_allprefixed(&url_counters, critbuf, crit_getcount, &uplink_counter);

    printf("retval %d uplink_counter %lu\n", retval, uplink_counter);

    if (retval_ap == 1) {

      uplink_counter = 0;
      
    }
    
    if (retval_qs == 2 && noincr == 1) {

      FCGX_FPrintF(request.out, "var uplink_counter = %lu;\n", uplink_counter);
      
      continue;

    }

    switch(retval_ap) {
    case 0:
    
      retval = sprintf(critbuf, "%s->%lu", client_url, uplink_counter);
      retval = critbit0_delete(&url_counters, critbuf);
      printf("(delete) retval %d\n", retval);
      
      break;
      
    }

    uplink_counter++;
    
    retval = sprintf(critbuf, "%s->%lu", client_url, uplink_counter); 
    retval = critbit0_insert(&url_counters, critbuf);
    printf("(insert) retval %d\n", retval);

    FCGX_FPrintF(request.out, "var uplink_counter = %lu;\n", uplink_counter);
    
  }

  free(critbuf);
  
  free(client_url);
  
  return 0;

}
