#include <stdio.h>
#include <stdlib.h>


typedef struct{
    int valeur;
    struct Arbre* fg;
    struct Arbre* fd;   
}Arbre;



Arbre* creationNoeud(int x){
    Arbre* arbre = malloc(sizeof(Arbre));
    if(arbre == NULL) exit(-1);
    arbre->valeur = x;
    arbre->fg = NULL;
    arbre->fd = NULL;

    return arbre;
}

Arbre* inserer(Arbre* arbre, int x){
    if(arbre==NULL) return creationNoeud(x);
    //chercher le noeud
    if(x < arbre->valeur){
        arbre->fg = inserer(arbre->fg, x);
    }

    if(x>arbre->valeur){
        arbre->fd = inserer(arbre->fd, x);
    }

    return arbre;
}

Arbre* rechercheNoeud(Arbre* arbre, int x){

    if(arbre == NULL) return -1;
    
    if(arbre->valeur == x) return 1;

    if(x < arbre->valeur){
        return rechercheNoeud(arbre->fg, x);
    }

    if(x > arbre->valeur){
        return rechercheNoeud(arbre->fd, x);
    }

    return -1;
}

Arbre* minNoeud(Arbre* arbre){
    
    if(arbre == NULL) return NULL;     
    
    if(arbre->fg != NULL){
        return minNoeud(arbre->fg);
    }
    else{
        return arbre;
    }
}


Arbre* maxNoeud(Arbre* arbre){
    if(arbre == NULL)return NULL;
    
    if(arbre->fd != NULL){
        return maxNoeud(arbre->fd);
    }

    else{
        return arbre;
    }
}


Arbre* supprimerNoeud(Arbre* arbre, int x){

    if(arbre == NULL) return NULL;

    //On cherche le noeud
    if(x < arbre->valeur){
        arbre->fg = supprimerNoeud(arbre->fg, x);
        return arbre;
    }

    if(x > arbre->valeur){
        arbre->fd =  supprimerNoeud(arbre->fd, x);
        return arbre;
    }
    
    // Donc la x == arbre->valeur

    /*
    3 cas possibles -   0 fils
                    -   1 fils (droit ou gauche)
                    -   2 fils (droit et gauche)

    */
    
    //0 fils
    if(arbre->fg == NULL && arbre ->fd == NULL){
        free(arbre);
        return NULL;
    }
    
    //1 fils gauche
    if(arbre->fd == NULL && arbre->fg != NULL){
        Arbre* temp = arbre->fg;
        free(arbre);
        return temp;
    }
    
    //1 fils droit
    if(arbre->fg == NULL && arbre->fd != NULL){
        Arbre* temp = arbre->fd;
        free(arbre);
        return temp;
    }
    
    //2 fils

    Arbre* succ = minNoeud(arbre->fd);

    arbre->valeur = succ -> valeur;

    arbre->fd = supprimerNoeud(arbre->fd, succ->valeur);


    return arbre;

}





void parcourPrefixe(Arbre* arbre){
    if(arbre == NULL) return;
    printf("|%d ", arbre->valeur);
    parcourPrefixe(arbre->fg);
    parcourPrefixe(arbre->fd);
}

void parcourInfixe(Arbre* arbre){
    if(arbre == NULL) return;
    parcourInfixe(arbre->fg);
    printf("|%d ", arbre->valeur);
    parcourInfixe(arbre->fd);
}


void parcourSuffixe(Arbre* arbre){
    if(arbre == NULL) return;
    parcourSuffixe(arbre->fg);
    parcourSuffixe(arbre->fd);
    printf("|%d ", arbre->valeur);
}





int main(void){

    Arbre* arbre = NULL;
    
    arbre = creationNoeud(5);
    arbre = inserer(arbre, 3);
    arbre = inserer(arbre, 50);
    arbre = inserer(arbre, 1);
    arbre = inserer(arbre, 15);
    arbre = inserer(arbre, 17);
    arbre = inserer(arbre, 99);

    parcourInfixe(arbre);

    printf("\n");
    return 0;
}
