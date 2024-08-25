#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "wrapper.h"

// Studente:
// Chiede alla segreteria se ci siano esami disponibili per un corso
// Invia una richiesta di prenotazione di un esame alla segreteria





// Definizione della struttura per gli esami
struct Esame {
    char nome[100];
    char data[100];
};

struct Richiesta {
    int TipoRichiesta;
    struct Esame esame;
};

int ConnessioneSegreteria(int socket_studente, struct sockaddr_in *indirizzo_server_segreteria);
void riceviListaEsami(int socket_studente, int numero_esami, struct Esame *esameCercato);
int conto_esami(int socket_studente, int *numero_esami);




int main() {
    int socket_studente;
    struct sockaddr_in indirizzo_server_segreteria;
    //Creiamo la richiesta_studente e decidiamo il tipo di richiesta
    struct Richiesta richiesta_studente = {};

    printf("Connesso alla segreteria Esse4!\n");
    printf("1 - Vedi esami disponibili \n2 - Prenota un esame\n");
    printf("Scelta:");
    scanf("%d", &(*richiesta_studente).TipoRichiesta);
    getchar();


    //gestione esame disponibile
    if(richiesta_studente.TipoRichiesta == 1) 
    {

        int numero_esami= 0; //variabile per capire se ci stanno esami 
        struct Esame *esameCercato;
        

        printf("Nome esam eche vuoi cercare: ");
        fgets((*richiesta_studente).esame.nome, 100, stdin);
        (*richiesta_studente).esame.nome[strlen((*richiesta_studente).esame.nome) - 1] = '\000';
        fflush(stdin);

        if(richiesta_studente.esame.nome[0] != '\000')
        {   
            //connessione alla segreteria
            socket_studente = ConnessioneSegreteria(socket_studente, &indirizzo_server_segreteria);

            //invio della richiesta alla segreteria
            if (write(socket_studente, richiesta_studente, sizeof(*richiesta_studente)) != sizeof(*richiesta_studente)) {
                perror("Write error 3");
                exit(1);
            }

            //ricevo il numero di esami dalla segreteria
            numero_esami = conto_esami(socket_studente, &numero_esami);

            if(numero_esami > 0)
            {
                //dati per la lista di esami 
                riceviListaEsami(socket_studente, numero_esami, esameCercato);

                printf("Numero\tNome\tData\n");
                printf("-----------------");

                //stampa di esami, ipoteticamente da dare un codice per la prenotazione
                for(int i = 0; i < numero_esami; i++)
                {
                    printf("%d\t%s\t%s", i+1, esameCercato.nome, esameCercato.data);
                }

            } else {
                printf("non ci sono esami per il corso che hai cercato");
                return 0;
            }
        }
        
    } else if (richiesta_studente.TipoRichiesta == 2)
    {
        //richiesta di prenotazione
    }

    close(socket_studente);
}





int ConnessioneSegreteria(int socket_studente, struct sockaddr_in *indirizzo_server_segreteria)
{
    socket_studente = Socket(AF_INET, SOCK_STREAM, 0);

    (*indirizzo_server_segreteria).sin_family = AF_INET;
    (*indirizzo_server_segreteria).sin_port = htons(5000);

    if(inet_pton(AF_INET, "127.0.0.1", &(*indirizzo_server_segreteria).sin_addr) <= 0)
    {
        fprintf(stderr,"errore pton compa");
        exit(1);
    }

    Connetti(socket_studente, (struct sockaddr *)indirizzo_server_segreteria, sizeof(*));
    return socket_studente;
}

void riceviListaEsami(int socket_studente, int numero_esami, struct Esame *esameCercato)
{
    if(read(socket_studente, esameCercato, sizeof(struct Esame)*numero_esami)!=sizeof(struct Esame)*numero_esami)
    {
        perror("Errore ricezione lista esami");
        exit(-1);
    }
}


int conto_esami(int socket_studente, int *numero_esami)
{
    if (read(socket_studente, numero_esami, sizeof(*numero_esami))!= sizeof(*numero_esami))
    {
        perror("Errore conto_esami");
        exit(-1);
    }
    return (*numero_esami);
}