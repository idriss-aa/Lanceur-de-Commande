
#include "File.h"






int main (int argc, char *argv[])
{
	
  fifo *fifo_p = NULL;
  requete_t req;
  struct sigaction action;
  pid_t pid = getpid ();
  char x[10];
  char n[10] = NOM_TUBE;
  char tube[10];
  char c[128];

  action.sa_flags = SA_SIGINFO;
  action.sa_sigaction = &sighand;
  
  //Si le server envoi un signal . 
  if (sigaction (SIGUSR1, &action, NULL) == -1)
    {
      perror ("sigusr: sigaction");
      return 0;
    }

  clientPrompt ();



/////////////////////// SHM////////////////////////


	//ouvrir la mémoire partagée par l'appel système shm_open
  int shm_fd = shm_open (NOM_SHM, O_RDWR, S_IRUSR | S_IWUSR);

  if (shm_fd == -1)
    {
      perror ("shm_open");
      exit (EXIT_FAILURE);
    }

	//on projete l'objet de mémoire partagée dans l'espace d'adressage virtuel du processus 
  char *file = mmap (NULL, TAILLE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

  if (file == MAP_FAILED)
    {
      perror ("mmap");
      exit (EXIT_FAILURE);
    }

  fifo_p = (fifo *) file;

  if (close (shm_fd) == -1)
    {
      perror ("close");
      exit (EXIT_FAILURE);
    }



///////////////////////Remplissage de la requete ////////////////////////

  if (argc < 1)
    {
      printf ("Erreur \n");
      exit (EXIT_FAILURE);
    }

	  sprintf (x, "%d", pid);

      strcpy (tube, strcat (n, x));

      create_tube_client (tube);

  while (fgets (c, 128, stdin) != NULL)
    {

      c[strlen (c) - 1] = '\0';
      //si le client veut quitter il peut ecrire quit et le processus vas envoyer un signal SIGTERM C à lui meme

      if (strncmp (c, "quit", strlen ("quit")) == 0)
	{
		//si le client tape la commande quit le processus envoi un signal SIGTERM Ã  lui meme
	  if (kill (pid, SIGTERM) == -1)
	    {
	      perror ("Erreur kill");
	      exit (EXIT_FAILURE);
	    }

	}


      strcpy (req.cmd, c);
      
      strcpy (req.nom, tube);
      
      req.pid = pid;
      //on ajoute la requete Ã  la file
      
      enfiler (fifo_p, req);
      
      read_write (tube);
      
      clientPrompt ();

    }



  exit (EXIT_SUCCESS);
}






