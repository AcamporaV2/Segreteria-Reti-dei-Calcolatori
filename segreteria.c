#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1" // Indirizzo IP del server
#define SERVER_PORT 12345      // Porta del server

// Definizione della struttura per gli esami
struct Esame {
    char nome[100];
    char data[100];
};

typedef enum {
    AGGIUNTA_ESAME,
    RIMOZIONE_ESAME
} TipoRichiesta;

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
}


int main() {
    int segreteria_socket;
    int selezione;
    struct sockaddr_in server_addr;
    struct Esame esame;
    char nome[100];
    char data[100];

    // Creazione del socket
    segreteria_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (segreteria_socket == -1) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    // Impostazione dei dettagli del server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connessione al server
    if (connect(segreteria_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Errore nella connessione al server");
        close(segreteria_socket);
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("\nCiao, benvenuto nella segreteria, cosa vorresti fare?\n");
        printf("1 - Inserisci esame\n");
        printf("2 - Rimuovi esame\n");
        printf("3 - Richiesta di prenotazioni esami\n");
        printf("4 - Fornisci date disponibili\n");
        printf("5 - Esci\n");
        printf("============================================\n");

        scanf("%d", &selezione);
        getchar(); // Consuma il carattere di nuova linea rimasto nel buffer

        switch (selezione) {
        case 1:
            // Input dal segretario: nome e data dell'esame
            printf("Inserisci il nome dell'esame: ");
            fgets(nome, sizeof(nome), stdin);
            nome[strcspn(nome, "\n")] = '\0'; // Rimuove il carattere di nuova linea

            printf("Inserisci la data dell'esame: ");
            fgets(data, sizeof(data), stdin);
            data[strcspn(data, "\n")] = '\0'; // Rimuove il carattere di nuova linea

            // Copia i dati nella struttura
            strncpy(esame.nome, nome, sizeof(esame.nome) - 1);
            esame.nome[sizeof(esame.nome) - 1] = '\0';
            strncpy(esame.data, data, sizeof(esame.data) - 1);
            esame.data[sizeof(esame.data) - 1] = '\0';

            // Invia il tipo di richiesta al server
            {
                TipoRichiesta tipo_richiesta = AGGIUNTA_ESAME;
                if (send(segreteria_socket, &tipo_richiesta, sizeof(tipo_richiesta), 0) == -1) {
                    perror("Errore nell'invio del tipo di richiesta");
                    close(segreteria_socket);
                    exit(EXIT_FAILURE);
                }

                // Invia i dati dell'esame
                invia_richiesta_aggiunta_esame(segreteria_socket, esame);
            }
            break;
        case 2:
            // Input dal segretario: nome e data dell'esame
            printf("Inserisci il nome dell'esame da rimuovere: ");
            fgets(nome, sizeof(nome), stdin);
            nome[strcspn(nome, "\n")] = '\0'; // Rimuove il carattere di nuova linea

            printf("Inserisci la data dell'esame da rimuovere: ");
            fgets(data, sizeof(data), stdin);
            data[strcspn(data, "\n")] = '\0'; // Rimuove il carattere di nuova linea

            // Copia i dati nella struttura
            strncpy(esame.nome, nome, sizeof(esame.nome) - 1);
            esame.nome[sizeof(esame.nome) - 1] = '\0';
            strncpy(esame.data, data, sizeof(esame.data) - 1);
            esame.data[sizeof(esame.data) - 1] = '\0';

            // Invia il tipo di richiesta al server
            {
                TipoRichiesta tipo_richiesta = RIMOZIONE_ESAME;
                if (send(segreteria_socket, &tipo_richiesta, sizeof(tipo_richiesta), 0) == -1) {
                    perror("Errore nell'invio del tipo di richiesta");
                    close(segreteria_socket);
                    exit(EXIT_FAILURE);
                }

                // Invia i dati dell'esame
                invia_richiesta_rimozione_esame(segreteria_socket, esame);
            }
            break;
        case 5:
            // Uscita dal loop e chiusura del socket
            printf("Uscita dal programma.\n");
            close(segreteria_socket);
            exit(EXIT_SUCCESS);
        default:
            printf("Selezione non valida. Per favore, scegli di nuovo.\n");
            break;
        }
    }

    // Chiusura della connessione
    close(segreteria_socket);

    return 0;
}
