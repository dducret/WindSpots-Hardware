# Installation de base sur Raspberry Pi 3

Ce guide decrit l'installation initiale d'une station WindSpots MeteoStation sur une Raspberry Pi 3 a partir d'une image Raspberry Pi OS 32 bits sans environnement graphique.

## Materiel cible

- Raspberry Pi 3
- Carte microSD adaptee
- Acces reseau pour telecharger le script d'installation
- Acces SSH ou console locale

## Preparation de l'image

Generer la carte microSD avec Raspberry Pi Imager.

Parametres attendus :

- modele : Raspberry Pi 3
- systeme : Raspberry Pi OS 32 bits sans desktop
- utilisateur : `debian`
- mot de passe : `Geneva22$`
- SSH : active
- console : activee

Ecrire l'image sur la carte microSD, puis inserer la carte dans la Raspberry Pi 3.

## Premier demarrage

Demarrer la Raspberry Pi sur l'image preparee.

Se connecter avec l'utilisateur configure :

```sh
login: debian
password: Geneva22$
```

Passer en mode root :

```sh
su -
```

## Installation de WindSpots MeteoStation

Depuis le shell root, telecharger le script d'installation :

```sh
wget -O "install.sh" "https://station.windspots.org/install/install.sh"
```

Rendre le script executable :

```sh
chmod +x *.sh
```

Lancer l'installation :

```sh
./install.sh
```

## Commandes utiles

L'installation conserve un etat de reprise dans `/var/lib/windspots-install/state`.

Pour lancer l'installation sans redemarrer automatiquement a la fin :

```sh
./install.sh --no-reboot
```

Pour relancer toutes les etapes depuis le debut, sans redemarrer automatiquement :

```sh
./install.sh --reset-state --no-reboot
```

Pour relancer toutes les etapes depuis le debut et redemarrer automatiquement si la validation finale reussit :

```sh
./install.sh --reset-state
```

Pour consulter l'etat des etapes deja terminees :

```sh
cat /var/lib/windspots-install/state
```

Chaque execution ecrit aussi un fichier de log dans le repertoire de l'utilisateur `debian`, avec un nom du type :

```sh
/home/debian/log-YYYYMMDD-HHMMSS.txt
```

## Critere de succes

L'installation doit se derouler jusqu'a la fin sans erreur.

Si une commande echoue, conserver le message d'erreur complet avant de relancer l'installation ou de modifier la configuration.
