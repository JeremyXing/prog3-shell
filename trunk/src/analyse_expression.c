#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "analyse_expression.h"
#include "commande.h"



/*----------- Tableau des fonctions internes(commande.h) ---------- */
#define NB_FONCTION 2
char * nom_fonction[NB_FONCTION] = { "pwd", "cd" };
fonction  tableau_fonction[NB_FONCTION] = { pwd, cd };

typedef int pid_t;

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
      (f)(e->arguments); // appel de la fonction interne
      break;
    }
  }

  if(! trouver){
    if(fork() == 0){
      execlp(e->arguments[0], NULL, NULL);
      perror("");
    }
  }
}

/* Analyse de l'expression */
void analyse_cmd(Expression * e){
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


