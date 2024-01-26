// client_studente.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define MAX_BUFFER_SIZE 1024

void check_exam_availability(int client_socket) {
    // Invia la richiesta di verifica disponibilità esami al server
    int request_type = 1;
    send(client_socket, &request_type, sizeof(request_type), 0);

    // Ricevi e stampa le date degli esami disponibili dalla segreteria
    char exam_dates[MAX_BUFFER_SIZE];
    recv(client_socket, exam_dates, sizeof(exam_dates), 0);
    printf("Date degli esami disponibili:\n%s\n", exam_dates);
}

void reserve_exam_student(int client_socket) {
    // Invia la richiesta di prenotazione esame al server
    int request_type = 2;
    send(client_socket, &request_type, sizeof(request_type), 0);

    // Ricevi e stampa la conferma dalla segreteria
    char confirmation[MAX_BUFFER_SIZE];
    recv(client_socket, confirmation, sizeof(confirmation), 0);
    printf("Segreteria: %s\n", confirmation);
}

int main() {
    int client_socket;
    struct sockaddr_in server_addr;

    // Inizializzazione del socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    // Configurazione dell'indirizzo del server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    // Connessione al server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Errore nella connessione al server");
        exit(EXIT_FAILURE);
    }

    // Esempi di operazioni dello studente
    check_exam_availability(client_socket);
    reserve_exam_student(client_socket);

    close(client_socket);

    return 0;
}
