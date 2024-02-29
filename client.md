# Documentation de Music Grid

Bienvenue dans la documentation du client web de Music Maker ! Ce document fournit un aperçu des fonctionnalités, des instructions d'installation et des directives d'utilisation pour le client web du projet Music Maker.

Ceci est une application web de grille musicale simple dans laquelle vous pouvez cliquer sur les cellules pour créer de la musique. L'idée est inspirée d'un des composants de [AntiStress](https://play.google.com/store/apps/details?id=com.JindoBlu.Antistress&hl=fr)

Démo : https://music-grid.surge.sh

## Table des matières

- [Installation](#installation)
- [Utilisation](#Fonctionnalités)
- [Fonctionnalités](#fonctionnalités)
- [Licence](#licence)

## Installation

Pour utiliser le client web de Music Maker, suivez ces étapes :

1. Clonez le dépôt : `git clone https://github.com/your-username/music-maker-webclient.git`
2. Installez les dépendances : `npm install`
3. Lancez le serveur de développement : `npm run dev`

## Comment exécuter

Assurez-vous d'avoir nodejs et npm installés.

Installez les dépendances...

```bash
cd music-grid
npm install
```

...puis lancez [Rollup](https://rollupjs.org):

```bash
npm run
```

Rendez-vous sur [localhost:5000](http://localhost:5000). Vous devriez voir l'application en cours d'exécution.

## Fonctionnalités

Le client web de Music Maker comprend les fonctionnalités suivantes :

- Fonctionnalité 1 : Envoi de notes de musique en temps réel via wifi
- Fonctionnalité 3 : Enregistrement multi-piste
- Fonctionnalité 4 : Partage et téléchargement de pistes

## Construction et exécution en mode production

Pour créer une version optimisée de l'application :

```bash
npm run build
```

Vous pouvez exécuter la nouvelle application construite avec `npm run start`. Cela utilise [sirv](https://github.com/lukeed/sirv), qui est inclus dans les `dependencies` de votre package.json pour que l'application fonctionne lorsque vous la déployez sur des plates-formes comme [Heroku](https://heroku.com).

## Mode application monopage

Par défaut, sirv répond uniquement aux demandes correspondant aux fichiers dans `public`. Cela maximise la compatibilité avec les serveurs de fichiers statiques, vous permettant de déployer votre application n'importe où.

Si vous construisez une application monopage (SPA) avec plusieurs routes, sirv doit pouvoir répondre aux demandes pour *n'importe* quel chemin. Vous pouvez le faire en modifiant la commande `"start"` dans package.json :

```js
"start": "sirv public --single"
```

## Déploiement sur le web

### Avec [now](https://zeit.co/now)

Installez `now` si ce n'est pas déjà fait :

```bash
npm install -g now
```

Ensuite, depuis le dossier de votre projet :

```bash
cd public
now deploy --name my-project
```

En alternative, utilisez le [client desktop de Now](https://zeit.co/download) et faites simplement glisser le dossier du projet décompressé vers l'icône de la barre des tâches.

### Avec [surge](https://surge.sh/)

Installez `surge` si ce n'est pas déjà fait :

```bash
npm install -g surge
```

Ensuite, depuis le dossier de votre projet :

```bash
npm run build
surge public my-project.surge.sh
```

## Licence

Le client web de Music Maker est sous licence [MIT License](/music-grid/LICENSE).
