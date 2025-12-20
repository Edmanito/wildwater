#ifndef LEAKS_H
#define LEAKS_H

/*
 * Calcule le volume total de fuites pour une usine donnée.
 *
 * csv_path    : chemin vers le fichier CSV
 * plant_id    : identifiant exact de l'usine
 * out_csv     : fichier résultat (data/leaks/...)
 * history_dat : fichier historique (.dat, ajout)
 *
 * Retour :
 *   0  -> succès
 *  >0  -> erreur
 */
int leaks_compute(
    const char *csv_path,
    const char *plant_id,
    const char *out_csv,
    const char *history_dat
);

#endif
