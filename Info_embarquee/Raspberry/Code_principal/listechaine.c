#include <stdio.h>
#include <stdlib.h>
////
// Fonctions sur listes chainées
////

typedef struct cellule *Ptcellule;

typedef struct cellule 
{
	int msg;	
	Ptcellule suivant;
} Cellule;

typedef Ptcellule Liste_chainee;


Ptcellule allouer()
{
	return(malloc(sizeof(Cellule)));
}

void ajout_tete(Ptcellule *L, int commande)
{
	Ptcellule P;
	P = allouer();
	P->msg = commande;
	P->suivant = *L;
	*L = P;
}
void sup_tete(Ptcellule *L)
{
	Ptcellule P;
	P = *L;
	*L = (*L)->suivant;
	free(P);
}
void sup_liste(Ptcellule *L)
{
	while(*L != NULL)
	{
		sup_tete(L);
	}
}
