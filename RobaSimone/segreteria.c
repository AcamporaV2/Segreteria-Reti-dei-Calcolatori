#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "wrapper.h"

#define SERVER_IP "127.0.0.1" // Indirizzo IP del server
#define SERVER_PORT 12345      // Porta del server

int ascolto_studenti(int segreteria_ascolto_socket, struct sockaddr_in *indirizzo_segreteria);
void esami_disponibili(struct Esame esame, int segreteria_connessione_socket);
void invio_esame_server(int socket_esami, struct Richiesta ricezione_esami);
int connessione_universita(int socket_esami, struct sockaddr_in *indirizzo_universita);

// Definizione della struttura per gli esami e della richiesta
struct Esame {
    char nome[100];
    char data[100];
};

struct Richiesta{
    int TipoRichiesta;
    struct Esame esame;
};

int ascolto_studenti(int segreteria_ascolto_socket, struct sockaddr_in *indirizzo_segreteria) {

    segreteria_ascolto_socket = Socket(AF_INET, SOCK_STREAM, 0);

    (*indirizzo_segreteria).sin_family = AF_INET;
    (*indirizzo_segreteria).sin_addr.s_addr = htonl(INADDR_ANY);
    (*indirizzo_segreteria).sin_port = htons(2000);

    Bind(segreteria_ascolto_socket, (struct sockaddr *)indirizzo_segreteria, sizeof(*indirizzo_segreteria));
    Ascolta(segreteria_ascolto_socket, 10);

    return segreteria_ascolto_socket;
}

void esami_disponibili(struct Esame esame, int segreteria_connessione_socket) {

    int socket_esami;
    struct sockaddr_in indirizzo_universita;
    struct Richiesta ricezione_esami;

    ricezione_esami.richiesta = 2;
    ricezione_esami.esame = esame;
    socket_esami = connessione_universita(socket_esami, &indirizzo_universita);
    invio_esame_server(socket_esami, ricezione_esami);
}

void invio_esame_server(int socket_esami, struct Richiesta ricezione_esami) {

    if(write(socket_esami, &ricezione_esami, sizeof(ricezione_esami)) != sizeof(ricezione_esami)) {
        perror("Errore invio esame");
        exit(1);
    }

}

int connessione_universita(int socket_esami, struct sockaddr_in *indirizzo_universita) {

    socket_esami = Socket(AF_INET, SOCK_STREAM, 0);

    (*indirizzo_universita).sin_family = AF_INET;
    (*indirizzo_universita).sin_port = htons(3000);

    if(inet_pton(AF_INET, "127.0.0.1", &(*indirizzo_universita).sin_addr) <= 0) {
        fprintf(stderr, "Errore inet 127.0.0.1");
        exit(1);
    }

    Connetti(socket_esami, (struct sockaddr *) indirizzo_universita, sizeof(*indirizzo_universita));

    return socket_esami;
}

int main() {
    int segreteria_connessione_socket;
    int segreteria_ascolto_socket;
    int selezione;
    struct sockaddr_in indirizzo_segreteria;
    struct Richiesta richiesta_ricevuta;

    segreteria_ascolto_socket = ascolto_studenti(segreteria_ascolto_socket, &indirizzo_segreteria);

    while (1) {

        segreteria_connessione_socket = Accetta(segreteria_ascolto_socket, (struct sockaddr*) NULL, NULL);

        if (read(segreteria_connesione_socket, &richiesta_ricevuta, sizeof(struct Richiesta)) != sizeof(struct Richiesta)) {
            perror("read error 1");
            exit(-1);
        }

        if (richiesta_ricevuta.TipoRichiesta = 1) {
            esami_disponibili(richiesta_ricevuta.esame, segreteria_connesione_socker);
        }

        // printf("\nCiao, benvenuto nella segreteria, cosa vorresti fare?\n");
        // printf("1 - Inserisci esame\n");
        // printf("2 - Rimuovi esame\n");
        // printf("3 - Richiesta di prenotazioni esami\n");
        // printf("4 - Fornisci date disponibili\n");
        // printf("5 - Esci\n");
        // printf("============================================\n");

    //     scanf("%d", &selezione);
    //     getchar(); // Consuma il carattere di nuova linea rimasto nel buffer
    //
    //     switch (selezione) {
    //     case 1:
    //         // Input dal segretario: nome e data dell'esame
    //         printf("Inserisci il nome dell'esame: ");
    //         fgets(nome, sizeof(nome), stdin);
    //         nome[strcspn(nome, "\n")] = '\0'; // Rimuove il carattere di nuova linea
    //
    //         printf("Inserisci la data dell'esame: ");
    //         fgets(data, sizeof(data), stdin);
    //         data[strcspn(data, "\n")] = '\0'; // Rimuove il carattere di nuova linea
    //
    //         // Copia i dati nella struttura
    //         strncpy(esame.nome, nome, sizeof(esame.nome) - 1);
    //         esame.nome[sizeof(esame.nome) - 1] = '\0';
    //         strncpy(esame.data, data, sizeof(esame.data) - 1);
    //         esame.data[sizeof(esame.data) - 1] = '\0';
    //
    //         // Invia il tipo di richiesta al server
    //         {
    //             TipoRichiesta tipo_richiesta = AGGIUNTA_ESAME;
    //             if (send(segreteria_socket, &tipo_richiesta, sizeof(tipo_richiesta), 0) == -1) {
    //                 perror("Errore nell'invio del tipo di richiesta");
    //                 close(segreteria_socket);
    //                 exit(EXIT_FAILURE);
    //             }
    //
    //             // Invia i dati dell'esame
    //             invia_richiesta_aggiunta_esame(segreteria_socket, esame);
    //         }
    //         break;
    //     case 2:
    //         // Input dal segretario: nome e data dell'esame
    //         printf("Inserisci il nome dell'esame da rimuovere: ");
    //         fgets(nome, sizeof(nome), stdin);
    //         nome[strcspn(nome, "\n")] = '\0'; // Rimuove il carattere di nuova linea
    //
    //         printf("Inserisci la data dell'esame da rimuovere: ");
    //         fgets(data, sizeof(data), stdin);
    //         data[strcspn(data, "\n")] = '\0'; // Rimuove il carattere di nuova linea
    //
    //         // Copia i dati nella struttura
    //         strncpy(esame.nome, nome, sizeof(esame.nome) - 1);
    //         esame.nome[sizeof(esame.nome) - 1] = '\0';
    //         strncpy(esame.data, data, sizeof(esame.data) - 1);
    //         esame.data[sizeof(esame.data) - 1] = '\0';
    //
    //         // Invia il tipo di richiesta al server
    //         {
    //             TipoRichiesta tipo_richiesta = RIMOZIONE_ESAME;
    //             if (send(segreteria_socket, &tipo_richiesta, sizeof(tipo_richiesta), 0) == -1) {
    //                 perror("Errore nell'invio del tipo di richiesta");
    //                 close(segreteria_socket);
    //                 exit(EXIT_FAILURE);
    //             }
    //
    //             // Invia i dati dell'esame
    //             invia_richiesta_rimozione_esame(segreteria_socket, esame);
    //         }
    //         break;
    //     case 5:
    //         // Uscita dal loop e chiusura del socket
    //         printf("Uscita dal programma.\n");
    //         close(segreteria_socket);
    //         exit(EXIT_SUCCESS);
    //     default:
    //         printf("Selezione non valida. Per favore, scegli di nuovo.\n");
    //         break;
    //     }
    // }

    // Chiusura della connessione
    close(segreteria_socket);

    return 0;
}

/*
void invia_richiesta_aggiunta_esame(int server_socket, struct Esame esame) {
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "AGGIUNGI_ESAME %s %s", esame.nome, esame.data);
    if (send(server_socket, buffer, strlen(buffer), 0) == -1) {
        perror("Errore nell'invio del messaggio");
        exit(EXIT_FAILURE);
    }
    printf("Richiesta di aggiunta esame inviata al server.\n");
}

void invia_richiesta_rimozione_esame(int server_socket, struct Esame esame) {
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "RIMOZIONE_ESAME %s %s", esame.nome, esame.data);
    if (send(server_socket, buffer, strlen(buffer), 0) == -1) {
        perror("Errore nell'invio del messaggio");
        exit(EXIT_FAILURE);
    }
    printf("Richiesta di rimozione esame inviata al server.\n");
}*/
