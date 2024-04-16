# Table des matières
- [Prérequis](#item-1)
- [Fonctionnalitées](#item-2)
    - [Pourquoi ces fonctionnalitées ?](#item-3)
- [Librairies utilisées](#item-4)
- [Configuration](#item-5)
- [Problèmes rencontrés](#item-6)
    - [Lire des potentiomètres](#item-7)
    - [Déboguer](#item-8)
        - [Afficher float](#item-9)
    - [Création d'un projet](#item-10)
    - [Refonte du projet en Arduino](#item-11)
        - [Convertir float en integer](#item-12)
        - [Fonction dans la loop au lieu que dans l'audio callback](#item-13)
    - [Pin analog](#item-14)
    - [Intégration du MIDI](#item-15)
        - [Comment lire plusieurs notes](#item-16)
        - [Interpréter les fonctions avec le MIDI](#item-17)
- [Glossaire](#item-18)

<a id="item-1"></a>

# Prérequis
- Daisy seed
- 18 Potentiomètres
- 7 Switch à 3 états 
- Visual Studio Code

<a id="item-2"></a>

# Fonctionnalitées
Comme produit finale nous aimerions que le synthétiseur possède les différentes fonctionnalitées qu'on retrouve sur l'image : 
![Synthétiseur Fonctionnalitées](/images/Synth_V2.drawio.png)

<a id="item-3"></a>

## Pourquoi ces fonctionnalitées ?
En gros, ces fonctionnalitées nous permetterons de modifier le son qui sera produit par le [synthétiseur](#synthetiseur) à l'aide de boutons, potentiomètre et des switchs à 3 états

<a id="item-4"></a>

# Librairies utilisées
- [libDaisy](https://github.com/electro-smith/libDaisy) (aciennement utilisée)
- [DaisySP](https://github.com/electro-smith/DaisySP) (aciennement utilisée)
- [DaisyDuino](https://github.com/electro-smith/DaisyDuino)

## Fonction qu'on utilise le plus
- convertValue(value, min, max) Convertie la value entre le min et le max donné
- transposeOctave(freq, octaveDifference, midiMin, midiMax) baisse ou augmente d'un octave la frequence de l'oscillateur
- setFreq(freq) changer la fréquence de l'oscillateur (DaisyDuino)
- setAmp(amp) change l'amplitude de l'oscillateur (DaisyDuino)

<a id="item-5"></a>

# Configuration
Pour le moment, nous avons un filage bien normal, mais nous aimerions comme produit final avoir un pcb qui nous permetterait d'avoir un design plus jolie et moins bordélique (image à venir)

<a id="item-6"></a>

# Problèmes rencontrés

<a id="item-7"></a>

## Lire des potentiomètres
- Comme on n’avait jamais utilisé C++ pour lire des potentiomètres on ne pouvait pas utiliser les fonctionnes qu’on connaissait déjà : analogRead et digitalRead. Alors, après voir rechercher dans la librairie libDaisy on a trouvé qu’il était possible avec la classe `AdcChannelConfig` de lire la valeur de un ou plusieurs [potentiomètres](https://forum.electro-smith.com/t/adc-reading/541/2)

<a id="item-8"></a>

## Déboguer
- Encore une fois, comme nous avions jamais utilisé C++ pour déboguer ce type d’appareil nous avons du trouver une façon de déboguer, Après avoir regardé dans la librairie libDaisy on a trouvé qu’il y avait une façon de print à la console, mais quand on printait dans la console rien s’affichait et on a installé une extension de serial monitor pour voir s’il le print allait passer et il a bien passé dedans alors c’est en installant l'extension : [Serial Monitor par Microsoft](https://marketplace.visualstudio.com/items?itemName=ms-vscode.vscode-serial-monitor) que nous avons résolu notre problème.

<a id="item-9"></a>

### Afficher des floats dans le Serial Monitor
- Après avoir trouvé comment afficher à la console j'ai essayé de faire afficher un float, mais j'arrivais uniquement à faire afficher des int et plusieurs de nos valeurs se retrouvent entre 0 et 1. Alors, nous avons du trouver une [solution](https://forum.electro-smith.com/t/lost-half-day-on-figuring-out-how-to-debug-print-floats/2977) : `hw.PrintLine("My Float: " FLT_FMT(6), FLT_VAR(6, value))`. Grâce à cette ligne de code cela nous permet de déboguer et de s'assurer que nos calculs sont exacts.

<a id="item-10"></a>

## Création d'un projet cpp pour l'environnement daisy
- Il ne suffit pas d'un simple fichier C++ pour parvenir à envoyer du code sur le Daisy Seed, et on aimerait travailler dans un dossier d'exemple pour notre projet entier. Le créateur de la librairie a créé un [script](https://github.com/electro-smith/DaisyWiki/wiki/How-To-Create-a-New-Project) afin de simplifier la création de ces fichiers, qui sont très complexes : `/helper.py create seed/MyNewProject`.

<a id="item-11"></a>

## Refonte du projet en Arduino
- Nous avons opté pour Arduino pour simplifier la communication série avec l'ESP32 et faciliter la communication MIDI. Arduino offre une librairie riche et bien documentée pour le protocole MIDI. Cette transition nous permettra de bénéficier d'une plateforme de développement robuste et bien établie, réduisant ainsi le temps nécessaire pour résoudre les problèmes de communication.

<a id="item-12"></a>

### Convertir des floats en integer
- En utilisant la bibliothèque en C++, j'utilisais <static_cast int> { expression }, mais en Arduino, pour je ne sais quelle raison, il n'aimait pas la manière dont je procédais et ne me retournait aucun son à cause de cela, et j'ai dû utiliser int(expression) afin de convertir un float en integer pour que le son "revienne".

<a id="item-13"></a>

### Fonction dans la loop au lieu que dans l'audio callback
Avec la librairie en C++, j'avais toutes mes fonctions pour le son dans l'audio callback, mais certaines on des calculs et en Arduino il n'aime pas du tout avoir des calculs dans l'audio callback ce qui rendait le son différent. Alors, j'ai du refaire la structure du programme, mais au début je ne savais pas que c'était cela le problème et j'ai uniquement remarqué quand j'ai retiré les fonctions avec les calculs.

<a id="item-14"></a>

## Nombre limité de pin analog
- On s'est rendu compte qu'il allait nous manquer de pin analog, alors nous avons décidé de mettre les potentiomètres qui ne rentraient pas sur le daisy seed sur le ESP32 et il communiquera les informations des potentiomètres par serie.

<a id="item-15"></a>

## Intégration du [MIDI](#midi)

<a id="item-16"></a>

### Comment lire plusieurs notes
- Au début, nous avions aucune idée comment on allait lire plusieurs notes au même moment puisque on utilisait un potentiomètre pour contrôler la fréquence. Nous avons décider d'avoir plusieurs oscillateurs afin d'interpréter une note par [oscillateur](#oscillateur).

<a id="item-17"></a>

### Interpréter les fonctions avec le MIDI
- Comme j'ai dit plus haut, nous utilisions 1 seul potentiomètre pour gérer la fréquence et avec la communication midi on reçoit plusieurs fréquences d'un seul, alors nous avons dû adapter notre programme pour cela.

<a id="item-18"></a>

# Glossaire

<a id="oscillateur"></a>

- Oscillateur : Un oscillateur est un dispositif électronique capable de générer des signaux périodiques, tels que des signaux sinus, carrés ou triangulaires, à une fréquence déterminée.

<a id="midi"></a>

- MIDI : MIDI est un protocole de communication musical standard permettant le contrôle et la transmission de données entre instruments électroniques et ordinateurs.

<a id="synthetiseur"></a>

- Synthetiseur : Un synthétiseur est un instrument électronique capable de produire une variété de sons en utilisant des oscillateurs et des techniques de modulation.