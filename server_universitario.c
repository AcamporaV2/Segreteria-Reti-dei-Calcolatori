#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define SERVER_PORT 12345      // Porta del server
#define FILE_PATH "esami.txt"  // Percorso del file degli esami

// Definizione della struttura per gli esami
struct Esame {
    char nome[100];
    char data[100];
};

typedef enum {
    AGGIUNTA_ESAME
} TipoRichiesta;


// Funzione per aggiungere un esame al file
void aggiungi_esame(struct Esame esame) {
    FILE *file = fopen(FILE_PATH, "a");
    if (file == NULL) {
        perror("Errore nell'apertura del file");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "%s %s\n", esame.nome, esame.data);
    fclose(file);
}

// Funzione per rimuovere un esame dal file
void rimuovi_esame(struct Esame esame) {
    FILE *file = fopen(FILE_PATH, "r");
    if (file == NULL) {
        perror("Errore nell'apertura del file");
        exit(EXIT_FAILURE);
    }

    FILE *temp = fopen("temp.txt", "w");
    if (temp == NULL) {
        perror("Errore nella creazione del file temporaneo");
        exit(EXIT_FAILURE);
    }

    char nome[100], data[100];
    while (fscanf(file, "%s %s", nome, data) != EOF) {
        if (strcmp(nome, esame.nome) != 0 || strcmp(data, esame.data) != 0) {
            fprintf(temp, "%s %s\n", nome, data);
        }
    }

    fclose(file);
    fclose(temp);

    if (remove(FILE_PATH) != 0) {
        perror("Errore nella rimozione del file");
        exit(EXIT_FAILURE);
    }

    if (rename("temp.txt", FILE_PATH) != 0) {
        perror("Errore nella rinomina del file temporaneo");
        exit(EXIT_FAILURE);
    }
}

// Funzione per gestire la richiesta di aggiunta di un esame
void gestisci_aggiunta_esame(int client_socket, char* buffer) {
    // Analizza la stringa ricevuta dal client per ottenere il nome e la data dell'esame
    char nome[100], data[100];
    sscanf(buffer, "AGGIUNGI_ESAME %s %s", nome, data);

    // Crea un'istanza di struct Esame con i dati ottenuti
    struct Esame esame;
    strcpy(esame.nome, nome);
    strcpy(esame.data, data);

    // Qui puoi implementare la logica per aggiungere l'esame al sistema
    printf("Ricevuta richiesta di aggiunta esame: %s - %s\n", esame.nome, esame.data);
    // Esempio: aggiungi l'esame a un file
    aggiungi_esame(esame);
}

// Funzione per gestire la richiesta di rimozione di un esame
void gestisci_rimozione_esame(int client_socket, struct Esame esame) {
    // Qui puoi implementare la logica per rimuovere l'esame dal sistema
    printf("Ricevuta richiesta di rimozione esame: %s - %s\n", esame.nome, esame.data);
    // Esempio: rimuovi l'esame da un file
    rimuovi_esame(esame);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    struct Esame esame;

    // Creazione del socket del server
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    // Impostazione dei dettagli del server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    // Bind del socket all'indirizzo e alla porta del server
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Errore nel binding del socket");
        exit(EXIT_FAILURE);
    }

    // Mettere il server in ascolto per le connessioni in entrata
    if (listen(server_socket, 5) == -1) {
        perror("Errore nell'ascolto delle connessioni");
        exit(EXIT_FAILURE);
    }

    printf("Server universitario in ascolto...\n");

    while (1) {
        // Accettare le connessioni dai client
        client_addr_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            perror("Errore nell'accettare la connessione");
            exit(EXIT_FAILURE);
        }

        // Ricevere il tipo di richiesta dal client
        TipoRichiesta tipo_richiesta;
        recv(client_socket, &tipo_richiesta, sizeof(tipo_richiesta), 0);

        // Gestire la richiesta in base al tipo ricevuto
    switch (tipo_richiesta) {
    case AGGIUNTA_ESAME: {
        // Ricevere i dati dell'esame dal client
        char buffer[1024];
        recv(client_socket, buffer, sizeof(buffer), 0);
        gestisci_aggiunta_esame(client_socket, buffer);
        break;
    }
    default:
        // Gestire un tipo di richiesta non valido
        printf("Tipo di richiesta non valido\n");
}

        
        // Chiudi il socket del client dopo aver gestito la richiesta
        close(client_socket);
    }

    // Chiudi il socket del server alla fine dell'esecuzione
    close(server_socket);
    return 0;
}
