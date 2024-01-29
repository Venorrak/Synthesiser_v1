# Énoncé – Travail ÉQUIPE
Équipe no : E  

Noms : Justin, Miguel et Vincent

# But Général du Projet
Le projet vise à concevoir et réaliser un synthétiseur à coût réduit en utilisant le module *Daisy Seed*. L'objectif est de démontrer les effets d'un synthétiseur sur le voltage traduit en son, à travers une communication série et l'utilisation d'un oscilloscope. Le produit final sera un synthétiseur open source, pouvant être reproduit avec des connaissances de base et à un prix abordable.
Définition du type de projet (Conception ou vulgarisation)

# Lien avec un Produit Existant et Explication des Différences
Aucun synthétiseur à coût réduit n'est actuellement disponible sur le marché. Le projet se distingue par son approche open source, offrant la possibilité de reproduire le synthétiseur chez soi.

# Justification

### Résultat Attendu (Produit Final Concret)
Le résultat attendu est un synthétiseur fonctionnel avec la capacité de générer des effets sonores variés. Le produit final comprendra également un écran d'oscilloscope pour visualiser les variations de signal.

### Existe-t-il quelque chose de semblable?

Un projet de synthétiseur open source tel que le notre existe déjà et il s'appelle "LMN-3". C'est un synthétiseur basé sur un board teensy 4.1 et un raspberry pi 4 qui est entièrement controllé à l'aide de son interface composé de boutons et d'encodeurs.

### En quoi votre projet est-il pertinent ? 



### Pourquoi ce projet et pas un autre? 



### Objectif final?

Produire un synthétiseur (open source) qui pourra interpréter des signaux midi envoyé par un clavier ou par des notes envoyés par un site web à l'aide de communication bluetooth. L'utilisateur pourra modifier les sons provenant du synthétiseur sur le moment et modifiant les effets à l'aide de potentiomètres et de boutons.

### Par quelle démarche vous y prendrez-vous?

Faire le projet un module à la fois du moins au plus prioritaire.

Jalons (pas en ordre) :
- Module de réception MIDI
- Synthétiseur qui interprête les notes
- Écran qui montre le son de façon visuel
- Communication bluetooth des notes avec site web
- Interfaçe web
- PCB
- Boitier imprimé 

### Quelles questions vous suscite-t-il?

- Comment communiquer les notes au sytnétiseur ?
- Comment interpreter le signal externe ?
- Quels effets seront disponibles sur le syntétiseur ?
- Comment voir les repercutions du changement de l'audio par les effets?

# audience visée
Le projet cible les passionnés de musique, les amateurs d'électronique et les personnes intéressées par la création de leurs propres instruments de musique.

# Démarche préliminaire
**Voici les étapes afin de mener le projet à terme.**

|Étapes|Temps|Justification|
|----------------------------------------------------------------|:----:|---------------------------------|
|Trouver microcontroleur pouvant gérer des signaux audio et simuler un oscillateur et trouver une librairie compatible.| 5h | Plusieurs choix avec des avantages différents pour chacun |
|Définir les fonctionnalités attendues.| 3h | Prendre le temps de définir à l'avance nos objectifs pour ne pas travailler dans le vide |
|Implémenter les fonctionnalitées sur un bread board prototype.| 5h | nécessaire pour tester le code et pour avoir un plan général de ce qu'on aura besoin pour le produit final |
|Consulter la documentation afin d'implémenter les fonctionnalités efficacement.| 20h | Les librairies utilisées avec le daisy seed sont très complètes il faudra donc du temps pour comprendre comment elle marche. |  
|Programmation du synthétiseur.| 30h | Difficultées probables causés par des éléments spécifiques du synthétiseur lors de la programmation (oscilloscope, communication MIDI) |
|Recherches et implémentation d'un oscilloscope sur le synthétiseur | 20h | L'oscioscope utilise une autre librairie complètement et nous allons probablement utiliser un autre arduino pour le faire fonctionner. |
|Conceptualisation d'un PCB pour remplacer le breadboard| 20h | Malgré le fait que je ne pense pas que cela soit compliqué, je n'ai jamais fait de PCB et je sais que c'est un long processus donc je donne une grande fenêtre de temps. -*justin* |
|Conception d'un case 3D + impression|4h| Le temps de faire une boite en 3D adapté aux dimensions du synthé |
<!-- Chaque étape doit inclure une évaluation de temps approximatif avec justification. -->


# Matériel
Matériel dont nous aurons besoin pour mener le projet à terme.

| Item | Justification | Coût avec livraison| 
|------|---------------|:------:|
| Daisy seed (Oscillateur) | Module Principal | 50.00$ |
| Oscilloscope (écran)| Afficher les donnés | 10.00$ |
| Divers | Potentiomètre, fils, résistance, etc | 20.00$ |
| PCB | Finition finale | 20$ - 35$ |
| Case imprimé 3D | Protection du produit | 2.00$ |

# Ressource disponible
Les ressources disponibles pour la réalisation du projet incluent le module Daisy Seed, un oscilloscope, divers composants électroniques (potentiomètres, fils, résistances, etc.), ainsi que des ressources en ligne pour la programmation du microcontrôleur tel que la documentation de la librairie du daisy seed, des chaines youtube spécialisées, un sever discord et des forums dédiés au daisy seed.

# Apprentissages à Faire
Le projet implique l'apprentissage de la programmation d'un microcontrôleur pour gérer des signaux audio, la compréhension des librairies associées au Daisy Seed, la mise en œuvre d'un oscilloscope, ainsi que la conception et l'impression 3D d'un boîtier et un PCB pour le synthétiseur.

# Exigences Particulières

Les principales exigences comprennent la compatibilité des librairies avec le Daisy Seed, la gestion des signaux audio, la communication série, et la création d'un oscilloscope intégré.



