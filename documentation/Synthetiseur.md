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
    - [Pin analog](#item-11)

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
En gros, ces fonctionnalitées nous permetterons de modifier le son qui sera produit par le synthétiseur à l'aide de boutons, potentiomètre et des switchs à 3 états

<a id="item-4"></a>

# Librairies utilisées
- [libDaisy](https://github.com/electro-smith/libDaisy)
- [DaisySP](https://github.com/electro-smith/DaisySP)

## Fonction qu'on utilise le plus
- GetPin(pinNumber) Configurer ou manipuler les broches GPIO (libDaisy)
- InitSingle(GetPin(pinNumber)) Fonctioner qui provient de la classe AdcChannelConfig qui sert à initializer une seule pin ADC
- Get(channelNumber) Retourne la valeur du channel associé à une pin qui se trouve entre 0 et 65536 (libDaisy)
- convertValue(value, min, max) Convertie la value entre le min et le max donné
- transposeOctave(freq, octaveDifference, midiMin, midiMax) baisse ou augmente d'un octave la frequence de l'oscillateur
- setFreq(freq) changer la fréquence de l'oscillateur (DaisySP)
- setAmp(amp) change l'amplitude de l'oscillateur (DaisySP)

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

## Nombre limité de pin analog
- On s'est rendu compte qu'il allait nous manquer de pin analog, alors nous avons décidé de mettre les potentiomètres qui ne rentraient pas sur le daisy seed sur le ESP32 et il communiquera les informations des potentiomètres par serie.

# Glossaire