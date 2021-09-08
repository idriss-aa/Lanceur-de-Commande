
#include "File.h"


bool libre[NB_THREAD] = { false };

sem_t t_mutex[NB_THREAD];
fifo *fifo_p;
requete_t data[NB_THREAD];


int main (void)
{

  union sigval value;
  requete_t req;
  int i;
  int shm_fd;
  char *shm_ptr;
  
	//Cr�ation des Threads 
   creatThreads(&run);

	//Cr�ation d'une m�moire partag�e 
  if ((shm_fd = shm_open (NOM_SHM, O_RDWR | O_CREAT, 0666)) == -1)
    {
      perror ("shm_open");
      exit (EXIT_SUCCESS);
    }
    
	//on fixe la taille d'objet de m�moire partag�e
  if (ftruncate (shm_fd, TAILLE_SHM) == -1)
    {
      perror ("ftruncate");
      exit (EXIT_FAILURE);
    }
    
	//on projete l'objet de m�moire partag�e dans l'espace d'adressage virtuel du processus 
  shm_ptr = mmap (NULL, TAILLE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

  if (shm_ptr == MAP_FAILED)
    {
      perror ("sem_open");
      exit (EXIT_FAILURE);
    }
  fifo_p = (fifo *) shm_ptr;


	/*on initialise le s�maphore mutex avec la valeur 1 et on mets la valeur 1
	 *  au deux�ime param�tre pour que ce mutex soit partag�e entre plusieurs processus*/
  if (sem_init (&fifo_p->mutex, 1, 1) == -1)
    {
      perror ("sem_init");
      exit (EXIT_FAILURE);
    }
/*on initialise le s�maphore mutex avec la valeur NB_THread qui est d�finie dans le fichier File.h*/
  if (sem_init (&fifo_p->vide, 1, NB_THREAD) == -1)
    {
      perror ("sem_init");
      exit (EXIT_FAILURE);
    }
/*on initialise le s�maphore mutex avec la valeur 0 */
  if (sem_init (&fifo_p->plein, 1, 0) == -1)
    {
      perror ("sem_init");
      exit (EXIT_FAILURE);
    }



  fifo_p->tete = 0;
  fifo_p->queue = 0;
  
/*on initialise les mutex des threads avec la valeur 0 pour qu'il soit d�vrouiller*/
  for (int k = 0; k < NB_THREAD; ++k)
    {
      if (sem_init (&t_mutex[k], 1, 0) == -1)
		{
		  perror ("sem_init");
		  exit (EXIT_FAILURE);
		}
    }
    
    printf("\n Connection au Processus Demon �tablie... \n");

	//pour que notre daemon ne meurt jamais on fait une boucle infini
  while (1)
    {
			//on retire la requete 
		  req = defiler (fifo_p);
		  i = recherche (libre);
		  
		  /*
		   *si le retour de "recherche" est -1 on envoi un signal SIGUSER1 à l'utilisateur
		   *sinon on déclare que le thread n'est pas libre on stocke la requete defiler 
		   * dans la case de tableau reservé au thread 
		   *
		   * */
		  if (i != -1)
		{

		  libre[i] = true;

		  data[i] = req;

			//on déverouille le mutex de thread 
		  V (&t_mutex[i]);
		}

		  else
		{

		  sigqueue (req.pid, SIGUSR1, value);
		}
    }
    
  if (shm_unlink (NOM_SHM) == -1)
    {
      perror ("shm_unlink");
      exit (EXIT_FAILURE);
    }

  destroy (&fifo_p->mutex);
  destroy (&fifo_p->plein);
  destroy (&fifo_p->vide);
  
   exit (EXIT_SUCCESS);
}




void *run (void *args)
{

  int i = *((int *) args);
  free (args);
  int fd = -1;

	//une boucle infini pour qu'on puisse ré-utiliser notre thread
  while (1)
		{

		//on attend que notre mutex soit dévrouiller 
		  P (&t_mutex[i]);

		//on crée un processus
		  switch (fork ())
		{
		case -1:
			  perror ("fork");
			  exit (EXIT_FAILURE);
			  
		case 0:

				//on ouvre le tube passer dans la requete
			  fd = open (data[i].nom, O_WRONLY);
			  if (fd == -1)
				{

				  perror ("Erreur Open:");
				  exit (EXIT_FAILURE);
				}
				//on redérige la sortie standard dans le descripteur de fichierde tube
			  if (dup2 (fd, STDOUT_FILENO) == -1)
				{
				  perror ("dup2");
				  exit (EXIT_FAILURE);
				}
				//on redérige le standard d'ereur dans la sortie standard
			  if (dup2 (STDOUT_FILENO, STDERR_FILENO) == -1)
				{
				  perror ("dup2");
				  exit (EXIT_FAILURE);
				}
			//on execute la commande
			  arrange (data[i].cmd);

			  
			  if (close (fd) == -1)
				{
				  perror ("close");
				  exit (EXIT_FAILURE);
				}

			  if (unlink (data[i].nom))
				{
				  perror ("unlink");
				  exit (EXIT_FAILURE);
				}

		default:
			//on tue le procesuss zombie
			  if(wait (NULL)==-1){
				  perror("wait");
				  }
			//on déclare que le thread est libre 
			  libre[i] = false;

			}



    }

	//implicitement pthread exit
  return NULL;
}







