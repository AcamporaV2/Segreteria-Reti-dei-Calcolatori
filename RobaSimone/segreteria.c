#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "wrapper.h"

//  Segreteria:

//Inserisce gli esami sul server dell'università (salvare in un file o conservare in memoria il dato)
//Inoltra la richiesta di prenotazione degli studenti al server universitario
//Fornisce allo studente le date degli esami disponibili per l'esame scelto dallo studente


struct Esame {
    char nome[100];
    char data[100];
};

struct Richiesta {
    int TipoRichiesta;
    struct Esame esame;
};


// Il file probabilmente con più roba, non sono sicuro funzioni all 100%
// ma dovrebbe avere la connessione degli studenti e al server uni



int ascolto_studenti(int segreteria_ascolto_socket, struct sockaddr_in *indirizzo_segreteria);
void esami_disponibili(struct Esame esame, int segreteria_connessione_socket);
void invio_esame_server(int socket_esami, struct Richiesta ricezione_esami);
int connessione_universita(int socket_esami, struct sockaddr_in *indirizzo_universita);
void mandaEsameNuovoServer(struct Esame esame);
void inserisci_nuovo_esame(void);



int main() {
    int segreteria_connessione_socket;
    int segreteria_ascolto_socket;
    struct sockaddr_in indirizzo_segreteria;
    struct Richiesta richiesta_ricevuta;
    int scelta;  // Variabile per la scelta dell'azione

    segreteria_ascolto_socket = ascolto_studenti(segreteria_ascolto_socket, &indirizzo_segreteria);

    while (1) {
        // Chiedi all'utente cosa vuole fare
        printf("Scegli un'azione:\n");
        printf("1. Aggiungi un nuovo esame\n");
        printf("2. Ascolta le richieste degli studenti\n");
        printf("Inserisci la tua scelta: ");
        scanf("%d", &scelta);
        getchar();  // Consuma il newline lasciato da scanf


        //inizio parte inserimento esame
        if (scelta == 1) {

            inserisci_nuovo_esame();

            printf("Vuoi aggiungere un altro esame o ascoltare le richieste degli studenti?\n");
            printf("1. Aggiungi un altro esame\n");
            printf("2. Ascolta le richieste degli studenti\n");
            printf("Inserisci la tua scelta: ");
            scanf("%d", &scelta);
            getchar();  // Consuma il newline lasciato da scanf
            if (scelta == 2) {
                continue;  // Esce dall'aggiunta esame e passa all'ascolto degli studenti
            }
        } 
        //Fine parte inserimento esami



        if (scelta == 2) {
            // Gestione delle richieste degli studenti
            segreteria_connessione_socket = Accetta(segreteria_ascolto_socket, (struct sockaddr*) NULL, NULL);

            pid_t pid = fork();  // Creazione del processo figlio

            if (pid < 0) {
                perror("Errore nella fork controlla bene");
                exit(EXIT_FAILURE);
            }

            if (pid == 0) {  // Processo figlio
                close(segreteria_ascolto_socket);  // Il figlio non ha bisogno di questo socket
                
                printf("studente connesso");

                if (read(segreteria_connessione_socket, &richiesta_ricevuta, sizeof(struct Richiesta)) != sizeof(struct Richiesta)) {
                    perror("read error");
                    exit(-1);
                }

                if (richiesta_ricevuta.TipoRichiesta == 1) {

                    esami_disponibili(richiesta_ricevuta.esame, segreteria_connessione_socket);
                    
                }
                else if(richiesta_ricevuta.TipoRichiesta == 2)
                {
                    int socket_prenotazione_esame;
                    struct sockaddr_in indirizzo_universita;
                    socket_prenotazione_esame = connessione_universita(socket_prenotazione_esame, &indirizzo_universita);

                    // Prenotazione esame
                    MandaPrenotazioneEsame(segreteria_connessione_socket, socket_prenotazione_esame, &richiesta_ricevuta.esame);

                    RiceviMatricola(segreteria_connessione_socket, socket_prenotazione_esame);

                    EsitoPrenotazione(segreteria_connessione_socket, socket_prenotazione_esame);

                }

                close(segreteria_connessione_socket);  // Chiude la connessione dopo aver gestito la richiesta
                exit(0);  // Termina il processo figlio
            } else {  // Processo padre
                close(segreteria_connessione_socket);  // Il padre non ha bisogno di questo socket
            }
        } else {
            printf("Scelta non valida. Riprova.\n");
        }
    }

    close(segreteria_ascolto_socket);

    return 0;
}


void MandaPrenotazioneEsame(int segreteria_connessione_socket, int socket_prenotazione_esame, struct Esame *esame) {
    // Invia il nome dell'esame al server universitario
    if (write(socket_prenotazione_esame, esame, sizeof(struct Esame)) != sizeof(struct Esame)) {
        perror("Errore invio esame al server universitario");
        exit(1);
    }
}

void RiceviMatricola(int segreteria_connessione_socket, int socket_prenotazione_esame) {
    char matricola[11]= "0124002485";
    
    // Riceve la matricola dallo studente
    if (read(segreteria_connessione_socket, matricola, sizeof(matricola)) <= 0) {
        perror("Errore ricezione matricola dallo studente");
        exit(1);
    }
    
    // Invia la matricola al server universitario
    if (write(socket_prenotazione_esame, matricola, sizeof(matricola)) != sizeof(matricola)) {
        perror("Errore invio matricola al server universitario");
        exit(1);
    }
}

void EsitoPrenotazione(int segreteria_connessione_socket, int socket_prenotazione_esame) {
    int esito_prenotazione;
    
    // Riceve l'esito della prenotazione dal server universitario
    if (read(socket_prenotazione_esame, &esito_prenotazione, sizeof(esito_prenotazione)) <= 0) {
        perror("Errore ricezione esito prenotazione dal server universitario");
        exit(1);
    }
    
    // Invia l'esito della prenotazione allo studente
    if (write(segreteria_connessione_socket, &esito_prenotazione, sizeof(esito_prenotazione)) != sizeof(esito_prenotazione)) {
        perror("Errore invio esito prenotazione allo studente");
        exit(1);
    }
}




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

    ricezione_esami.TipoRichiesta = 2;
    ricezione_esami.esame = esame;
    socket_esami = connessione_universita(socket_esami, &indirizzo_universita);
    invio_esame_server(socket_esami, ricezione_esami);
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
void invio_esame_server(int socket_esami, struct Richiesta ricezione_esami) {

    if(write(socket_esami, &ricezione_esami, sizeof(ricezione_esami)) != sizeof(ricezione_esami)) {
        perror("Errore invio esame");
        exit(1);
    }

}


void inserisci_nuovo_esame() {
    struct Esame nuovo_esame;

    printf("Inserisci il nome dell'esame: ");
    fgets(nuovo_esame.nome, sizeof(nuovo_esame.nome), stdin);
    nuovo_esame.nome[strcspn(nuovo_esame.nome, "\n")] = '\0';  // Rimuovi newline

    printf("Inserisci la data dell'esame (YYYY-MM-DD): ");
    fgets(nuovo_esame.data, sizeof(nuovo_esame.data), stdin);
    nuovo_esame.data[strcspn(nuovo_esame.data, "\n")] = '\0';  // Rimuovi newline

    mandaEsameNuovoServer(nuovo_esame);
    
}

void mandaEsameNuovoServer(struct Esame esame) {
    int socket_segreteria;
    struct sockaddr_in indirizzo_universita;
    struct Richiesta aggiuntaEsame;

    aggiuntaEsame.TipoRichiesta = 1;
    aggiuntaEsame.esame = esame;

    socket_segreteria = connessione_universita(socket_segreteria, &indirizzo_universita);

    if(write(socket_segreteria, &aggiuntaEsame, sizeof(aggiuntaEsame)) != sizeof(aggiuntaEsame))
    {
        perror("Errore manda esame server");
        exit(1);
    }

    printf("Esame aggiunto con successo!\n");

    close(socket_segreteria);
}
