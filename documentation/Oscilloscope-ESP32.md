
# Table des matières
- [Prérequis](#item-1)
- [Fonctionnalitées](#item-2)
- [Configuration](#item-3)
    - [Pinout Final](#item-4)
    - [Pinout prototype](#item-5)
- [Code](#item-6)
    - [Librairies](#item-7)
    - [Base de départ du code](#item-8)
    - [Comment ça marche ?](#item-9)

<a id="item-1"></a>

# Prérequis
- ESP32 **devkit v1**
- Arduino IDE
- Écran OLED (***SSD1309***)
- 3 boutons
- 5 potentiomètres

<a id="item-2"></a>

# Fonctionnalitées

L'ESP32 va prendre en charge plusieurs fonctionnalitées du synthétiseur

- Recevoir les notes provenant du site web à l'aide du module wifi
- Transformer et transferer les notes au Daisy seed
- transferer les valeurs des potentiomètres au Daisy seed
- Affiche sur l'écran la forme du signal audio qui sort du Daisy seed
- Intéragis avec les boutons qui modifient l'interface de l'écran

<a id="item-3"></a>

# Configuration

**Croquis du filage sans le PCB** : 
![ESP32 diagramme](/images/ESP32_config.png)

<a id="item-4"></a>

## Pinout Final

- 5V -> VCC (screen)
- GND -> GND
- D15 -> Bouton 1
- D2 -> Bouton 2
- D4 -> Bouton 3
- D21 -> SDA
- D22 -> SDK
- A0 / D13 -> audio
- TX -> Rx (daisy seed)
- D12 -> potentiomètre 1
- D14 -> potentiomètre 2
- D27 -> potentiomètre 3
- D26 -> potentiomètre 4
- D25 -> potentiomètre 5

<a id="item-5"></a>

## Pinout prototype
Ce pinout à été crée pendant les tests de l'oscilloscope avec arduino.
Le programme va utiliser ces pins jusqu'au transfert vers le ESP32.

- 5V -> VCC
- GND -> GND
- 2 -> Bouton 1
- 10 -> Bouton 2
- 11 -> Bouton 3
- SDA -> SDA
- SDK -> SDK
- A0 -> audio
- TX -> Rx (daisy seed)

<a id="item-6"></a>

# Code
Description du fonctionnement du code

<a id="item-7"></a>

## Librairies

- WIRE.h *
- Adafruit_GFX.h
- Adafruit_SSD1309.h
- OneButton.h

\* = intégré dans le arduino IDE.

Adafruit_GFX, Adafruit_SSD1309 & OneButton Peuvent être trouvés dans le library manager de arduino IDE.

<a id="item-8"></a>

## Base de départ du code

Le code pour simuler un oscilloscope à été tiré d'un repository github "[Scope-O-Matic](https://github.com/josbouten/Scope-O-Matic/tree/master?tab=readme-ov-file)". Le code de ce repository sera majoritairement réutilisé avec des modifications pour s'adapter à Un autre model d'écran et à une utilisation plus adapté pour nous. Ce projet était à la base un module utilisé dans un [Eurorack](https://fr.wikipedia.org/wiki/Eurorack) donc il ne serait pas adapté à notre utilisation si on ne le modifirait pas.

Le résultat final est un code beaucoup plus simple et épuré qui prend la moitié moins de place.

La librairie pour le SSD1309 était grandement similaire à celle du 1306 donc peu de changements ont été nécessaires pour adapter le code pour qu'il fonctionne.

La librairie OneButton sert à simplifier l'usage des bouton à travers plusieurs loop et la lenteur causé par les processus de l'ocilloscope.

<a id="item-9"></a>

## Comment ça marche ?

#### setConditions
Cette fonction détermine les paramètres d'affichage en fonction de l'échelle qui peut être modifié par les différents boutons (x-scale, y-scale)

#### readWave
Cette fonction viens lire les données provenant de la Pin A0 et l'enregistre dans un array. Aussi elle change des variables en fonction du x-scale.

#### dataAnalyze
Cette fonction décide des valeurs maximum et minimum du display et d'autres paramètres d'affichage.

#### startScreen
Affiche une image de marque au lancement du programme.

#### dispHold
Quand l'utilisateur appuie sur le bouton "hold" le rafraichissement de l'écran stoppe et affiche le mot "hold" en haut de l'écran jusqu'à ce que le mode soit désactivé.

#### plotData
Cette fonction prend les points enregistré dans la fonction readWave et dessine des lignes entre ces points. L'affichage est changé par les fonctions dataAnalyze et setConditions.

#### updateButtons
Cette fonction viens updater les boutons pour détecter si quelqu'un à appuyer sur le bouton.