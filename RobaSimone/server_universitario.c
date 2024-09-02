#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "wrapper.h"

// Definizione delle strutture
struct Esame {
    char nome[100];
    char data[100];
};

struct Richiesta {
    int TipoRichiesta;
    struct Esame esame;
};

// Funzioni dichiarate
void aggiungi_esame_file(struct Esame esame);
int gestisci_prenotazione(int universita_connessione_socket, struct Richiesta richiesta_ricevuta);

int main() {
    int universita_connessione_socket;
    int universita_ascolto_socket;
    struct sockaddr_in indirizzo_universita;
    struct Richiesta richiesta_ricevuta;

    // Creazione del socket di ascolto
    universita_ascolto_socket = Socket(AF_INET, SOCK_STREAM, 0);

    indirizzo_universita.sin_family = AF_INET;
    indirizzo_universita.sin_addr.s_addr = htonl(INADDR_ANY);
    indirizzo_universita.sin_port = htons(6940);

    // Associazione del socket all'indirizzo e porta
    Bind(universita_ascolto_socket, (struct sockaddr *) &indirizzo_universita, sizeof(indirizzo_universita));

    // Inizio dell'ascolto
    Ascolta(universita_ascolto_socket, 10);
    printf("Server in ascolto sulla porta 6940...\n");

    while (1) {
        // Accettazione di una connessione in ingresso
        universita_connessione_socket = Accetta(universita_ascolto_socket, (struct sockaddr *)NULL, NULL);

        // Creazione di un processo figlio per gestire la richiesta
        pid_t pid = fork();
        if (pid < 0) {
            perror("Errore nella fork");
            close(universita_connessione_socket);
            continue; // Prosegue con il prossimo ciclo
        }

        if (pid == 0) { // Processo figlio
            close(universita_ascolto_socket);

            // Lettura della richiesta dal client
            ssize_t bytes_read = read(universita_connessione_socket, &richiesta_ricevuta, sizeof(struct Richiesta));
            if (bytes_read != sizeof(struct Richiesta)) {
                perror("Errore nella lettura della richiesta");
                close(universita_connessione_socket);
                exit(EXIT_FAILURE);
            }

            // Gestione della richiesta in base al tipo
            if (richiesta_ricevuta.TipoRichiesta == 1) {
                aggiungi_esame_file(richiesta_ricevuta.esame);
            } else if (richiesta_ricevuta.TipoRichiesta == 2) {
                gestisci_prenotazione(universita_connessione_socket, richiesta_ricevuta);
            } else {
                fprintf(stderr, "Tipo di richiesta non valido\n");
            }

            // Chiusura del socket di connessione
            close(universita_connessione_socket);
            exit(EXIT_SUCCESS);
        } else { // Processo padre
            close(universita_connessione_socket);
        }
    }

    // Chiusura del socket di ascolto
    close(universita_ascolto_socket);
    return 0;
}

// Funzione per aggiungere un esame al file
void aggiungi_esame_file(struct Esame esame) {
    FILE *Lista_esami = fopen("esami.txt", "a");
    if (Lista_esami == NULL) {
        perror("Errore apertura file esami.txt");
        exit(EXIT_FAILURE);
    }

    // Scrittura dell'esame nel file
    if (fwrite(&esame, sizeof(struct Esame), 1, Lista_esami) != 1) {
        perror("Errore scrittura file esami.txt");
        fclose(Lista_esami);
        exit(EXIT_FAILURE);
    }

    fclose(Lista_esami);
    printf("Esame aggiunto con successo\n");
}

// Funzione per gestire la prenotazione di un esame
int gestisci_prenotazione(int universita_connessione_socket, struct Richiesta richiesta_ricevuta) {
    char matricola[20];
    int esito_prenotazione = 1; // 1 indica successo, 0 indica fallimento

    // Ricezione della matricola dallo studente
    ssize_t bytes_read = read(universita_connessione_socket, matricola, sizeof(matricola));
    if (bytes_read <= 0) {
        perror("Errore ricezione matricola");
        return -1;
    }

    matricola[bytes_read] = '\0'; // Assicurati che la stringa sia terminata correttamente
    printf("Prenotazione ricevuta per esame: %s, matricola: %s\n", richiesta_ricevuta.esame.nome, matricola);

    // Invia l'esito della prenotazione
    ssize_t bytes_written = write(universita_connessione_socket, &esito_prenotazione, sizeof(esito_prenotazione));
    if (bytes_written != sizeof(esito_prenotazione)) {
        perror("Errore invio esito prenotazione");
        return -1;
    }

    return 0;
}
