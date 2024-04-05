# Documentation de la communication wifi

Bienvenue dans la documentation de la communication wifi ! Ce document fournit un aperçu de la communication wifi entre le client web, le serveur de Music Maker et l'oscilloscope.

## Table des matières

- [Fonctionnalités](#fonctionnalités)
- [Configuration](#Configuration)
- [Installation](#installation)
- [Problèmes courants](#Problèmes)


## Fonctionnalités

La communication wifi comprend les fonctionnalités suivantes :

- Envoi de notes de musique en temps réel via wifi à partir d'un client web
- Affichage des notes sur un oscilloscope
- Envoie de données midi à un synthétiseur externe

## Installation

Pour utiliser la communication, suivez ces étapes :

1. Clonez le dépôt : `git clone `

2. Ouvrez le fichier `wifi.ino` dans l'IDE Arduino. 

3. Selectionnez la carte 'ESP32-WROOM-DA Module' lorsque vous compilez le code.

4. Téléversez le code sur votre ESP32.

## Configuration

Pour utiliser la communication wifi, suivez ces étapes :
 
1. Avoir suvi les étapes d'installation

2. Connectez vous au réseau wifi de l'ESP32 sur votre client web

3. Le client web est maintenant connecté à l'ESP32 et peut envoyer des données midi!

## Problèmes

**Je ne vois pas mon le reseau wifi de mon ESP32.**

Assurez-vous que votre ESP32 est suffisament alimenté et que le code est correctement téléversé. Vous pouvez également essayer de redémarrer votre ESP32 à l'aide du bouton physique.

**Je ne reçois pas de données de mon client.**

Assurez-vous que le client est connecté sur le réseau wifi de l'ESP32. Assurez-vous que les lignes de code suivantes sont bien présent dans le code du ESP32 :

```cpp
    client.println("Access-Control-Allow-Origin : *");
    client.println("Access-Control-Allow-Methods : GET,PUT,POST,DELETE");
    client.println("Access-Control-Allow-Headers : Content-Type");
```

Ces lignes permettent de gérer les requêtes CORS, qui sont nécessaires pour que le client puisse envoyer des données à l'ESP32.

