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

## Critere de succes

L'installation doit se derouler jusqu'a la fin sans erreur.

Si une commande echoue, conserver le message d'erreur complet avant de relancer l'installation ou de modifier la configuration.

