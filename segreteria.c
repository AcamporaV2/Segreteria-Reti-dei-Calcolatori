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
    AGGIUNTA_ESAME
}TipoRichiesta;

// Funzione per inviare la richiesta di aggiunta di un esame al server
void invia_richiesta_aggiunta_esame(int server_socket, struct Esame esame) {
    char buffer[1024];
    
    // Costruzione del messaggio da inviare al server
    sprintf(buffer, "AGGIUNGI_ESAME %s %s", esame.nome, esame.data);
    
    // Invio del messaggio al server
    send(server_socket, buffer, strlen(buffer), 0);
    
    printf("Richiesta di aggiunta esame inviata al server.\n");
}

int main() {
    int segreteria_socket;
    int selezione;
    struct sockaddr_in server_addr;
    struct Esame nuovo_esame;
    
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
    
    // Errore della connessione al server
    if (connect(segreteria_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Errore nella connessione al server");
        exit(EXIT_FAILURE);
    }

    while(1) {
        printf("\nCiao, benvenuto nella segreteria, cosa vorresti fare?\n");
        printf("1 - Inserisci esame\n");
        printf("2 - Rimuovi esame\n");
        printf("3 - Richiesta di prenotazioni esami\n");
        printf("4 - Fornisci date disponibili\n");
        printf("============================================\n");

        scanf("%d", &selezione);

        switch (selezione) {
        case 1:
            // Input dal segretario: nome e data dell'esame
            printf("Inserisci il nome dell'esame: ");
            scanf("%s", nuovo_esame.nome);
            printf("\nInserisci la data dell'esame: ");
            scanf("%s", nuovo_esame.data);

            // Invia il tipo di richiesta al server
            TipoRichiesta tipo_richiesta = AGGIUNTA_ESAME;
            send(segreteria_socket, &tipo_richiesta, sizeof(tipo_richiesta), 0);

            // Invia i dati dell'esame
            invia_richiesta_aggiunta_esame(segreteria_socket, nuovo_esame);
            break;
        default:
            break;
        }

}
    // Chiusura della connessione
    close(segreteria_socket);
    
    return 0;
}
