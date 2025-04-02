# Jeu du Cochon 🐷

Un jeu de dés simple et amusant où les joueurs doivent accumuler des points tout en évitant de perdre leur tour.

## Description

Le Jeu du Cochon est un jeu de dés où les joueurs tentent d'atteindre 100 points. À chaque tour, un joueur peut lancer le dé autant de fois qu'il le souhaite, en accumulant les points. Cependant, si le joueur obtient un 1, il perd tous les points du tour et c'est au tour du joueur suivant.

## Règles du jeu

- Chaque tour, un joueur lance un dé jusqu'à ce qu'il obtienne un 1 ou décide de "mettre en banque"
- Si le joueur obtient un 1, il ne marque aucun point et c'est au tour du joueur suivant
- Si le joueur obtient un autre nombre, il est ajouté à son score du tour
- Si le joueur choisit de "mettre en banque", son score du tour est ajouté à son score total
- Le premier joueur à atteindre 100 points gagne la partie

## Installation

### Prérequis
- Un compilateur C (gcc recommandé)
- Windows ou Linux/Unix

### Compilation
```bash
gcc jeuxducauchon.c -o jeuxducauchon
```

### Exécution
```bash
./jeuxducauchon [noms des joueurs]
```

## Fonctionnalités

- Support multijoueur (jusqu'à 8 joueurs)
- Mode contre l'ordinateur avec IA
- Interface en couleur pour une meilleure lisibilité
- Gestion sécurisée des entrées utilisateur
- Support multilingue (français/anglais)

## Intelligence Artificielle

L'IA du jeu utilise une stratégie sophistiquée :
- Vise un score minimum de 20 points par tour
- S'adapte si elle est proche de la victoire
- Devient plus agressive si un autre joueur est proche de gagner
- Prend des risques calculés pour maximiser ses chances de victoire

## Contrôles

- `r` : Continuer à lancer le dé
- `b` : Mettre en banque les points du tour

## Licence

Ce projet est sous licence MIT. N'hésitez pas à l'utiliser et à le modifier selon vos besoins.

## Auteur

Sonnerat Tom et Anthony Futre

## Remerciements

- Merci à tous les testeurs qui ont contribué à améliorer le jeu
- Inspiration : Jeu traditionnel du cochon
