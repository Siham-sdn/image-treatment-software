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


## Fonctionnement détaillé des fonctionnalités

### 📁 1. Chargement d'image
L'utilisateur peut importer une image via :
- Le menu « Ouvrir… »
- Le glisser-déposer directement dans la fenêtre
Le programme utilise un gestionnaire d'événements pour récupérer le chemin du fichier, charger l'image en mémoire, puis l'afficher dans le panneau principal.

### 🖼️ 2. Affichage de l'image originale
Une fois l'image chargée :
- Le fichier est décodé (format BMP, PNG, JPG, selon support)
- L'image est stockée dans un objet interne (souvent `wxImage` ou équivalent si la GUI utilise wxWidgets)
- Le `ImagePanel` se charge de l’afficher avec un redimensionnement automatique si nécessaire

### ✨ 3. Application de traitements (filtres)
Selon les modules déjà implémentés :
- Chaque traitement manipule les pixels de l’image chargée  
- L’algorithme parcourt l’image (boucles sur X et Y)
- Une nouvelle image modifiée est générée puis affichée en aperçu

Exemples de traitements possibles :
- Conversion en niveaux de gris  
- Augmentation du contraste  
- Filtre de flou  
- Détection de contours  
(Il suffit d’ajouter les fonctions dans le code source pour enrichir l’application)

### 🧩 4. Panneau de miniatures (Thumbnail Panel)
Lorsque des traitements sont appliqués :
- Une copie miniaturisée (thumbnail) est générée
- Elle est affichée dans un panneau dédié pour visualiser les différentes versions
- L'utilisateur peut cliquer sur une miniature pour la réafficher en grand

### 🎯 5. Drag & Drop (treatmentDropTarget)
L'application supporte le glisser-déposer d’images :
- Le fichier est détecté grâce à un `DropTarget` personnalisé
- Le programme vérifie si l’extension est valide
- L’image est directement chargée dans le panneau principal

### 💾 6. Sauvegarde des images
Si l’option est activée :
- L’utilisateur peut enregistrer l’image modifiée  
- L’image est exportée dans un format standard (JPG, PNG, BMP)
- Un dialogue de sauvegarde permet de choisir le nom et l’emplacement

---

## 📸 Capture d'écran

<img width="960" height="540" alt="Interface " src="https://github.com/user-attachments/assets/a0a5949d-1d64-4f46-b63c-0a2e5bbd6a60" />





