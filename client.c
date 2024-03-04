#define _GNU_SOURCE

#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "parson.h"
#include "buffer.h"
#include "helpers.h"
#include "requests.h"
#include <stdbool.h>

// variabila care ne ajuta sa stim daca s-a logat sau nu user-ul
int logged_in = 0;


// functie care verifica daca am sau nu caractere in numarul de pagini si intoarce un bool
bool Has_characters(char* page_count){

    int num_of_characters = strlen(page_count);
    for(int i = 0; i < num_of_characters; i++){
        if(page_count[0] == '0'){
            return false;
        } else if(page_count[0] == '-'){
            printf("The page count should be pozitive !!!\n");
            return false;
        } else if(page_count[i] >= 'a' && page_count[i] <= 'z') {
            printf("Your book should only have numbers as page_count !!! \n");
            return false;
        } else if(page_count[i] >= 'A' && page_count[i] <= 'Z') {
            printf("Your book should only have numbers as page_count !!! \n");
            return false;
        }
    }
    return true;
}


char* substr(char* elem){
    elem = strtok(elem, " ");
    return elem;
}

char* null_substr(char* elem){
    elem = strtok(NULL, " ");
    return elem;
}

char* points(char* elem, char* str1, char* str2, char* str3){
    elem = strtok(elem, str1);
    elem = strtok(NULL, str2);
    elem = strtok(elem, str3);

    return elem;
}

void register_client() {
    int socket, index;
    int code_to_exit;
    char *array = "";
    int spaces_username = 0;
    int spaces_password = 0;
    char *message_for_server, *message_received_server;
    char *username_when_register, *password_when_register;
    size_t size_for_user_register, size_for_password_register;

    // initiam conexiunea la server
    socket = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
    // alocam memorie pentru username si password
    username_when_register = calloc(150, sizeof(char));
    size_for_user_register = 150;

    password_when_register = calloc(150, sizeof(char));
    size_for_password_register = 150;
    
    // introducem de la tastatura username-ul si parola, 
    // am folosit getline pentru partea de citire de la tastatura
    printf("username=");
    getline(&username_when_register, &size_for_user_register, stdin);

    // verific sa nu fie spatii in username :
    // M-am inspirat de aici : https://stackoverflow.com/questions/41649672/a-check-in-c-language-if-there-is-a-space-inside-the-array-string
    for(int i = 0; username_when_register[i] != '\0'; ++i){
        if(username_when_register[i] == ' '){
            printf("ERROR : You are not allowed to have white spaces in your username when registering !!\n");
            spaces_username = 1;
            break;
        }
    }

    if(spaces_username == 1){
        return;
    }

    index = strlen(username_when_register) - 1;
    username_when_register[index] = '\0';

    printf("password="); 
    getline(&password_when_register, &size_for_password_register, stdin);

    // verific sa nu fie spatii in parola : 
    // M-am inspirat de aici : https://stackoverflow.com/questions/41649672/a-check-in-c-language-if-there-is-a-space-inside-the-array-string

     for(int i = 0; password_when_register[i] != '\0'; i++){
        if(password_when_register[i] == ' '){
            printf("ERROR : You are not allowed to have white spaces in your password when registering !!\n");
            spaces_password = 1;
            break;
        }
    }

    if(spaces_password == 1){
        return;
    }
    
    index = strlen(password_when_register) - 1;
    password_when_register[index] = '\0';

    JSON_Value *json_value = json_value_init_object();
    JSON_Object *json_elem = json_value_get_object(json_value);

    // formatam mesajele de tip JSON, m-am inspirat din linkul de github oferit in documentatia temei pentru partea de C
    if (json_value != NULL && json_elem != NULL) {
        json_object_set_string(json_elem, "username", username_when_register);
        json_object_set_string(json_elem, "password", password_when_register);
    }
    
    array = json_serialize_to_string_pretty(json_value);

    // retinem mesajul care a fost trimis catre server
    message_for_server = compute_post_request("34.254.242.81:8080",
     "/api/v1/tema/auth/register", "application/json", &array, 1, NULL, NULL);

    // trimitem mesajul catre server
    send_to_server(socket, message_for_server);

    // aici primi mesaj de la server
    message_received_server = receive_from_server(socket);

    // pe baza mesajului primit de la server extragem code_to_exit
    // am folosit strtok in functiile substr si null_substr
    // https://www.codecademy.com/resources/docs/c/strings/strtok
    // m-am inspirat din link-ul atasat si mai sus
    if (message_received_server != NULL) {
        message_received_server = substr(message_received_server);
        message_received_server = null_substr(message_received_server);
    } 

    code_to_exit = atoi(message_received_server);

    if (code_to_exit == 200) {
        printf("ok !!!\n");
    } else if (code_to_exit == 201) {
        printf("ok !!!\n");
    } else {
        printf("ERROR : The username : %s is already taken !!!\n", username_when_register);
    }

    // eliberam memoria
    if (username_when_register != NULL) {
        free(username_when_register);
    }
    
    if (password_when_register != NULL) {
        free(password_when_register);
    }
    
    if (array != NULL) {
        json_free_serialized_string(array);
        
    }

    if (json_value != NULL) {
        json_value_free(json_value);
    }

    // inchidem conexiunea cu serverul 
    close(socket);
}

char* login_client() {
    logged_in = 1;
    int index, socket;
    int spaces_username = 0;
    int spaces_password = 0;
    char *username_when_login, *password_when_login;
    size_t size_for_user_login, size_for_password_login;
    char *returned_value = "", *array = "";
    char *message_for_server, *message_received_server, *code_to_exit;

    // deschidem conexiunea cu serverul
    socket = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);

    // alocam memorie pentru username si parola
    username_when_login = calloc(150, sizeof(char));
    size_for_user_login = 150;

    password_when_login = calloc(150, sizeof(char));
    size_for_password_login = 150;

    // citim de la tastatura username-ul si password-ul
    printf("username=");
    getline(&username_when_login, &size_for_user_login, stdin);

    // verific sa nu fie spatii in username : 
    // M-am inspirat de aici : https://stackoverflow.com/questions/41649672/a-check-in-c-language-if-there-is-a-space-inside-the-array-string
    for(int i = 0; username_when_login[i] != '\0'; ++i){
        if(username_when_login[i] == ' '){
            printf("ERROR : You are not allowed to have white spaces in your username when logging in\n");
            spaces_username = 1;
            break;
        }
    }

    if(spaces_username == 1){
        return NULL;
    }
    
    printf("password="); 
    getline(&password_when_login, &size_for_password_login, stdin);

    // verific sa nu fie spatii in password : 
    // M-am inspirat de aici : https://stackoverflow.com/questions/41649672/a-check-in-c-language-if-there-is-a-space-inside-the-array-string
     for(int i = 0; password_when_login[i] != '\0'; ++i){
        if(password_when_login[i] == ' '){
            printf("ERROR : You are not allowed to have white spaces in your password when logging in\n");
            spaces_password = 1;
            break;
        }
    }

    if(spaces_password == 1){
        return NULL;
    }

    // eliminam caracterul '\n'
    index = strlen(username_when_login) - 1;
    username_when_login[index] = '\0';

    index = strlen(password_when_login) - 1;
    password_when_login[index] = '\0';

    // formatam mesaju de tip JSON
    JSON_Value *json_value = json_value_init_object();
    JSON_Object *json_elem = json_value_get_object(json_value);

    if (json_elem != NULL) {
        json_object_set_string(json_elem, "username", username_when_login);
        json_object_set_string(json_elem, "password", password_when_login);
    }
    
    array = json_serialize_to_string_pretty(json_value);

    // retinem mesajul trimis catre server
    message_for_server = compute_post_request("34.254.242.81:8080",
     "/api/v1/tema/auth/login", "application/json", &array, 1, NULL, NULL);

    send_to_server(socket, message_for_server);

    // retinem mesajul primit de la server
    message_received_server = receive_from_server(socket); 
    
    // extragem codul de exit folosind strtok in functiile substr si null_substr,
    // dupa cum am facut si la register
    code_to_exit = calloc(10000, sizeof(char));
    memcpy(code_to_exit, message_received_server, strlen(message_received_server));
    
    if (code_to_exit != NULL) {
        int last_pos = strlen(message_received_server);
        code_to_exit[last_pos] = '\0';
        code_to_exit = substr(code_to_exit);
        code_to_exit = null_substr(code_to_exit);
    }

    // in functie de codul de exit afisam diferite mesaje
    int nr = atoi(code_to_exit);
    if (nr != 200) {
        printf("ERROR : Your credentials do not match !!!\n");
        
        returned_value = NULL;
    } else {
        char * starting_cookies = calloc(10000, sizeof(char));
        starting_cookies = memmem(message_received_server, strlen(message_received_server), "Set-Cookie", strlen("Set-Cookie"));
        if(starting_cookies != NULL) {
            starting_cookies = points(starting_cookies, ": ", ": ", ";");
        }
    
        fputs(starting_cookies, stdout);
        printf("\n");
        
        // returned_value va returna fie "" adica NULL sau starting_cookies, unde starting_cookies reprezinta partea de cookies
        returned_value = calloc(strlen(starting_cookies) + 1, sizeof(char));
        memcpy(returned_value, starting_cookies, strlen(starting_cookies));
    }

    // eliberam memoria
    if (username_when_login != NULL) {
        free(username_when_login);
    }

    if (password_when_login != NULL) {
        free(password_when_login);
    }

    close(socket);

    return returned_value;
}

char* library_access_client(char* starting_cookies){
    // deschidem conexiunea cu serverul
    int socket = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);

    // retinem mesajul catre server
    char* message_for_server = compute_get_request("34.254.242.81:8080",
     "/api/v1/tema/library/access", NULL, starting_cookies, NULL);
    char* returned_value = "";
    send_to_server(socket, message_for_server);

    // retinem mesajul primit de la server
    char* message_received_server = receive_from_server(socket); 

    char* code_to_exit = calloc(10000, sizeof(char));
    if(code_to_exit != NULL){
        memcpy(code_to_exit, message_received_server, strlen(message_received_server));
        int last_pos = strlen(message_received_server);
        code_to_exit[last_pos] = '\0';
        code_to_exit = substr(code_to_exit);
        code_to_exit = null_substr(code_to_exit);
    }
    
    int nr = atoi(code_to_exit);
    // in functie de codul de exit afisam diferite mesaje
    if(nr == 400){
        printf("ERROR : Request not suitable for this action !!!\n");
        close(socket);
        return returned_value;
    } else if (nr == 200) {
        char * tkn = calloc(10000, sizeof(char));
        tkn = memmem(message_received_server, strlen(message_received_server), "token", strlen("token"));
        if(tkn != NULL){
            // formatam tkn(token-ul)
            tkn = points(tkn, ":", ":", "\"");
        }
        
        fputs(tkn, stdout);
        printf("\n");

        close(socket);
        returned_value = calloc(strlen(tkn) + 1, sizeof(char));
        memcpy(returned_value, tkn, strlen(tkn));
        return returned_value;
    } else {
        printf("ERROR : You cannot acces this library - you are not authentificated !!! \n");
        close(socket);
        return returned_value;
    }    
}
void read_books_client(char *cookies, char *tkn){
    // deschidem conexiunea cu server-ul
    int socket = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
    
    // retinem mesajul care va fi trimis catre server
    char* message_for_server = compute_get_request("34.254.242.81:8080",
     "/api/v1/tema/library/books", NULL, cookies, tkn);

    // trimitem mesajul catre server
    send_to_server(socket, message_for_server);
    
    //retinem mesajul primit de la server
    char* message_received_server = receive_from_server(socket); 

    // formatam codul de exit  pe baza mesajului primit de la server
    char* code_to_exit = calloc(10000, sizeof(char));
    if(code_to_exit != NULL){
        memcpy(code_to_exit, message_received_server, strlen(message_received_server));
        int last_pos = strlen(message_received_server);
        code_to_exit[last_pos] = '\0';
        code_to_exit = substr(code_to_exit);
        code_to_exit = null_substr(code_to_exit);
    }
    

    int nr = atoi(code_to_exit);
    
    //in functie de codul de exit afisam diverse mesaje fie de succes fie de eroare
    if(nr == 403){
        printf("ERROR : You cannot access this library !!! \n");
    } else if (nr == 200){
        char* library = calloc(1000, sizeof(char));
        library = memmem(message_received_server, strlen(message_received_server), "[", strlen("["));
        fputs(library, stdout);
        printf("\n");
    } else {
        printf("ERROR : You cannot access this library !!! \n");
    }
    close(socket);

}

void adding_books(char* cookies, char* tkn){

    // initiem conexiunea cu serverul si alocam memorie pentru fiecare camp al cartii
    int socket = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
    
    char* title = calloc(150, sizeof(char));
    char* author = calloc(150, sizeof(char));
    char* genre = calloc(150, sizeof(char));
    char* publisher = calloc(150, sizeof(char));
    char* page_count = calloc(150, sizeof(char));
    int page_count_1;
    size_t size_title = 150, size_author = 150, size_genre = 150, size_publisher = 150, size_page_count = 150;

    // alocam memorie pentru fiecare camp al unei carti
    // pentru fiecare camp eliminam \n

    if(tkn == NULL || cookies == NULL){
        printf("ERROR : You cannot access this library !!! \n");
        goto exiting;
    } 
    
        printf("title=");
        getline(&title, &size_title, stdin);
        int position_1 = strlen(title) - 1;
        title[position_1] = '\0';

        printf("author=");
        getline(&author, &size_author, stdin);
        int position_2 = strlen(author) - 1;
        author[position_2] = '\0';
        printf("genre=");
        getline(&genre, &size_genre, stdin);
        int position_3 = strlen(genre) - 1;
        genre[position_3] = '\0';

        printf("publisher=");
        getline(&publisher, &size_publisher, stdin);
        int position_4 = strlen(publisher) - 1;
        publisher[position_4] = '\0';

        printf("page_count=");
        getline(&page_count, &size_page_count, stdin);

        bool true_or_false = Has_characters(page_count);
        if(true_or_false == false){
            goto exiting;
        } else {
            page_count_1 = atoi(page_count);
        }



        
    // formatam mesajul JSON
        JSON_Value *json_value = json_value_init_object();
        JSON_Object *json_elem = json_value_get_object(json_value);
        if(json_elem != NULL){
            if(strlen(title) == 0 || strlen(author) == 0 || strlen(genre) == 0 || strlen(publisher) == 0 || page_count == 0){
                exit(0);
            } else {
                if(strlen(title) > 0){
                    json_object_set_string(json_elem, "title", title);
                }
                if(strlen(author) > 0){
                    json_object_set_string(json_elem, "author", author);
                }
                if(strlen(genre) > 0){
                    json_object_set_string(json_elem, "genre", genre);
                }
                if(strlen(publisher) > 0){
                    json_object_set_string(json_elem, "publisher", publisher);
                }
                json_object_set_number(json_elem, "page_count", page_count_1);
            }
        }
            


    char* array = "";
    array = json_serialize_to_string_pretty(json_value);

    //retinem mesajul pentru server
    char* message_for_server = compute_post_request("34.254.242.81:8080",
     "/api/v1/tema/library/books", "application/json", &array, 1, cookies, tkn);

    // trimitem mesajul catre server
    send_to_server(socket, message_for_server);

    // retinem mesajul primit de la server
    char* message_received_server = receive_from_server(socket); 
    
    char* code_to_exit = calloc(10000, sizeof(char));
    if(code_to_exit != NULL){
        memcpy(code_to_exit, message_received_server, strlen(message_received_server));
        int last_pos = strlen(message_received_server);
        code_to_exit[last_pos] = '\0';
        code_to_exit = substr(code_to_exit);
        code_to_exit = null_substr(code_to_exit);
    }

    // am formatat codul de exit si in functie de acesta afisam atat 
    // de mesaje de succes cat si de eroare    
    int nr = atoi(code_to_exit);
    
    if(nr == 403){
        printf("ERROR : You cannot access this library !!! \n");
    } else if(nr == 200){
        printf("ok !!!\n");
    } else if(nr == 500) {
        printf("ERROR : You cannot access this library !!! \n");
    } else {
        printf("ERROR : Your fields are incorrect !!! \n");
    }

    // eliberam memorie
    if(title != NULL){
        free(title);
    }

    if(author != NULL){
        free(author);
    }
    
    if(genre != NULL){
        free(genre);
    }

    if(publisher != NULL){
        free(publisher);
    } 

    if(page_count != NULL){
        free(page_count);
    }


    json_free_serialized_string(array);
    json_value_free(json_value);

    close(socket);
    exiting:
        close(socket);   
}

void client_gets_a_book(char* cookies, char* tkn){

    // deschidem conexiunea cu serverul
    int socket = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
    size_t size1 = 150;

    // alocam memoria pentru cartea care vrem sa o viualizam
    char* book_name = calloc(150, sizeof(char));

    // retinem id-ul cartii in book_name si eliminam \n

    if(tkn != NULL && cookies != NULL){
        printf("id=");
        getline(&book_name, &size1, stdin);
        int position_1 = strlen(book_name) - 1;
        book_name[position_1] = '\0';
    }
    
    

    // retinem url-ul
    char* url_site = calloc(150, sizeof(char));
    memcpy(url_site, "/api/v1/tema/library/books/", 30);
    
    int length = strlen(url_site);
    url_site[length] = '\0';
    url_site = strcat(url_site, book_name);

    // retinem mesajul pentru server
    char* message_for_server = compute_get_request("34.254.242.81:8080",
     url_site, NULL, cookies, tkn);

    // trimitem emsajul catre server
    send_to_server(socket, message_for_server);    

    // retinem mesajul primit de la server
    char* message_received_server = receive_from_server(socket);

    // formatam codul de exit
    char* code_to_exit = calloc(10000, sizeof(char));
    memcpy(code_to_exit, message_received_server, strlen(message_received_server));

    if(code_to_exit != NULL){
        int last_pos = strlen(message_received_server);
        code_to_exit[last_pos] = '\0';
        code_to_exit = substr(code_to_exit);
        code_to_exit = null_substr(code_to_exit);
    }

    
    // in functie de codul de exit convertit la intreg afisam aatt mesaje de succes cat si de eroare
    int nr = atoi(code_to_exit);
    if(nr == 400 || nr == 404){
        printf("ERROR : Your fields are incorrect - There is no such book with this id\n");
    } else if(nr == 200){
        // formatam stringul care va fi afisat in caz de avem cod de exit 200, strng-ul va contine 
        // toate campurile unei carti intre o pereche de paranteze patrate []
        
        char* book_of_choice = calloc(1000, sizeof(char));
        book_of_choice = memmem(message_received_server, strlen(message_received_server), "[", strlen("["));
        char* position = strchr(message_received_server, '{');
        int index = position - message_received_server;
        book_of_choice = message_received_server + index;
        char* new_book_of_choice = calloc(1000, sizeof(char));
        strcpy(new_book_of_choice, "[");
        strcat(new_book_of_choice, book_of_choice);
        strcat(new_book_of_choice, "]");
        fputs(new_book_of_choice, stdout);
        printf("\n");
    } else {
        printf("ERROR : You cannot access this library !!!\n");
    }

    // eliberam memorie
    if(url_site != NULL){
        free(url_site);
    }

    if(book_name != NULL){
        free(book_name);
    }
   
    // inchidem conexiunea cu serverul
    close(socket);

}


void delete_book(char* cookies, char* tkn){

    // initiem conexiunea cu serverul
    int socket = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
    size_t size1 = 150;

    // alocam memorie pentru cartea pe care vrem sa o stergem
    char* book_name = calloc(150, sizeof(char));

    if(tkn != NULL && cookies != NULL){
        printf("id=");
        // retinem numele cartii pe care vrem sa o stergem in book_name
        getline(&book_name, &size1, stdin);
        int position = strlen(book_name) - 1;
        book_name[position] = '\0';
    }
    

    char* url_site = calloc(150, sizeof(char));
    // retinem url-ul si eliminam \n
    memcpy(url_site, "/api/v1/tema/library/books/", 30);
    int position_2 = strlen(url_site);
    url_site[position_2] = '\0';
    url_site = strcat(url_site, book_name);

    // retinem mesajul pentru server
    char *message_for_server = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // in liniile de cod ce urmeaza returnam lungimea finala a 
    // stringurilor oferite de user pentru a depozita 
    // rezultatul si setam respectiv initializam unele string-uri
    sprintf(line, "DELETE %s HTTP/1.1", url_site);

    strcat(message_for_server, line);
    strcat(message_for_server, "\r\n");

    memset(line, 0, LINELEN);
    sprintf(line, "HOST: %s ", "34.254.242.81:8080");
    strcat(message_for_server, line);
    strcat(message_for_server, "\r\n");

    if (tkn != NULL) {
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Delimiter %s", tkn);
        strcat(message_for_server, line);
        strcat(message_for_server, "\r\n");
    }

    if (cookies != NULL) {
        memset(line, 0, LINELEN);
        sprintf(line, "Cookie: %s", cookies);
        strcat(message_for_server, line);
        strcat(message_for_server, "\r\n");
    }
    strcat(message_for_server, "");
    strcat(message_for_server, "\r\n");

    // trimitem mesajul catre server
    send_to_server(socket, message_for_server);    

    char* message_received_server = receive_from_server(socket);
    // retinem mesajul primit de la server

    char* code_to_exit = calloc(10000, sizeof(char));
    memcpy(code_to_exit, message_received_server, strlen(message_received_server));
    if(code_to_exit != NULL){
         int last_pos = strlen(message_received_server);
        code_to_exit[last_pos] = '\0';
        code_to_exit = substr(code_to_exit);
        code_to_exit = null_substr(code_to_exit);
    }
    // formatam codul de exit si pe baza acestuia convertit la int 
    // intoarcem atat mesaje de succes cat si de eroare
    
    int nr = atoi(code_to_exit);
    if(nr == 403 || nr == 404){
        printf("ERROR : You cannot access this library !!!\n");
    } else if(nr == 200){
        printf("ok!!!\n");
    } else {
        printf("ERROR : You cannot access this library !!!\n");
    }

    // eliberam memoria si inchidem conexiunea cu serverul
    if(url_site != NULL){
        free(url_site);
    }

    if(book_name != NULL){
        free(book_name);
    }
    close(socket);

}

void logout(char* cookies, char* tkn){

    // initiem conexiunea cu serverul
    int socket = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);

    // retinem mesajul pentru server
    char* message_for_server = compute_get_request("34.254.242.81:8080",
     "/api/v1/tema/auth/logout", 0, cookies, tkn);

    // trimitem mesajul catre server
    send_to_server(socket, message_for_server);    

    // retinem mesajul primit de la server
    char* message_received_server = receive_from_server(socket);

    char* code_to_exit = calloc(10000, sizeof(char));
    memcpy(code_to_exit, message_received_server, strlen(message_received_server));
    if(code_to_exit != NULL){
        int last_pos = strlen(message_received_server);
        code_to_exit[last_pos] = '\0';
        code_to_exit = substr(code_to_exit);
        code_to_exit = null_substr(code_to_exit);
    }

    // am formatat codul de exit si pe baza acestuia afisam mesaje de succes sau de eroare
    
    int nr = atoi(code_to_exit);
    if(nr == 400 || nr == 404){
        printf("ERROR : You have to log in first - you are not authentificated !!!\n");
    } else if(nr == 200){
        if(message_received_server != NULL){
            fputs(message_received_server, stdout);
            printf("\n");
        }
        printf("ok!!!\n");

    } else {
        printf("ERROR : Your request is not suitable for this action !!!\n");
    }

    // inchidem conexiunea cu serverul
    close(socket);

}



int main(int argc, char *argv[]){

char* input = calloc(30, sizeof(char));
char* cookies = NULL;
char* tkn = NULL;
size_t size_input = 30;
int exit_value;

for(;;){    
    memset(input, 0, 30);
    // citesc de la tastatura comanda si in functie de aceasta apelez diferite functii pentru efectuarea comenzilor
    getline(&input, &size_input, stdin);
    
    switch(input[0]){
        case 'e':
            if(strcmp(input, "exit\n") == 0){
                exit_value = 1;
                goto breaking_loop;
                break;
            }
             else if(strcmp(input, "enter_library\n") == 0){
                tkn = library_access_client(cookies);
            }
            break;
        case 'l':
            if(strcmp(input, "login\n") == 0){
                cookies = login_client();
            } else if(strcmp(input, "logout\n") == 0 && logged_in == 1){ 
                // in caz de utlilizatorul s-a logat si vrea sa dea logout 
                // ii permitem sa acceseze functia care afectueaza operatia de logout
                logout(cookies, tkn);
                cookies = "";
                tkn = "";
            } else if(strcmp(input, "logout\n") == 0 && logged_in == 0){
                // in caz de utilizatorul nu s-a logat si vrea sa dea logout 
                // afisam un mesaj corespunzator
                printf("ERROR : You need to authentificate first !!! \n");
            } 
            break;
        case 'g':
            if(strcmp(input, "get_books\n") == 0){
                read_books_client(cookies, tkn);
            } else if(strcmp(input, "get_book\n") == 0){
                client_gets_a_book(cookies, tkn);
            }    
            break;
        case 'r':
            if(strcmp(input, "register\n") == 0){
                register_client();
            }
            break;
        case 'a':
            if(strcmp(input, "add_book\n") == 0){ 
                adding_books(cookies, tkn);
            } 
            break;
        case 'd':
            if(strcmp(input, "delete_book\n") == 0){
                delete_book(cookies, tkn);
            } 
            break;
    }

    // am folosit goto ca sa ies din loop-ul infinit al for-ului
    breaking_loop:
            if(exit_value > 0){
                break;
            }
}

// eliberez memoria
free(input);

return 0;

}
