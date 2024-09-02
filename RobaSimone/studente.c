#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "wrapper.h"

// Definizione della struttura per gli esami
struct Esame {
    char nome[100];
    char data[100];
};

struct Richiesta {
    int TipoRichiesta;
    struct Esame esame;
};

// Dichiarazioni delle funzioni
int numeroPrenotazione(int socket_studente, int *numero_prenotazione);
void mandaMatricola(int socket_studente, char *matricola);
void mandaEsamePrenotazione(int socket_studente, struct Esame *esameDaPrenotare);
int ConnessioneSegreteria(int *socket_studente, struct sockaddr_in *indirizzo_server_segreteria);
void riceviListaEsami(int socket_studente, int numero_esami, struct Esame *esameCercato);
int conto_esami(int socket_studente, int *numero_esami);

int main() {
    int socket_studente;
    struct sockaddr_in indirizzo_server_segreteria;
    struct Richiesta richiesta_studente = {};
    int numero_esami = 0;
    struct Esame *esameCercato = NULL;
    
    printf("Connesso alla segreteria Esse4!\n");
    printf("1 - Vedi esami disponibili \n2 - Prenota un esame\n");
    printf("Scelta: ");
    scanf("%d", &richiesta_studente.TipoRichiesta);
    getchar(); // Pulisce il buffer di input

    if (richiesta_studente.TipoRichiesta == 1) {
        printf("Nome esame che vuoi cercare: ");
        fgets(richiesta_studente.esame.nome, sizeof(richiesta_studente.esame.nome), stdin);
        richiesta_studente.esame.nome[strcspn(richiesta_studente.esame.nome, "\n")] = '\0'; // Rimuove il newline

        if (strlen(richiesta_studente.esame.nome) > 0) {
            socket_studente = ConnessioneSegreteria(&socket_studente, &indirizzo_server_segreteria);

            if (write(socket_studente, &richiesta_studente, sizeof(richiesta_studente)) != sizeof(richiesta_studente)) {
                perror("Errore invio richiesta esami");
                close(socket_studente);
                exit(1);
            }

            numero_esami = conto_esami(socket_studente, &numero_esami);

            if (numero_esami > 0) {
                esameCercato = (struct Esame *)malloc(sizeof(struct Esame) * numero_esami);
                if (esameCercato == NULL) {
                    perror("Errore di allocazione memoria");
                    close(socket_studente);
                    exit(1);
                }
                
                riceviListaEsami(socket_studente, numero_esami, esameCercato);

                printf("Numero\tNome\tData\n");
                printf("-----------------\n");

                for (int i = 0; i < numero_esami; i++) {
                    printf("%d\t%s\t%s\n", i + 1, esameCercato[i].nome, esameCercato[i].data);
                }
                
                free(esameCercato);
            } else {
                printf("Non ci sono esami disponibili per il corso cercato.\n");
            }

            close(socket_studente);
        }
    } 
    else if (richiesta_studente.TipoRichiesta == 2) {
        socket_studente = ConnessioneSegreteria(&socket_studente, &indirizzo_server_segreteria);

        if (write(socket_studente, &richiesta_studente, sizeof(richiesta_studente)) != sizeof(richiesta_studente)) {
            perror("Errore invio richiesta prenotazione");
            close(socket_studente);
            exit(1);
        }

        numero_esami = conto_esami(socket_studente, &numero_esami);

        if (numero_esami > 0) {
            esameCercato = (struct Esame *)malloc(sizeof(struct Esame) * numero_esami);
            if (esameCercato == NULL) {
                perror("Errore di allocazione memoria");
                close(socket_studente);
                exit(1);
            }

            riceviListaEsami(socket_studente, numero_esami, esameCercato);

            printf("Numero\tNome\tData\n");
            printf("-----------------\n");

            for (int i = 0; i < numero_esami; i++) {
                printf("%d\t%s\t%s\n", i + 1, esameCercato[i].nome, esameCercato[i].data);
            }

            int scelta_esame;
            printf("Inserisci il numero dell'esame a cui vuoi prenotarti: ");
            scanf("%d", &scelta_esame);
            getchar(); // Pulisce il buffer di input

            if (scelta_esame > 0 && scelta_esame <= numero_esami) {
                char matricola[11];
                printf("Inserisci la tua matricola: ");
                fgets(matricola, sizeof(matricola), stdin);
                matricola[strcspn(matricola, "\n")] = '\0'; // Rimuove il newline

                struct Esame esameDaPrenotare = esameCercato[scelta_esame - 1];

                mandaEsamePrenotazione(socket_studente, &esameDaPrenotare);
                mandaMatricola(socket_studente, matricola);

                int numero_prenotazione;
                numeroPrenotazione(socket_studente, &numero_prenotazione);

                printf("Prenotazione completata con successo. Il tuo numero di prenotazione è: %d\n", numero_prenotazione);
            } else {
                printf("Scelta non valida.\n");
            }

            free(esameCercato);
        }

        close(socket_studente);
    }

    return 0;
}

int numeroPrenotazione(int socket_studente, int *numero_prenotazione) {
    if (read(socket_studente, numero_prenotazione, sizeof(*numero_prenotazione)) != sizeof(*numero_prenotazione)) {
        perror("Errore ricezione numero prenotazione");
        exit(1);
    }
    return *numero_prenotazione;
}

void mandaMatricola(int socket_studente, char *matricola) {
    if (write(socket_studente, matricola, strlen(matricola) + 1) != strlen(matricola) + 1) {
        perror("Errore invio matricola");
        exit(1);
    }
}

void mandaEsamePrenotazione(int socket_studente, struct Esame *esameDaPrenotare) {
    if (write(socket_studente, esameDaPrenotare, sizeof(struct Esame)) != sizeof(struct Esame)) {
        perror("Errore invio esame prenotazione");
        exit(1);
    }
}

int ConnessioneSegreteria(int *socket_studente, struct sockaddr_in *indirizzo_server_segreteria) {
    *socket_studente = Socket(AF_INET, SOCK_STREAM, 0);

    indirizzo_server_segreteria->sin_family = AF_INET;
    indirizzo_server_segreteria->sin_port = htons(2000);

    if (inet_pton(AF_INET, "127.0.0.1", &indirizzo_server_segreteria->sin_addr) <= 0) {
        perror("Errore inet_pton");
        exit(1);
    }

    Connetti(*socket_studente, (struct sockaddr *)indirizzo_server_segreteria, sizeof(*indirizzo_server_segreteria));
    return *socket_studente;
}

void riceviListaEsami(int socket_studente, int numero_esami, struct Esame *esameCercato) {
    if (read(socket_studente, esameCercato, sizeof(struct Esame) * numero_esami) != sizeof(struct Esame) * numero_esami) {
        perror("Errore ricezione lista esami");
        exit(1);
    }
}

int conto_esami(int socket_studente, int *numero_esami) {
    if (read(socket_studente, numero_esami, sizeof(*numero_esami)) != sizeof(*numero_esami)) {
        perror("Errore ricezione numero esami");
        exit(1);
    }
    return *numero_esami;
}
