# Image Treatment Software 🖼️

## Description
**Image Treatment Software** Cette application est un logiciel desktop développé en **C++** permettant le **traitement d’images, de vidéos et de flux webcam en temps réel** à l’aide d’une interface graphique moderne basée sur un système de **pipeline de traitements**.
L’utilisateur peut charger une image, une vidéo ou activer la webcam, puis appliquer plusieurs filtres sous forme de chaîne de traitement (pipeline).

    
## Prérequis  
Pour compiler et exécuter le projet, il vous faut :  
- Un compilateur C++ compatible (supportant C++11 ou supérieur)  
- Un environnement de développement compatible avec les fichiers `.sln` / `.vcxproj` (Visual Studio sur Windows, ou équivalent pour d’autres OS)  
- (Optionnel) Bibliothèques ou dépendances externes si utilisées — à adapter selon les traitements implémentés  



## Fonctionnalités principales 
- Chargement d’images
- Chargement de vidéos
- Capture vidéo via **Webcam**
- Application de plusieurs traitements :

  - Flou Gaussien
  - Détection de contours (Canny)
  - Niveaux de gris
  - Rotation 90°
  - Miroir horizontal
  - Seuillage
  - Négatif

    
- Système de **Pipeline de traitements**
- Prévisualisation en temps réel
- Génération de **miniatures**
- Lecture vidéo (Play / Stop)
- Sauvegarde :
  - Images traitées
  - Vidéos traitées
  - Pipelines

## Interface de l'application
L’interface est composée de :

<img width="960" height="540" alt="Interface " src="https://github.com/user-attachments/assets/3b944803-390d-4864-8c72-717c33435d03" />

- **Zone Traitements** : liste des filtres disponibles
- **Zone Pipeline** : chaîne des traitements sélectionnés
- **Zone d’affichage centrale** : affichage image / vidéo
- **Zone Miniatures** : aperçu des résultats
- **Commandes vidéo** : Play, Stop, barre de progression
- **Boutons principaux** :
  - Charger Image
  - Charger Vidéo
  - Démarrer Webcam
  - Sauvegarder Image 
  - Sauvegarder Vidéo
  - Sauvegarder Pipeline
  - Charger Pipeline
  

## Installation & compilation  

```bash
# Cloner le dépôt
git clone https://github.com/Siham-sdn/image-treatment-software.git

# Ouvrir le fichier solution dans Visual Studio
# puis compiler le projet pour générer l’exécutable (GUI.exe) 
```


Chemins

```bash

/ (root)
  ├── GUI.sln            # Solution Visual Studio
  ├── GUI.vcxproj        # Projet principal
  ├── ImagePanel.cpp / .h  
  ├── MainFrame.cpp / .h  
  ├── ThumbnailPanel.cpp / .h  
  ├── TreatmentDropTarget.cpp / .h  
  └── ...                # Autres fichiers sources
```

## Démonstration vidéo
Vous pouvez voir une démonstration de l'application sur YouTube :

[![Regarder la vidéo](https://img.youtube.com/vi/OuklhfQZ1ZQ/0.jpg)](https://www.youtube.com/watch?v=OuklhfQZ1ZQ)


