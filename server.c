#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_EXAMS 100
#define PORT 8080

typedef struct {
    char course_name[50];
    char exam_date[50];
} Exam;

Exam exams[MAX_EXAMS];
int exam_count = 0;

void handle_exam_request(int client_socket) {
    // Invia al client la lista delle date degli esami disponibili
    char response[1024];
    strcpy(response, "Date degli esami disponibili:\n");
    for (int i = 0; i < exam_count; ++i) {
        strcat(response, exams[i].exam_date);
        strcat(response, "\n");
    }
    write(client_socket, response, strlen(response));
}

void handle_add_exam(int client_socket, char *data) {
    // Aggiungi un nuovo esame alla lista
    Exam new_exam;
    sscanf(data, "%s %s", new_exam.course_name, new_exam.exam_date);
    exams[exam_count++] = new_exam;

    // Invia conferma al client
    char response[] = "Esame aggiunto con successo!\n";
    write(client_socket, response, strlen(response));
}

void load_exams_from_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Errore durante l'apertura del file degli esami");
        exit(EXIT_FAILURE);
    }

    while (fscanf(file, "%s %s", exams[exam_count].course_name, exams[exam_count].exam_date) == 2) {
        exam_count++;
        if (exam_count >= MAX_EXAMS) {
            fprintf(stderr, "Limite massimo di esami raggiunto.\n");
            break;
        }
    }

    fclose(file);
}

int main() {
    load_exams_from_file("exam_dates.txt");

    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;

    // Inizializza il socket del server
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Errore durante la creazione del socket");
        exit(EXIT_FAILURE);
    }

    // Configura l'indirizzo del server
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Collega il socket del server all'indirizzo
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Errore durante il binding del socket");
        exit(EXIT_FAILURE);
    }

    // Inizia ad ascoltare le connessioni in entrata
    if (listen(server_socket, 5) == -1) {
        perror("Errore durante l'ascolto delle connessioni in entrata");
        exit(EXIT_FAILURE);
    }

    printf("Server in attesa di connessioni...\n");

    // Inizializza l'insieme di fd_set
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(server_socket, &read_fds);

    while (1) {
        // Copia l'insieme di fd_set per usarlo nella select
        fd_set temp_fds = read_fds;

        // Esegui la select
        if (select(FD_SETSIZE, &temp_fds, NULL, NULL, NULL) == -1) {
            perror("Errore nella select");
            exit(EXIT_FAILURE);
        }

        // Controlla se ci sono nuove connessioni in entrata
        if (FD_ISSET(server_socket, &temp_fds)) {
            // Accetta la connessione dal client
            socklen_t client_address_len = sizeof(client_address);
            client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
            if (client_socket == -1) {
                perror("Errore durante l'accettazione della connessione");
                exit(EXIT_FAILURE);
            }

            printf("Connessione accettata da %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

            // Aggiungi il nuovo socket client all'insieme di fd_set
            FD_SET(client_socket, &read_fds);
        }

        // Gestisci le richieste dei client esistenti
        for (int i = server_socket + 1; i < FD_SETSIZE; ++i) {
            if (FD_ISSET(i, &temp_fds)) {
                char request[1024];
                ssize_t bytes_read = read(i, request, sizeof(request));

                if (bytes_read <= 0) {
                    // Errore o connessione chiusa
                    printf("Connessione chiusa da %d\n", i);
                    close(i);
                    FD_CLR(i, &read_fds);
                } else {
                    // Analizza la richiesta del client
                    if (strncmp(request, "GET_EXAM_DATES", 14) == 0) {
                        handle_exam_request(i);
                    } else if (strncmp(request, "ADD_EXAM", 8) == 0) {
                        handle_add_exam(i, request + 9);
                    }
                }
            }
        }
    }

    // Chiudi il socket del server
    close(server_socket);

    return 0;
}
