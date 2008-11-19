#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

#include "analyse_expression.h"
#include "commande.h"
#include "Shell.h"




/*----------- Tableau des fonctions internes(commande.h) ---------- */
char * nom_fonction[NB_FONCTION] = { "pwd", "cd", "history", "builtins", "kill", "times", "exit" };
fonction  tableau_fonction[NB_FONCTION] = { pwd, cd , history, builtins, killer, times, toexit};

/*Le nom de la fonction n'est pas oblig� de corespondre a la fonction (ex: "kill" associ� a killer()), ca fonctionne.*/
/*-----------------------------------------------------------------*/

int ecrire_history(char ** arguments){
  int argc = LongueurListe(arguments);
  int fichier = open("history.tmp", O_WRONLY | O_CREAT | O_APPEND, 0644);
  if(fichier == -1){
    perror("");
    return 1;
  }

  int i;
  char * espace = " ";
  for(i = 0; i < argc; i++){
    write(fichier, arguments[i], strlen(arguments[i]) * sizeof(char));
    write(fichier, espace, sizeof(char));
  }
  
  char * back = "\n";
  write(fichier, back, sizeof(char));
  close(fichier);  
  return 0;
}

/*Execute le tube suivant : gauche | droite */
static void tube(Expression * gauche, Expression * droite){

 if(fork()==0){ //Processus fils du mini Shell
    int tube[2];
    pipe(tube);
    //int status;

    pid_t processus = fork();
    switch(processus){
    case 0: //fils
      dup2(tube[1], 1);
      close(tube[0]);
      close(tube[1]);
      execvp(gauche->arguments[0], gauche->arguments);
      perror("exec");
      break;
    default: //pere
      //waitpid(0, &status, WNOHANG);
      wait(NULL);
      dup2(tube[0],0);
      close(tube[0]);
      close(tube[1]);
      execvp(droite->arguments[0], droite->arguments);
      perror("exec");
      break;
    }
  }
}

/* Parcours infixe de l'abre */
void arbre(Expression * racine){ // parcours infixe
  if(racine != NULL){
    arbre(racine->gauche); //gauche

    switch(racine->type){ // racine
    case VIDE:
      printf("vide ");
      break;
    case SIMPLE:
      printf("%s ",racine->arguments[0]);
      break;
    case SEQUENCE:
      printf("; ");
      break;
    case SEQUENCE_ET:
      printf("&& ");
      break;
    case SEQUENCE_OU:
      printf("|| ");
      break;
    case BG:
      printf("&");
      break;
    case PIPE:
      printf("| ");
      break;
    case REDIRECTION_I:
      printf("< %s ",racine->arguments[0]);
      break;
    case REDIRECTION_O:
      printf("> %s ",racine->arguments[0]);
      break;
    case REDIRECTION_A:
      printf(">> %s ",racine->arguments[0]);
      break;
    case REDIRECTION_E:
      printf("2> %s ",racine->arguments[0]);
      break;
    case REDIRECTION_EO:
      printf("&> %s ",racine->arguments[0]);
      break;
    default:
      printf("inconnue ");
      break;
    }
    arbre(racine->droite); // droite
  }
}


/*Recherche la commande dans le tableau des fonctions internes et l'execute si elle existe*/
void executer_cmd(Expression * e){
  bool trouver = false;
  char * nom_commande = e->arguments[0];

  for(int i=0; i < NB_FONCTION; i++){
    if( strcmp(nom_fonction[i], nom_commande) == 0 ){
      trouver = true;
      fonction f = tableau_fonction[i];
      int retour = (f)(e->arguments); // appel de la fonction interne
      afficher_prompt(retour);
      break;
    }
  }

  if(! trouver){
    int retour;
    if(fork() == 0){
      //execlp(e->arguments[0], NULL, NULL);//Ne fonctionne pas, ne tient pas compte des options
      retour = execvp(e->arguments[0], e->arguments );
      if(retour == -1)
	fprintf(stderr, "%s : command not found\n", e->arguments[0]);
      exit(EXIT_FAILURE);
    }
    retour = wait(NULL);
    if(retour == -1)
      afficher_prompt(1);
    else afficher_prompt(0);
  }

}

/* Analyse de l'expression */
void analyse_cmd(Expression * e){

  ecrire_history(e->arguments);

  switch(e->type){
  case SIMPLE:
    executer_cmd(e); // Dans le cas d'une commande simple, on l'execute.
    break;
  case PIPE:
    tube(e->gauche, e->droite);
    break;
  default:
    printf("Seules les commandes SIMPLES sont execut�es.\n");
    break;
  }
}

/*Contruction du prompt */
void afficher_prompt(int retour){

  /* Recuperation de whoami */
  char * user = NULL;
  user = getenv("USER");//extrait le contenu de la variable d'environnemtn USER
  if(user == NULL)
    user = "unknown_user";

  
  /* Code de pwd() */
  long size;
  char *buf;
  char *ptr;
  
  size = pathconf(".", _PC_PATH_MAX);
  if ((buf = (char *)malloc((size_t)size)) != NULL)
    {
      ptr = getcwd(buf, (size_t)size);
    }
  /* Fin Code de pwd() */  

  if(retour == 1)
    retour = 1;
  else retour = 0;
  
  printf("%s [%s] %d -$ ", user, ptr, retour);
}




