#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "buffer.h"
#include "parson.h"
#include "helpers.h"
#include "requests.h"

// functie auxiliara pentru compute_get_request folositra in a returna lungimea finala a 
// stringurilor oferite de user pentru a depozita 
// rezultatul

void add_anything_get_request(char*anything, int identifier, char*line){
    if(identifier == 1){
      sprintf(line, "GET %s HTTP/1.1", anything);
    } else if(identifier == 2){
      sprintf(line, "HOST: %s ", anything);
    } else if(identifier == 3){
     sprintf(line, "Authorization: Delimiter %s", anything);
    } else if(identifier == 4){
      sprintf(line, "Cookie: %s", anything);
    }
}

// functie auxiliara pentru compute_post_request folositra in a returna lungimea finala a 
// stringurilor oferite de user pentru a depozita 
// rezultatul

void add_anything_post_request(char*anything, int identifier, char*line){
    if(identifier == 1){
     sprintf(line, "POST %s HTTP/1.1", anything);
    } else if(identifier == 2){
      sprintf(line, "HOST: %s ", anything);
    } else if(identifier == 3){
        sprintf(line, "Authorization: Delimiter %s", anything);
    } else if(identifier == 4){
      if(strlen(anything) > 0){
        sprintf(line, "Cookie: %s", anything);
      }
    }

}

char *compute_get_request(char *host, char *url, char *query_params,
                            char *cookie, char* token) {
   int identifier = 0;
   char *msg = calloc(BUFLEN, sizeof(char));
   char *line = calloc(LINELEN, sizeof(char));

    if (query_params != NULL) { //  verificam parametrii de cerere daca exista
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        identifier = 1;
        // vom retine url-ul
        add_anything_get_request(url, identifier, line);
        
    }

    // adaugam linia in mesajul msg
    strcat(msg, line);
    strcat(msg, "\r\n"); 

    // adaugam host-ul
    memset(line, 0, LINELEN);
    identifier = 2;
    // vom retine host-ul
    add_anything_get_request(host, identifier, line);
    strcat(msg, line);
    strcat(msg, "\r\n");

    // adaugam token 
    if (token != NULL) {
      memset(line, 0, LINELEN);
      identifier = 3;
      add_anything_get_request(token, identifier, line);
      strcat(msg, line);
      strcat(msg, "\r\n");
    }

    // adaugam cookie
    if (cookie != NULL) {
      memset(line, 0, LINELEN);
      identifier = 4;
      add_anything_get_request(cookie, identifier, line);
      strcat(msg, line);
      strcat(msg, "\r\n");
    }
    strcat(msg, "");
    strcat(msg, "\r\n");

    // intoarcem mesajul
    return msg;
}

char *compute_post_request(char *host, char *url, char* content, char **body_data,
                            int body_field_count, char *cookie, char* token) {

    int size, identifier = 0;
    char *msg = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));

    identifier = 1;
    add_anything_post_request(url, identifier, line); // retinem prima linie a mesajului msg
    strcat(msg, line);
    strcat(msg, "\r\n");

    memset(line, 0, LINELEN); 
    identifier = 2;
    // adaugam host-ul
    add_anything_post_request(host, identifier, line);
    strcat(msg, line);
    strcat(msg, "\r\n");


    size = 0;
    // retin im body_data_buffer ce valori sunt in body_data
    for(int i = 0; i < body_field_count - 1; i++)  {
      strcat(body_data_buffer, body_data[i]);
    	strcat(body_data_buffer, "&");
    }

    strcat(body_data_buffer, body_data[body_field_count - 1]);
    size = size + strlen(body_data_buffer);

    // vom retine tipul continutului si ce marime sau mai bine zis diemnsiune a fost calculcata mai inainte
    sprintf(line, "Content-Type: %s\r\nContent-Length: %d", content, size);
    // vom adauga linia la mesaj
    strcat(msg, line);
    strcat(msg, "\r\n");

    // adaugam token
    if (token != NULL) {
      memset(line, 0, LINELEN);
      identifier = 3;
      add_anything_post_request(token, identifier, line);
      strcat(msg, line);
      strcat(msg, "\r\n");
    }

    // adaugam cookie
  	if (cookie != NULL) {
      memset(line, 0, LINELEN);
      identifier = 4;
      add_anything_post_request(cookie, identifier, line);
      strcat(msg, line);
      strcat(msg, "\r\n");
    }

    // adaugam noua linie a header-ului
    strcat(msg, "");
    strcat(msg, "\r\n");
    // adaugam payload-ul
    memset(line, 0, LINELEN);

    // adaugam noua linie a header-ului
    strcat(msg, body_data_buffer);
    strcat(msg, "\r\n");


    // eliberam memoria
    free(line);

    return msg;
}
