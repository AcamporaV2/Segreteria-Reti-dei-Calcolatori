#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "wrapper.h"

// Server universitario:

//Riceve l'aggiunta di nuovi esami
//Riceve la prenotazione di un esame


// Definizione delle struct
struct Esame {
    char nome[100];
    char data[100];
};

struct Richiesta {
    int TipoRichiesta;
    struct Esame esame;
};

struct Prenotazione {
    struct Esame esame;
    int NumPrenotazione;
    char Matricola[11];
};

// Funzioni dichiarate
void aggiungi_esame_file(struct Esame esame);
int gestisci_prenotazione(int universita_connessione_socket, struct Richiesta richiesta_ricevuta);
void gestisci_esami_disponibili(int socket, struct Richiesta richiesta_ricevuta);

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
            perror("Errore nella fork controlla bene");
            exit(EXIT_FAILURE);
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

            } else if (richiesta_ricevuta.TipoRichiesta == 3) {

                gestisci_esami_disponibili(universita_connessione_socket, richiesta_ricevuta);

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

    // Scrittura dell'esame nel file usando fprintf
    if (fprintf(Lista_esami, "%s,%s\n", esame.nome, esame.data) < 0) {
        perror("Errore scrittura file esami.txt");
        fclose(Lista_esami);
        exit(EXIT_FAILURE);
    }

    fclose(Lista_esami);
    printf("Esame aggiunto con successo\n");
}

int gestisci_prenotazione(int universita_connessione_socket, struct Richiesta richiesta_ricevuta) {
    struct Prenotazione prenotazione;
    static int numPrenotazioneCounter = 1; // Contatore per assegnare un numero univoco alla prenotazione
    int esito_prenotazione = 1; // 1 indica successo, 0 indica fallimento

    // Ricezione della matricola dallo studente tramite la segreteria
    ssize_t MatricolaStudente = read(universita_connessione_socket, prenotazione.Matricola, sizeof(prenotazione.Matricola) - 1);
    if (MatricolaStudente <= 0) {
        if (MatricolaStudente == 0) {
            fprintf(stderr, "Connessione chiusa dalla segreteria durante la ricezione della matricola.\n");
        } else {
            perror("Errore ricezione matricola");
        }
        return -1;
    }

    // Assicurati che la stringa sia terminata correttamente
    prenotazione.Matricola[MatricolaStudente] = '\0';

    // Popolazione della struct Prenotazione
    prenotazione.esame = richiesta_ricevuta.esame;
    prenotazione.NumPrenotazione = numPrenotazioneCounter++;

    printf("Prenotazione ricevuta per esame: %s, matricola: %s\n", prenotazione.esame.nome, prenotazione.Matricola);

    // Scrittura della prenotazione nel file "prenotazioni.txt"
    FILE *file_prenotazioni = fopen("prenotazioni.txt", "a");
    if (file_prenotazioni == NULL) {
        perror("Errore apertura file prenotazioni.txt");
        esito_prenotazione = 0; // Imposta esito a 0 in caso di errore
    } else {
        // Scrivi la prenotazione nel file usando il formato "NumPrenotazione,EsameNome,EsameData,Matricola\n"
        fprintf(file_prenotazioni, "%d,%s,%s,%s\n", 
                prenotazione.NumPrenotazione, 
                prenotazione.esame.nome, 
                prenotazione.esame.data, 
                prenotazione.Matricola);
        fclose(file_prenotazioni);
    }

    // Invia l'esito della prenotazione
    ssize_t bytes_written = write(universita_connessione_socket, &esito_prenotazione, sizeof(esito_prenotazione));
    if (bytes_written != sizeof(esito_prenotazione)) {
        perror("Errore invio esito prenotazione");
        return -1;
    }

    return 0;
}


void gestisci_esami_disponibili(int socket, struct Richiesta richiesta_ricevuta) {
    FILE *Lista_esami = fopen("esami.txt", "r");
    struct Esame esami_disponibili[100]; // Supponiamo un massimo di 100 esami
    int numero_esami = 0;
    char nome[100], data[100];

    if (Lista_esami == NULL) {
        perror("Errore apertura file esami.txt");
        exit(EXIT_FAILURE);
    }

    // Leggiamo gli esami dal file e li filtriamo per nome
    while (fscanf(Lista_esami, "%99[^,],%99[^\n]\n", nome, data) == 2) {
        if (strcmp(nome, richiesta_ricevuta.esame.nome) == 0) {
            strcpy(esami_disponibili[numero_esami].nome, nome);
            strcpy(esami_disponibili[numero_esami].data, data);
            numero_esami++;
        }
    }
    fclose(Lista_esami);

    // Inviamo il numero di esami disponibili alla segreteria
    if (write(socket, &numero_esami, sizeof(numero_esami)) != sizeof(numero_esami)) {
        perror("Errore invio numero esami alla segreteria");
        exit(EXIT_FAILURE);
    }

    // Inviamo gli esami disponibili alla segreteria
    if (write(socket, esami_disponibili, sizeof(struct Esame) * numero_esami) != sizeof(struct Esame) * numero_esami) {
        perror("Errore invio esami disponibili alla segreteria");
        exit(EXIT_FAILURE);
    }
}


