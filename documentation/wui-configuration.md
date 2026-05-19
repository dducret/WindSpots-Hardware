# Interface web de configuration

La station expose une interface web de configuration dans `/opt/windspots/html/config`.

Cette interface est prevue pour etre utilisee depuis le reseau de management Bluetooth de la station. Le sous-reseau attendu est `10.10.10.0/24`.

## Fichiers modifies par la WUI

La WUI modifie directement les fichiers suivants :

- `/opt/windspots/etc/main`
- `/opt/windspots/etc/fswebcam.conf`
- `/opt/windspots/etc/wpa`
- `/tmp/reboot`

La WUI declenche aussi des commandes systeme controlees :

- `/opt/windspots/bin/eth0.sh up|down`
- `/opt/windspots/bin/wlan0.sh up|down`
- `/opt/windspots/bin/ppp0.sh up|down`
- `/opt/windspots/bin/ws-configure.sh`
- `/usr/bin/php /opt/windspots/bin/php/generate_wpa.php`

## Modele de permissions

Les fichiers de configuration applicatifs doivent rester modifiables par le serveur web, sans rendre tout le systeme modifiable par tous les utilisateurs.

Modele attendu :

- proprietaire principal : `windspots`
- groupe d'ecriture WUI : `www-data`
- repertoire `/opt/windspots/etc` : `2775`
- fichiers de configuration dans `/opt/windspots/etc` : `0664`
- fichier `/etc/wpa_supplicant/wpa_supplicant.conf` : `root:www-data` avec droits `0660`

Les fichiers systeme `/etc/hosts`, `/etc/hostname` et `/etc/issue` ne doivent pas etre rendus modifiables par tous. Ils sont mis a jour par `ws-configure.sh`, lance via une regle sudoers limitee.

## Politique sudoers WUI

Le fichier `/etc/sudoers.d/windspots-wui` autorise uniquement `www-data` a executer les commandes necessaires a la WUI.

La politique doit etre validee avec :

```sh
visudo -cf /etc/sudoers.d/windspots-wui
```

## Acces nginx

Les blocs nginx `/config` et `/config/*.php` utilisent une authentification basic et limitent l'acces au reseau Bluetooth `10.10.10.0/24` et a `127.0.0.1`.

Si une station doit etre administree depuis un autre reseau, modifier explicitement la configuration nginx plutot que de retirer toute restriction.

## WPA

`wifiwpa.php` ajoute les couples `SSID;mot_de_passe` dans `/opt/windspots/etc/wpa`, puis lance `generate_wpa.php` via sudo.

`generate_wpa.php` regenere `/etc/wpa_supplicant/wpa_supplicant.conf` a partir de cette source.

Les valeurs SSID et WPA ne sont pas passees directement au shell par `wifiwpa.php`.

`generate_wpa.php` valide le format source, la longueur des SSID et passphrases, genere la configuration via `wpa_passphrase`, puis remplace le fichier cible de maniere atomique.
