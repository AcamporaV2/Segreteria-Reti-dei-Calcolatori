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

void handle_exam_reservation(int client_socket, char *data) {
    // Gestisci la prenotazione di un esame
    // Implementa la logica di prenotazione e invia una conferma al client
    // Puoi, ad esempio, cercare l'esame nel vettore exams e aggiornare lo stato di prenotazione
    // In questo esempio, la funzione handle_exam_reservation assume che il client invii il nome dell'esame da prenotare.
    char response[1024];
    snprintf(response, sizeof(response), "Prenotazione per l'esame %s confermata!\n", data);
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

    // Inizializza il socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Errore durante la creazione del socket");
        exit(EXIT_FAILURE);
    }

    // Configura l'indirizzo del server
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Collega il socket al server
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

    while (1) {
        // Accetta la connessione dal client
        socklen_t client_address_len = sizeof(client_address);
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        if (client_socket == -1) {
            perror("Errore durante l'accettazione della connessione");
            exit(EXIT_FAILURE);
        }

        printf("Connessione accettata da %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        // Gestisci la richiesta del client
        char request[1024];
        read(client_socket, request, sizeof(request));

        // Aggiornato: Analizza la richiesta del client
        if (strncmp(request, "GET_EXAM_DATES", 14) == 0) {
            handle_exam_request(client_socket);
        } else if (strncmp(request, "ADD_EXAM", 8) == 0) {
            handle_add_exam(client_socket, request + 9);
        } else if (strncmp(request, "RESERVE_EXAM", 12) == 0) {
            handle_exam_reservation(client_socket, request + 13);
        }

        // Chiudi la connessione con il client
        close(client_socket);
    }

    // Chiudi il socket del server
    close(server_socket);

    return 0;
}
