
#ifndef FILE_H
#define FILE_H


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>		
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#define NOM_SHM "/mon_shm"
#define NOM_TUBE "Tube"
#define quitter "quit"
#define NB_THREAD 3
#define TAILLE_SHM (sizeof(  fifo) + NB_THREAD)

#define BUF_SIZE 1024
#define prompt "client :> "


typedef struct  {
	pid_t pid; //pid du processus qui a envoyé la commande
    char nom[256]; //infos sur le tubes nom+size
    char cmd[256]; //infos sur la commande donné par le client !   
} requete_t;



   typedef struct   {
	sem_t plein;
	sem_t vide;
	sem_t mutex;
	int tete; // Position d'ajout dans le tampon
	int queue; // Position de suppression dans le tampon
	requete_t buffer[]; // Le tampon contenant les données 
	}fifo; 

void sighand(int signo, siginfo_t *info, void *extra);
void clientPrompt();
int recherche(bool *libre);
void arrange(char *line);
void parse(char *cmd, char **argv);
requete_t defiler(fifo *fifo_p);
void enfiler(fifo *fifo_p,requete_t req);
void V(sem_t *semaphore);
void P(sem_t *semaphore);
int ouverture_tube_client(char *tube);
void creatThreads(void * (* run_type ) ( void *));
void destroy (sem_t * semaphore);
void read_write (char *tube);
void create_tube_client (char *tube);
void *run (void *args);

#endif
