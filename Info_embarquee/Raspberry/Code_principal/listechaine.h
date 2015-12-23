#ifndef H_LISTE_CHAINE
#define H_LISTE_CHAINE

typedef struct cellule *Ptcellule;

typedef struct cellule 
{
	int msg;	
	Ptcellule suivant;
} Cellule;

typedef Ptcellule Liste_chainee;

////
// Fonctions sur listes chainées
////

Ptcellule allouer();
void ajout_tete(Ptcellule *, int);
void sup_tete(Ptcellule *);
void sup_liste(Ptcellule *);



#endif