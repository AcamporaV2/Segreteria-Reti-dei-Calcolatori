#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1" // Indirizzo IP del server
#define SERVER_PORT 12345      // Porta del server

// Funzione per inviare la richiesta di informazioni sugli esami alla segreteria
void richiedi_informazioni_esami(int segreteria_socket) {
    // Invia una richiesta di informazioni sugli esami al server
    char buffer[1024];
    sprintf(buffer, "INFORMAZIONI_ESAMI");
    send(segreteria_socket, buffer, strlen(buffer), 0);
    printf("Richiesta di informazioni sugli esami inviata al server.\n");
}

int main() {
    int studente_socket;
    struct sockaddr_in server_addr;

    // Creazione del socket
    studente_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (studente_socket == -1) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    // Impostazione dei dettagli del server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connessione alla segreteria
    if (connect(studente_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Errore nella connessione al server");
        exit(EXIT_FAILURE);
    }

    printf("Connessione alla segreteria stabilita.\n");

    // Richiedi informazioni sugli esami disponibili
    richiedi_informazioni_esami(studente_socket);

    // Chiusura della connessione
    close(studente_socket);

    return 0;
}
