
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
- ESP **-WROOM-32**
- Arduino IDE
- Écran OLED (***ST7735S***)
- Bouton X 5

<a id="item-2"></a>

# Fonctionnalitées

L'ESP32 va prendre en charge plusieurs fonctionnalitées du synthétiseur

- Recevoir les notes provenant du site web à l'aide du module wifi
- Transformer et transferer les notes au Daisy seed
- Affiche sur l'écran la forme du signal audio qui sort du Daisy seed
- Intéragis avec les boutons qui modifient l'interface de l'écran

<a id="item-3"></a>

# Configuration

**Croquis du filage sans le PCB** : 
![ESP32 diagramme](/images/ESP32_config.png)

<a id="item-4"></a>

## Pinout Final

- 5V -> VCC
- GND -> GND
- 3V3 -> LED(LCD)
- 3 -> MOSI
- 2 -> SCK
- TX -> RX (Daisy seed)
- 10 -> DC
- 8 -> RST
- 7 -> LED
- 6 -> Bouton 1
- 5 -> Bouton 2
- 4 -> Bouton 3
- 18 -> Bouton 4
- 19 -> Bouton 5
- 0 -> sound input

<a id="item-5"></a>

## Pinout prototype
Ce pinout à été crée pendant les tests de l'oscilloscope avec arduino.
Le programme va utiliser ces pins jusqu'au transfert vers le ESP32.

- 5V -> VCC
- GND -> GND
- 3V3 -> LED(LCD)
- 2 -> LED
- 3 -> Bouton 1
- 4 -> Bouton 2
- 5 -> Bouton 3
- 6 -> Bouton 4
- 7 -> Bouton 5
- 8 -> RST
- 9 -> DC
- 10 -> CS
- 11 -> SDA
- 13 -> SDK
- A0 -> audio

<a id="item-6"></a>

# Code
Description du fonctionnement du code

<a id="item-7"></a>

## Librairies

- WIRE.h *
- SPI.h *
- EEPROM.h *
- Adafruit_GFX.h
- Adafruit_ST7735.h

"*" = intégré dans le arduino IDE.

Adafruit_GFX & Adafruit_ST7735 Peuvent être trouvés dans le library manager de arduino IDE.

<a id="item-8"></a>

## Base de départ du code

Le code pour l'utilisation de l'écran LCD à été adapté à partir de ce [site internet](https://www.electronics-lab.com/project/using-st7735-1-8-color-tft-display-arduino/)

Le code pour simuler un oscilloscope à été tiré d'un repository github "[Scope-O-Matic](https://github.com/josbouten/Scope-O-Matic/tree/master?tab=readme-ov-file)". Le code de ce repository sera majoritairement réutilisé avec des modifications pour s'adapter à Un autre model d'écran et à une utilisation plus adapté pour nous. Ce projet était à la base un module utilisé dans un [Eurorack](https://fr.wikipedia.org/wiki/Eurorack) donc il ne serait pas adapté à notre utilisation si on ne le modifirait pas.

<a id="item-9"></a>

## Comment ça marche ?