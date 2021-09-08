
#include "File.h"

//le role de cette fonction est de donner une forme juste de la commande passer par le client
void parse(char *cmd, char **argv){
	
	int narg = 0;
	//dans cette boucle on élimineles espaces,tabulation...
	while (*cmd)
	 {
		//si on rencontre une espace on termine la chaine et on le stocke  
		if (*cmd == ' ' || *cmd == '\t' || *cmd == '&')
			*cmd++ = '\0';


	//sinon on ajoute le charactére à la chaine de caractères 
	else {
		argv[narg++] = cmd++;
		while (*cmd != '\0' && *cmd != ' ' && *cmd != '\t') cmd++;
		
		}
	}
	//on finit le tableau des chaines de caratère par NULL pour le passer comme un argument à execvp
	argv[narg]=NULL;
}

/*cette fonction exécute la commande retourner par 
 * la fonction parse dans les paramétre il prends 
 * la commande passer par le client*/
 
void arrange(char *line){
	
		/*un pointeur sur des chaines de caractére
		 *  pour stocker le résultat de la commande parse*/
		char *argv[256]; 
		
		
		parse(line,argv);	
		
		/*on passe à l'appel système execvp le retour de la fonction parse on cas d'rreur
		 *  on affiche que la commande est introuvable et puisque la sortie standard est redérigée vers le tube.
		 *  le message s'affiche implicitement au client*/ 
		 
		if(execvp(argv[0],argv)==-1){
			printf("\n %s : commande introuvable\n",argv[0]);
			exit(1);
			}
		
			
}


/*
 *  cette fonction  retourne l'id de la case libre dans le tableau de bool passer dans les arguments de cette fonction */

int recherche(bool *libre){
	
	int thread=-1;
	
	for(int i=0;i<NB_THREAD;++i){
		if(!libre[i]){
			 thread=i;
			 break;
			}
		}

	return thread;
	
}
//création de tube
void create_tube_client (char *tube)
{

  //création du tube nommé que le demon utilisera
  if (mkfifo (tube, S_IRUSR | S_IWUSR) == -1)
    {

      perror ("Erreur création du tube client");
      exit (EXIT_FAILURE);
    }


}
/*ajouter la commande à la file passer dans l'argument en utilisant l'algorithme producteur consomateur*/
void enfiler (fifo * fifo_p, requete_t req)
{
	//on attends si la file est plein
  P (&fifo_p->vide);
  
	//on attends que le mutex est dévrouillé
  P (&fifo_p->mutex);
	//après qu'on entre à la section critique on vérouille le mutex
	
	//on ajoute la requete passer dans les arguments
  fifo_p->buffer[fifo_p->tete] = req;
  fifo_p->tete = (fifo_p->tete + 1) % NB_THREAD;
  
	//on dévrouille le mutex en incrémentant le sémaphore mutex
  V (&fifo_p->mutex);
  
	//on incrémente la valeur de sémaphore plein 
  V (&fifo_p->plein);


}
requete_t defiler (fifo * fifo_p)
{
	//on attends si la file est vide
  P (&fifo_p->plein);
  
	//on attends que le mutex est dévrouillé
  P (&fifo_p->mutex);
   //après qu'on entre à la section critique on vérouille le mutex
  
	//on retire la requete de la file
  requete_t req = fifo_p->buffer[fifo_p->queue];
  fifo_p->queue = (fifo_p->queue + 1) % NB_THREAD;
  //on dévrouille le mutex en incrémentant le sémaphore mutex
  V (&fifo_p->mutex);
  //on incrémente la valeur de sémaphore vide 
  V (&fifo_p->vide);

  return req;
}



//fonction qui execute l'appel système sem_wait sur le semaphore passer dans les aurgument
void P(sem_t *semaphore){
	if (sem_wait(semaphore) == -1) {
			perror("sem_wait");
			exit(EXIT_FAILURE);
		}
	}
	
//fonction qui execute l'appel système sem_post sur le semaphore passer dans les aurgument
void V(sem_t *semaphore){
	
	if (sem_post(semaphore) == -1) {
			perror("sem_post");
			exit(EXIT_FAILURE);
		}
}
//fonction qui execute l'appel système sem_destroy sur le semaphore passer dans les aurgument
void destroy (sem_t * semaphore)
{
  if (sem_destroy (semaphore) == -1)
    {
      perror ("sem_destroy");
      exit (EXIT_FAILURE);
    }
}


//recuperer le signal recu par le daemon et afficher un message à le client

void sighand (int signo, siginfo_t * info, void *extra)
{
  if (signo < 0)
    {
      fprintf (stderr, "Wrong signal number\n");
      exit (EXIT_FAILURE);
    }
  else
    {
      printf ("le serveur est occupés n");
      exit (EXIT_SUCCESS);
    }
}



//ouverture de tube et lire les information passer dans ce tube 
void read_write (char *tube)
{

  int fd = -1;
  fd = ouverture_tube_client (tube);

  ssize_t n;
  char buf[BUF_SIZE + 1];

  while ((n = read (fd, buf, BUF_SIZE)) > 0)
    {
      buf[n] = '\0';
      printf ("%s", buf);
    }

  if (n == -1)
    {
      perror ("Erreur : lecture");
      exit (EXIT_FAILURE);
    }

  if (close (fd) == -1)
    {
      perror ("Erreur fermeture du descripteur");
      exit (EXIT_FAILURE);
    }


}

void clientPrompt ()
{
  printf ("\033[0;32m");
  printf (prompt);
  printf ("\033[0m");
}

//ouverture de tube 
int ouverture_tube_client(char *tube){
	int fd = open(tube,O_RDONLY);
	if(fd == -1){
		perror("Erreur d'ouverture du tube");
		exit(EXIT_FAILURE);
	}
	
	return fd;
}


/*fonction qui crée des thread ,chaque thread exécute 
 * la fonction runtype passer dans les argument et chaque thread lui donne un id 
  */
 
void creatThreads (void * (* run_type ) ( void *)){

  pthread_t th[NB_THREAD];
  ssize_t n;


  for (int i = 0; i < NB_THREAD; i++)
    {
		 int *arg = malloc (sizeof (*arg));
		  
		  if (arg == NULL)
		{
		  fprintf (stderr, "Couldn't allocate memory for thread arg.\n");
		  exit (EXIT_FAILURE);
		}

		  *arg = i;

		  n = pthread_create (&th[i], NULL, run_type, arg);
		  
		  if (n == -1)
		{

		  perror ("pthread_create");
		  exit (EXIT_FAILURE);
		}




    }
    


 }




