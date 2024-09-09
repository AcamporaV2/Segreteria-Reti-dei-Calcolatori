#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "wrapper.h"

#define MAX_RETRY 3
#define MAX_WAIT_TIME 5  // in secondi
#define SIMULA_TIMEOUT 1 // Imposta a 1 per simulare timeout, 0 per disabilitare
// Studente:

// Chiede alla segreteria se ci siano esami disponibili per un corso
// Invia una richiesta di prenotazione di un esame alla segreteria

// Definizione della struct
struct Esame
{
    char nome[100];
    char data[100];
};

struct Richiesta
{
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

int main()
{
    int socket_studente;
    struct sockaddr_in indirizzo_server_segreteria;
    struct Richiesta richiesta_studente = {};
    int numero_esami = 0;
    struct Esame *esameCercato = NULL;

    printf("\nConnesso alla segreteria Esse4!\n");

    while (1)
    {
        printf("\n\n1 - Vedi esami disponibili \n2 - Prenota un esame\n");
        printf("Scelta: ");
        scanf("%d", &richiesta_studente.TipoRichiesta);
        getchar(); // Pulisce il buffer di input

        if (richiesta_studente.TipoRichiesta == 1)
        {

            printf("Nome esame che vuoi cercare: ");
            fgets(richiesta_studente.esame.nome, sizeof(richiesta_studente.esame.nome), stdin);
            richiesta_studente.esame.nome[strcspn(richiesta_studente.esame.nome, "\n")] = '\0'; // Rimuove il newline

            if (strlen(richiesta_studente.esame.nome) > 0)
            {
                socket_studente = ConnessioneSegreteria(&socket_studente, &indirizzo_server_segreteria);

                if (write(socket_studente, &richiesta_studente, sizeof(richiesta_studente)) != sizeof(richiesta_studente))
                {
                    perror("Errore invio richiesta esami");
                    close(socket_studente);
                    exit(1);
                }

                numero_esami = conto_esami(socket_studente, &numero_esami);

                if (numero_esami > 0)
                {
                    esameCercato = (struct Esame *)malloc(sizeof(struct Esame) * numero_esami);
                    if (esameCercato == NULL)
                    {
                        perror("Errore di allocazione memoria");
                        close(socket_studente);
                        exit(1);
                    }

                    riceviListaEsami(socket_studente, numero_esami, esameCercato);

                    printf("Numero\tNome\tData\n");
                    printf("-----------------\n");

                    for (int i = 0; i < numero_esami; i++)
                    {
                        printf("%d\t%s\t%s\n", i + 1, esameCercato[i].nome, esameCercato[i].data);
                    }

                    free(esameCercato);
                }
                else
                {
                    printf("Non ci sono esami disponibili per il corso cercato.\n");
                }

                close(socket_studente);
            }
        }
        else if (richiesta_studente.TipoRichiesta == 2)
        {
            printf("Nome esame che vuoi prenotare: ");
            fgets(richiesta_studente.esame.nome, sizeof(richiesta_studente.esame.nome), stdin);
            richiesta_studente.esame.nome[strcspn(richiesta_studente.esame.nome, "\n")] = '\0'; // Rimuove il newline

            socket_studente = ConnessioneSegreteria(&socket_studente, &indirizzo_server_segreteria);

            if (write(socket_studente, &richiesta_studente, sizeof(richiesta_studente)) != sizeof(richiesta_studente))
            {
                perror("Errore invio richiesta prenotazione");
                close(socket_studente);
                exit(1);
            }

            printf("Inizio conto esami\n");

            numero_esami = conto_esami(socket_studente, &numero_esami);

            printf("Ho contato gli esami\n");

            if (numero_esami > 0)
            {
                printf("Numero esami: %d\n", numero_esami);
                esameCercato = (struct Esame *)malloc(sizeof(struct Esame) * numero_esami);
                if (esameCercato == NULL)
                {
                    perror("Errore di allocazione memoria");
                    close(socket_studente);
                    exit(1);
                }

                riceviListaEsami(socket_studente, numero_esami, esameCercato);

                printf("Numero\tNome\tData\n");
                printf("-----------------\n");

                for (int i = 0; i < numero_esami; i++)
                {
                    printf("%d\t%s\t%s\n", i + 1, esameCercato[i].nome, esameCercato[i].data);
                }

                int scelta_esame;
                printf("Inserisci il numero dell'esame a cui vuoi prenotarti: ");
                scanf("%d", &scelta_esame);
                getchar(); // Pulisce il buffer di input

                if (scelta_esame > 0 && scelta_esame <= numero_esami)
                {
                    char matricola[11];
                    printf("Inserisci la tua matricola: ");
                    fgets(matricola, sizeof(matricola), stdin);
                    matricola[strcspn(matricola, "\n")] = '\0'; // Rimuove il newline

                    struct Esame esameDaPrenotare = esameCercato[scelta_esame - 1];

                    // Assicurati di inviare prima la struttura Esame
                    mandaEsamePrenotazione(socket_studente, &esameDaPrenotare);

                    // Poi invia la matricola
                    mandaMatricola(socket_studente, matricola);

                    int prenotazione;
                    numeroPrenotazione(socket_studente, &prenotazione);

                    printf("Prenotazione completata con successo. Il tuo numero di prenotazione è: %d\n", prenotazione);
                }
                else
                {
                    printf("Scelta non valida.\n");
                }

                free(esameCercato);
            }

            close(socket_studente);
        }
    }

    return 0;
}

int numeroPrenotazione(int socket_studente, int *numero_prenotazione)
{
    if (read(socket_studente, numero_prenotazione, sizeof(*numero_prenotazione)) != sizeof(*numero_prenotazione))
    {
        perror("Errore ricezione numero prenotazione");
        exit(1);
    }
    return *numero_prenotazione;
}

void mandaMatricola(int socket_studente, char *matricola)
{
    // Usa strlen(matricola) + 1 per includere il terminatore nullo
    size_t length = strlen(matricola); // +1 per includere il terminatore nullo
    if (write(socket_studente, matricola, length) != length)
    {
        perror("Errore invio matricola");
        exit(1);
    }
}

void mandaEsamePrenotazione(int socket_studente, struct Esame *esameDaPrenotare)
{
    // Invia l'intera struttura Esame
    if (write(socket_studente, esameDaPrenotare, sizeof(struct Esame)) != sizeof(struct Esame))
    {
        perror("Errore invio esame prenotazione");
        exit(1);
    }
    printf("Debug: Esame inviato dallo studente: Nome = %s, Data = %s\n", esameDaPrenotare->nome, esameDaPrenotare->data);
}

int ConnessioneSegreteria(int *socket_studente, struct sockaddr_in *indirizzo_server_segreteria)
{
    int retry_count = 0;
    int success = 0;

    // Configurazione del socket
    *socket_studente = Socket(AF_INET, SOCK_STREAM, 0);

    indirizzo_server_segreteria->sin_family = AF_INET;
    indirizzo_server_segreteria->sin_port = htons(2000);

    if (inet_pton(AF_INET, "127.0.0.1", &indirizzo_server_segreteria->sin_addr) <= 0)
    {
        perror("Errore inet_pton");
        exit(EXIT_FAILURE);
    }

    while (retry_count < MAX_RETRY && !success)
    {
        // Tentativo di connessione
        Connetti(*socket_studente, (struct sockaddr *)indirizzo_server_segreteria, sizeof(*indirizzo_server_segreteria));
        
        // Verifica se la connessione è riuscita
        if (errno == 0)
        {
            success = 1; // Connessione riuscita
        }
        else
        {
            perror("Errore di connessione");
            retry_count++;

            if (retry_count < MAX_RETRY)
            {
                printf("Tentativo di riconnessione in corso...\n");
                sleep(2); // Pausa tra i tentativi
            }
            else
            {
                if (SIMULA_TIMEOUT)
                {
                    int aspetta_tempo = rand() % MAX_WAIT_TIME + 1;
                    printf("Simulazione timeout: attesa di %d secondi\n", aspetta_tempo);
                    sleep(aspetta_tempo);

                    // Dopo la pausa, riprova a connetterti
                    retry_count = 0; // Resetta il contatore dei tentativi
                }
                else
                {
                    fprintf(stderr, "Numero massimo di tentativi raggiunto. Uscita.\n");
                    close(*socket_studente);
                    exit(EXIT_FAILURE);
                }
            }
        }
    }

    if (!success)
    {
        fprintf(stderr, "Impossibile connettersi alla segreteria dopo %d tentativi.\n", MAX_RETRY);
        close(*socket_studente);
        exit(EXIT_FAILURE);
    }

    return *socket_studente; // Restituisce il socket in caso di successo
}

void riceviListaEsami(int socket_studente, int numero_esami, struct Esame *esameCercato)
{
    if (read(socket_studente, esameCercato, sizeof(struct Esame) * numero_esami) != sizeof(struct Esame) * numero_esami)
    {
        perror("Errore ricezione lista esami");
        exit(1);
    }
}

int conto_esami(int socket_studente, int *numero_esami)
{
    if (read(socket_studente, numero_esami, sizeof(*numero_esami)) != sizeof(*numero_esami))
    {
        perror("Errore ricezione numero esami");
        exit(1);
    }
    return *numero_esami;
}
