#ifndef SIM800L_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void send_data(const char *data);
unsigned long read_data(char **data);
int is_url_ssl(char *url);
char *trim_url(char *url);
char *get_url_location(char *url);

int GSM_HTTP(char *method, char *url, char *body);
int GSM_HTTP_PUT(char *url, char *body);
int GSM_HTTP_PATCH(char *url, char *body);

// Remove
int initialize_local_port();
void destroy_local_port();

#endif