=====================================
        PROJET WILDWATER
=====================================

-------------------------------------
PRÉREQUIS
-------------------------------------
- GCC (compilateur C)
- Make
- Gnuplot
- Bash
- Fichiers de données au format CSV, c'est dans ce dossier qu'il faudra mettre le fichier CSV !!!!!!!!
  (exemple : exemple/data.csv)

-------------------------------------
PRÉPARATION
-------------------------------------

Avant toute génération d’histogrammes ou de leaks,
il est obligatoire de compiler les programmes C.

Rendre le script exécutable :
chmod +x scripts/script.sh

Compiler les programmes C :
make

Cette commande génère l’exécutable :
exec/Wildwater

-------------------------------------
HISTOGRAMMES
-------------------------------------

Les histogrammes sont générés via le script principal.
Chaque exécution régénère automatiquement les fichiers,
ce qui permet de relancer facilement avec un nouveau CSV.

Commande générale :
bash scripts/script.sh exemple/<nom_du_fichier.csv> histo <mode>

Modes disponibles :

[ MAX ] Capacités maximales des usines
bash scripts/script.sh exemple/data.csv histo max

[ SRC ] Volumes captés par usine
bash scripts/script.sh exemple/data.csv histo src

[ REAL ] Volumes réellement traités
bash scripts/script.sh exemple/data.csv histo real

Si l’histogramme MAX n’existe pas lors du lancement
de SRC ou REAL, il est généré automatiquement.

[ ALL ] Génération complète (MAX + SRC + REAL)
bash scripts/script.sh exemple/data.csv histo all

-------------------------------------
LEAKS
-------------------------------------

Calcul des fuites pour une usine donnée :
bash scripts/script.sh exemple/<nom_du_fichier.csv> leaks "Facility complex #RH400057F"

Si l’identifiant fourni ne correspond pas à une usine valide,
le traitement est arrêté avec un message d’erreur.

-------------------------------------
NETTOYAGE
-------------------------------------

Suppression de l’exécutable et de tous les fichiers générés :
make clean

-------------------------------------
ORGANISATION DES FICHIERS
-------------------------------------

data/histo   : CSV et PNG des histogrammes
data/dat     : fichiers DAT intermédiaires (Gnuplot)
data/tmp     : fichiers temporaires
data/leaks   : résultats leaks par usine

=====================================
