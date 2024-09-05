#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "wrapper.h"

// Definizione delle struct
struct Esame {
    char nome[100];
    char data[100];
};

struct Richiesta {
    int TipoRichiesta;
    struct Esame esame;
};

// Funzioni dichiarate
int ascolto_studenti(struct sockaddr_in *indirizzo_segreteria);
void esami_disponibili(struct Esame esame, int segreteria_connessione_socket);
void invio_esame_server(int socket_esami, struct Richiesta ricezione_esami);
int connessione_universita(struct sockaddr_in *indirizzo_universita);
void mandaEsameNuovoServer(struct Esame esame);
void inserisci_nuovo_esame(void);
void MandaPrenotazioneEsame(int socket_prenotazione_esame, struct Esame *esame);
void RiceviMatricola(int segreteria_connessione_socket, int socket_prenotazione_esame);
void EsitoPrenotazione(int segreteria_connessione_socket, int socket_prenotazione_esame);
void MandaNumeroPrenotazione(int segreteria_connessione_socket, int socket_prenotazione_esame);

int main() {
    int segreteria_ascolto_socket;
    struct sockaddr_in indirizzo_segreteria;
    int scelta;

    segreteria_ascolto_socket = ascolto_studenti(&indirizzo_segreteria);

    while (1) {
        printf("\nScegli un'azione:\n");
        printf("1. Aggiungi un nuovo esame\n");
        printf("2. Ascolta le richieste degli studenti\n");
        printf("Inserisci la tua scelta: ");
        scanf("%d", &scelta);
        getchar();  // Consuma il newline lasciato da scanf

        if (scelta == 1) {
            inserisci_nuovo_esame();
            continue;
        } 

        if (scelta == 2) {
            printf("Ascolto studenti sulla porta 2000...\n");

            // Processo figlio per gestire le richieste in modo continuo
            pid_t pid = fork();

            if (pid < 0) {
                perror("Errore nella fork");
                exit(EXIT_FAILURE);
            }

            if (pid == 0) { // Processo figlio
                while (1) {
                    int segreteria_connessione_socket;
                    struct Richiesta richiesta_ricevuta;

                    // Gestione continua delle richieste degli studenti
                    segreteria_connessione_socket = Accetta(segreteria_ascolto_socket, (struct sockaddr *)NULL, NULL);

                    if (read(segreteria_connessione_socket, &richiesta_ricevuta, sizeof(struct Richiesta)) != sizeof(struct Richiesta)) {
                        perror("Errore nella lettura della richiesta");
                        close(segreteria_connessione_socket);
                        continue; // Continua a gestire altre richieste
                    }

                    if (richiesta_ricevuta.TipoRichiesta == 1) {
                        // Richiesta di visualizzare esami disponibili
                        esami_disponibili(richiesta_ricevuta.esame, segreteria_connessione_socket);

                    } else if (richiesta_ricevuta.TipoRichiesta == 2) {
                        // Gestione della prenotazione esame
                        int socket_prenotazione_esame;
                        struct sockaddr_in indirizzo_universita;
                        socket_prenotazione_esame = connessione_universita(&indirizzo_universita);

                        MandaPrenotazioneEsame(socket_prenotazione_esame, &richiesta_ricevuta.esame);
                        RiceviMatricola(segreteria_connessione_socket, socket_prenotazione_esame);
                        EsitoPrenotazione(segreteria_connessione_socket, socket_prenotazione_esame);
                        MandaNumeroPrenotazione(segreteria_connessione_socket, socket_prenotazione_esame);

                        close(socket_prenotazione_esame);
                    } else {
                        fprintf(stderr, "Tipo di richiesta non valido\n");
                    }

                    close(segreteria_connessione_socket); // Chiudi il socket dopo aver gestito la richiesta
                }

                // Uscita dal processo figlio in caso di errore critico (opzionale)
                close(segreteria_ascolto_socket);
                exit(EXIT_SUCCESS);

            } else { // Processo padre
                // Mantieni il processo padre in attesa per prevenire il ritorno al terminale
                // Puoi usare sleep in un ciclo o semplicemente usare wait per gestire i processi figli
                while (1) {
                    wait(NULL);  // Attende la terminazione di qualsiasi processo figlio
                    // Alternativamente, puoi usare sleep(1); per far dormire il processo padre indefinitamente
                }
            }

        } else {
            printf("Scelta non valida. Riprova.\n");
        }
    }

    close(segreteria_ascolto_socket);
    return 0;
}

void MandaNumeroPrenotazione(int segreteria_connessione_socket, int socket_prenotazione_esame) {
    int numero_prenotazione;

    // Ricezione del numero di prenotazione dal server universitario
    if (read(socket_prenotazione_esame, &numero_prenotazione, sizeof(numero_prenotazione)) != sizeof(numero_prenotazione)) {
        perror("Errore ricezione numero prenotazione dal server universitario");
        exit(EXIT_FAILURE);
    }

    // Invio del numero di prenotazione al client studente tramite la segreteria
    if (write(segreteria_connessione_socket, &numero_prenotazione, sizeof(numero_prenotazione)) != sizeof(numero_prenotazione)) {
        perror("Errore invio numero prenotazione allo studente");
        exit(EXIT_FAILURE);
    }

    printf("Numero prenotazione inviato allo studente: %d\n", numero_prenotazione);
}

void MandaPrenotazioneEsame(int socket_prenotazione_esame, struct Esame *esame) {
    if (write(socket_prenotazione_esame, esame, sizeof(struct Esame)) != sizeof(struct Esame)) {
        perror("Errore invio esame al server universitario");
        exit(EXIT_FAILURE);
    }
}

void RiceviMatricola(int segreteria_connessione_socket, int socket_prenotazione_esame) {
    char matricola[11]; // Array per la matricola, massimo 10 caratteri + terminatore nullo

    // Ricezione della matricola dallo studente
    ssize_t MatricolaStudente = read(segreteria_connessione_socket, matricola, sizeof(matricola) - 1); // Leggi fino a 10 caratteri
    if (MatricolaStudente <= 0) {
        if (MatricolaStudente == 0) {
            fprintf(stderr, "Connessione chiusa dallo studente durante la ricezione della matricola.\n");
        } else {
            perror("Errore ricezione matricola dallo studente");
        }
        exit(EXIT_FAILURE);
    }

    // Assicurati che la stringa sia terminata correttamente
    matricola[MatricolaStudente] = '\0'; // Aggiungi il terminatore nullo alla fine

    // Invia la matricola al server universitario
    if (write(socket_prenotazione_esame, matricola, sizeof(matricola)) != sizeof(matricola)) {
        perror("Errore invio matricola al server universitario");
        exit(EXIT_FAILURE);
    }
}

void EsitoPrenotazione(int segreteria_connessione_socket, int socket_prenotazione_esame) {
    int esito_prenotazione;
    
    if (read(socket_prenotazione_esame, &esito_prenotazione, sizeof(esito_prenotazione)) <= 0) {
        perror("Errore ricezione esito prenotazione dal server universitario");
        exit(EXIT_FAILURE);
    }
    
    if (write(segreteria_connessione_socket, &esito_prenotazione, sizeof(esito_prenotazione)) != sizeof(esito_prenotazione)) {
        perror("Errore invio esito prenotazione allo studente");
        exit(EXIT_FAILURE);
    }
}

int ascolto_studenti(struct sockaddr_in *indirizzo_segreteria) {
    int segreteria_ascolto_socket = Socket(AF_INET, SOCK_STREAM, 0);

    indirizzo_segreteria->sin_family = AF_INET;
    indirizzo_segreteria->sin_addr.s_addr = htonl(INADDR_ANY);
    indirizzo_segreteria->sin_port = htons(2000);

    Bind(segreteria_ascolto_socket, (struct sockaddr *)indirizzo_segreteria, sizeof(*indirizzo_segreteria));
    Ascolta(segreteria_ascolto_socket, 10);

    return segreteria_ascolto_socket;
}

void esami_disponibili(struct Esame esame, int segreteria_connessione_socket) {
    int socket_esami;
    struct sockaddr_in indirizzo_universita;
    struct Richiesta ricezione_esami;
    struct Esame esami_disponibili[100]; // Assumiamo che ci siano al massimo 100 esami
    int numero_esami = 0;

    ricezione_esami.TipoRichiesta = 3; // TipoRichiesta 3 per ottenere gli esami disponibili
    ricezione_esami.esame = esame;

    socket_esami = connessione_universita(&indirizzo_universita);
    invio_esame_server(socket_esami, ricezione_esami);

    // Riceviamo il numero di esami dal server universitario
    if (read(socket_esami, &numero_esami, sizeof(numero_esami)) != sizeof(numero_esami)) {
        perror("Errore ricezione numero esami dal server universitario");
        close(socket_esami);
        exit(EXIT_FAILURE);
    }

    // Inviamo il numero di esami allo studente
    if (write(segreteria_connessione_socket, &numero_esami, sizeof(numero_esami)) != sizeof(numero_esami)) {
        perror("Errore invio numero esami allo studente");
        close(socket_esami);
        exit(EXIT_FAILURE);
    }

    // Riceviamo gli esami disponibili dal server universitario
    if (read(socket_esami, esami_disponibili, sizeof(struct Esame) * numero_esami) != sizeof(struct Esame) * numero_esami) {
        perror("Errore ricezione lista esami dal server universitario");
        close(socket_esami);
        exit(EXIT_FAILURE);
    }

    // Inviamo la lista di esami disponibili allo studente
    if (write(segreteria_connessione_socket, esami_disponibili, sizeof(struct Esame) * numero_esami) != sizeof(struct Esame) * numero_esami) {
        perror("Errore invio lista esami allo studente");
        close(socket_esami);
        exit(EXIT_FAILURE);
    }
}

int connessione_universita(struct sockaddr_in *indirizzo_universita) {
    int socket_esami = Socket(AF_INET, SOCK_STREAM, 0);

    indirizzo_universita->sin_family = AF_INET;
    indirizzo_universita->sin_port = htons(6940);

    if (inet_pton(AF_INET, "127.0.0.1", &indirizzo_universita->sin_addr) <= 0) {
        perror("Errore inet 127.0.0.1");
        exit(EXIT_FAILURE);
    }

    Connetti(socket_esami, (struct sockaddr *) indirizzo_universita, sizeof(*indirizzo_universita));

    return socket_esami;
}

void invio_esame_server(int socket_esami, struct Richiesta ricezione_esami) {
    if (write(socket_esami, &ricezione_esami, sizeof(ricezione_esami)) != sizeof(ricezione_esami)) {
        perror("Errore invio esame");
        exit(EXIT_FAILURE);
    }
}

void inserisci_nuovo_esame() {
    struct Esame nuovo_esame;

    printf("Inserisci il nome dell'esame: ");
    fgets(nuovo_esame.nome, sizeof(nuovo_esame.nome), stdin);
    nuovo_esame.nome[strcspn(nuovo_esame.nome, "\n")] = '\0';

    printf("Inserisci la data dell'esame (YYYY-MM-DD): ");
    fgets(nuovo_esame.data, sizeof(nuovo_esame.data), stdin);
    nuovo_esame.data[strcspn(nuovo_esame.data, "\n")] = '\0';

    mandaEsameNuovoServer(nuovo_esame);
}

void mandaEsameNuovoServer(struct Esame esame) {
    int socket_segreteria;
    struct sockaddr_in indirizzo_universita;
    struct Richiesta aggiuntaEsame;

    aggiuntaEsame.TipoRichiesta = 1;
    aggiuntaEsame.esame = esame;

    socket_segreteria = connessione_universita(&indirizzo_universita);

    if (write(socket_segreteria, &aggiuntaEsame, sizeof(aggiuntaEsame)) != sizeof(aggiuntaEsame)) {
        perror("Errore manda esame server");
        exit(EXIT_FAILURE);
    }

    printf("Esame aggiunto con successo!\n");

    close(socket_segreteria);
}
