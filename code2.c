

typedef struct{
    int valeur;
    struct Arbre* fg;
    struct Arbre* fd;
}Arbre*


Arbre* creationNoeud(int x){
    Arbre* arbre = malloc(sizeof(Arbre));
    if(arbre==NULL)exit(-1);

    arbre->valeur = x;
    arbre->fg = NULL;
    arbre->fd = NULL;
    
    return arbre;
}


void parcoursInfixe(Arbre* arbre){
    if(arbre==NULL)return NULL;
    parcoursInfixe(arbre->fg);
    printf("|%d ", arbre->valeur);
    parcoursInfixe(arbre->fd);
}



Arbre* rechercheABR(Arbre* arbre, int x){
    if(arbre==NULL)return NULL;

    if(arbre->valeur == x)return arbre; 


    if(x < arbre->valeur){
        return rechercheABR(arbre->fg, x);
    }
    if(x > arbre->valeur){
        return rechercheABR(arbre->fd, x);
    }
}





