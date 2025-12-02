# Image Treatment Software 🖼️

## Description  
**Image Treatment Software** est une application C++/GUI qui permet de traiter des images — par exemple via un panneau d’affichage, traitement pixel par pixel, prévisualisation en miniature, etc.  
Le but du projet est de fournir un outil simple et fonctionnel pour le traitement d’images via une interface graphique desktop.  

## Fonctionnalités principales  
- Chargement d’images depuis le disque  
- Affichage de l’image originale  
- Application de traitements ou filtres (selon les modules implémentés)  
- Miniatures / prévisualisation des images traitées  
- Interface graphique conviviale (fenêtre principale, panneau image, aperçu miniature, drag & drop)  

## Prérequis  
Pour compiler et exécuter le projet, il vous faut :  
- Un compilateur C++ compatible (supportant C++11 ou supérieur)  
- Un environnement de développement compatible avec les fichiers `.sln` / `.vcxproj` (Visual Studio sur Windows, ou équivalent pour d’autres OS)  
- (Optionnel) Bibliothèques ou dépendances externes si utilisées — à adapter selon les traitements implémentés  

## Installation & compilation  

```bash
# Cloner le dépôt
git clone https://github.com/Siham-sdn/image-treatment-software.git
cd image-treatment-software

# Ouvrir le fichier solution dans Visual Studio
# puis compiler le projet pour générer l’exécutable (GUI.exe) ```



bash

/ (root)
  ├── GUI.sln            # Solution Visual Studio
  ├── GUI.vcxproj        # Projet principal
  ├── ImagePanel.cpp / .h  
  ├── MainFrame.cpp / .h  
  ├── ThumbnailPanel.cpp / .h  
  ├── TreatmentDropTarget.cpp / .h  
  └── ...                # Autres fichiers sources
