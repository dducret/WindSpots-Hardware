# Tests cible apres installation

Cette fiche liste les controles a executer sur une Raspberry Pi Debian Trixie apres installation ou apres modification des scripts.

## Installation

```sh
cd /tmp
wget -O install.sh https://station.windspots.org/install/install.sh
chmod +x install.sh
./install.sh --no-reboot
```

Controles attendus:

- le script se termine sans erreur;
- le journal `/var/log/windspots-install.log` ne contient pas d'erreur bloquante;
- le fichier d'etat `/var/lib/windspots-install/state` contient les etapes terminees;
- une relance de `./install.sh --no-reboot` reprend sans rejouer les etapes deja marquees.

Pour forcer une installation complete:

```sh
./install.sh --reset-state --no-reboot
```

## Services et configuration systeme

```sh
nginx -t
visudo -cf /etc/sudoers.d/windspots-wui
systemctl is-enabled nginx
systemctl is-enabled php8.4-fpm
systemctl is-enabled bluetooth
systemctl is-enabled NetworkManager
```

Controles attendus:

- la configuration nginx est valide;
- la politique sudo limitee de la WUI est valide;
- les services necessaires sont actives au demarrage.

## Permissions

```sh
stat -c '%U:%G %a %n' \
  /opt/windspots/etc/main \
  /opt/windspots/etc/fswebcam.conf \
  /opt/windspots/etc/wpa \
  /etc/wpa_supplicant/wpa_supplicant.conf \
  /var/log/windspots.log
```

Controles attendus:

- les fichiers de configuration WindSpots sont modifiables par le groupe `www-data` ou `windspots` selon leur usage;
- `/etc/wpa_supplicant/wpa_supplicant.conf` reste lisible seulement par `root` et `www-data`;
- aucun fichier sensible n'est en mode `777`.

## Interface web de configuration

Depuis le reseau Bluetooth `10.10.10.0/24`, ouvrir:

```text
http://10.10.10.1/config/
```

Controles attendus:

- l'authentification basic auth est demandee;
- l'interface `/config` est accessible depuis le reseau Bluetooth;
- l'interface `/config` est refusee depuis un autre reseau;
- l'enregistrement Wi-Fi ajoute une entree dans `/opt/windspots/etc/wpa` et regenere `/etc/wpa_supplicant/wpa_supplicant.conf`;
- les actions Ethernet, Wi-Fi, PPP et reconfiguration station retournent un statut coherent dans l'interface.

## Base meteo et initwsdb

```sh
cd /opt/windspots/bin/cpp/initwsdb
make clean
make
/opt/windspots/bin/cpp/initwsdb/initwsdb --help
sqlite3 /var/tmp/windspots/ws.db '.schema'
```

Controles attendus:

- `initwsdb` compile sur la cible;
- la base `/var/tmp/windspots/ws.db` existe apres demarrage ou reconfiguration;
- le schema contient les tables attendues pour les donnees meteo.

## Scripts principaux

```sh
sh -n /opt/windspots/bin/*.sh
bash -n /opt/windspots/bin/boot.sh
sudo -u www-data sudo -n /opt/windspots/bin/eth0.sh up
sudo -u www-data sudo -n /opt/windspots/bin/eth0.sh down
```

Les commandes de test reseau modifient l'etat de la station. Les executer seulement sur une cible de validation.
