# Initialisation de la base meteo

Le programme `initwsdb` initialise la base SQLite temporaire de la station.

Source :

- `/opt/windspots/bin/cpp/initwsdb/initwsdb.cpp`

Binaire installe :

- `/opt/windspots/bin/initwsdb`

## Role

`initwsdb` cree la base `${TMP}/ws.db` si elle n'existe pas, puis initialise le schema minimal :

- table `data`
- index `i1` sur `data(last_update)`
- table `log`

Il ne lit pas directement `/opt/windspots/etc/main`.

## Lien avec la configuration station

La configuration est chargee par `common.sh`, qui source `/opt/windspots/etc/main`.

Ensuite, `ws-configure.sh` appelle :

```sh
/opt/windspots/bin/initwsdb -s "${STATION}" -l "${LOG}" -t "${TMP}"
```

Les parametres importants pour `initwsdb` sont :

- `-l` : repertoire de log, utilise pour ecrire `windspots.log`
- `-t` : repertoire temporaire, utilise pour creer `ws.db`
- `-s` : nom de station, actuellement affiche mais pas stocke dans la base

## Reconfiguration depuis la WUI

Quand la WUI modifie `/opt/windspots/etc/main`, elle lance ensuite `ws-configure.sh`.

`ws-configure.sh` :

- arrete `w3rpi`
- archive l'ancienne base `ws.db`
- recree une base vide
- lance `initwsdb`
- met a jour `/etc/hosts`, `/etc/hostname` et `/etc/issue`
- enregistre la station via `upload_station.php`

## Points d'attention

Le schema SQLite existe aussi dans `w3rpi/eventManager.cpp`. Toute evolution du schema doit etre appliquee aux deux endroits tant qu'il n'y a pas de source unique.

Le reset de `ws.db` doit etre effectue pendant que `w3rpi` ne tourne pas, sinon il y a un risque de conflit d'acces SQLite.

## Build

`initwsdb` embarque une date de build via `INITWSDB_BUILD_DATE`, definie par le `Makefile` lors de la compilation. `version.h` contient la date de la version comme valeur de secours.

La date est injectee comme chaine C preprocesseur avec :

```make
CFLAGS += -DINITWSDB_BUILD_DATE=\"$(date +'%Y%m%d')\"
```

Les fichiers de dependances generes avec `-MMD` sont inclus par le `Makefile`. Une modification de `version.h` force ainsi la recompilation de `initwsdb`.

Le nettoyage `make clean` supprime les objets, fichiers de dependances et le binaire sans echouer si ces fichiers n'existent pas.
