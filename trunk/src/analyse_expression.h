#ifndef ANALYSE_EXPRESSION
#define ANALYSE_EXPRESSION

#include "Shell.h"


#define NB_FONCTION 9

typedef int pid_t;
extern char * nom_fonction[NB_FONCTION]; //tableau contenant les noms des fonctions builtin

extern void interpreter(Expression * e);
extern void afficher_prompt(int retour);


#endif /* ANALYSE_EXPRESSION */

