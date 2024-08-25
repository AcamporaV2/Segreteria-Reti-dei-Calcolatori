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



//Quello meno sviluppato, dovrebbe solo poter ricevere la richiesta di aggiunta per ora



// Funzione per aggiungere un esame al file 
void aggiungi_esame_file(struct Esame); 

int main() {
    int universita_connessione_socket;
    int universita_ascolto_socket;
    struct sockaddr_in indirizzo_universita;
    struct Richiesta richiesta_ricevuta;

    universita_ascolto_socket = Socket(AF_INET, SOCK_STREAM,0);

    indirizzo_universita.sin_family = AF_INET;
    indirizzo_universita.sin_addr.s_addr = hotnl(INADDR_ANY);
    indirizzo_universita.sin_port = htons(6940);

    Bind(universita_ascolto_socket, (struct sockaddr *) &indirizzo_universita, sizeof(indirizzo_universita));

    Ascolta(universita_ascolto_socket, 10);

    while(1) 
    {

        universita_connessione_socket = Accetta (universita_ascolto_socket, (struct sockaddr *)NULL, NULL);

        pid_t pid = fork();  // Creazione del processo figlio

         if (pid < 0) {
            perror("Errore nella fork controlla bene");
            exit(EXIT_FAILURE);
        }


        if(pid == 0) {

            close(universita_ascolto_socket);

            if(read(universita_connessione_socket, &richiesta_ricevuta, sizeof(struct Richiesta)) != sizeof(struct Richiesta))
            {
                perror("Qualche errore nel server amico");
                exit(-1);
            };

            if(richiesta_ricevuta.TipoRichiesta == 1)
            {
                aggiungi_esame_file(richiesta_ricevuta.esame);
            }
            
        }

    }


    return 0;
}

void aggiungi_esame_file(struct Esame esame) 
{
    FILE *Lista_esami;

    if((Lista_esami = fopen("esami.txt)", "a+")) == NULL)
    {
        printf("errore apertura file o sce");
        exit(1);
    }

    fwrite(&esame, sizeof(struct Esame), 1, Lista_esami);
    fclose(Lista_esami);
    printf("Esame aggiunto fratm");
}
