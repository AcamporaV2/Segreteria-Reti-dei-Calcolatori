#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "wrapper.h"

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
void salva_esame_su_file(struct Esame esame);
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

                if (read(segreteria_connessione_socket, &richiesta_ricevuta, sizeof(struct Richiesta)) != sizeof(struct Richiesta)) {
                    perror("read error");
                    exit(-1);
                }

                if (richiesta_ricevuta.TipoRichiesta == 1) {
                    esami_disponibili(richiesta_ricevuta.esame, segreteria_connessione_socket);
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
void salva_esame_su_file(struct Esame esame) {
    FILE *file = fopen("esami.txt", "a");  // Apertura del file in modalità append

    if (file == NULL) {
        perror("Errore apertura file esami.txt");
        exit(1);
    }

    fprintf(file, "%s %s\n", esame.nome, esame.data);  // Scrittura dell'esame nel file
    fclose(file);  // Chiusura del file
}

void inserisci_nuovo_esame() {
    struct Esame nuovo_esame;

    printf("Inserisci il nome dell'esame: ");
    fgets(nuovo_esame.nome, sizeof(nuovo_esame.nome), stdin);
    nuovo_esame.nome[strcspn(nuovo_esame.nome, "\n")] = '\0';  // Rimuovi newline

    printf("Inserisci la data dell'esame (YYYY-MM-DD): ");
    fgets(nuovo_esame.data, sizeof(nuovo_esame.data), stdin);
    nuovo_esame.data[strcspn(nuovo_esame.data, "\n")] = '\0';  // Rimuovi newline

    salva_esame_su_file(nuovo_esame);
    printf("Esame aggiunto con successo!\n");
}
