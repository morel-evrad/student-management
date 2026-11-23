# Student Management

Application console C++ de gestion des étudiants et de leurs notes. L’interface est en russe.

## Compiler sous Windows

Installer Visual Studio avec la charge de travail **Développement Desktop en C++** et le jeu d’outils MSVC v145. Ouvrir `StudentManagement.slnx`, compiler la solution et lancer l’application avec **Ctrl+F5**.

Pour compiler en ligne de commande, ouvrir **x64 Native Tools Command Prompt for VS** à la racine du dépôt.

```bat
cl /nologo /std:c++20 /utf-8 /EHsc /W4 main.cpp /Fe:StudentManagement.exe
StudentManagement.exe
```

## Utiliser le menu

1. Ajouter un étudiant avec un identifiant entier positif et unique.
2. Afficher la liste, l’effectif, les notes et les moyennes.
3. Rechercher un étudiant par identifiant.
4. Supprimer un étudiant en confirmant avec `y` ou `Y`.
5. Modifier le nom d’un étudiant.
6. Ajouter une note entre 0 et 20. Utiliser un point pour les décimales.
7. Rechercher tous les noms contenant un fragment, en respectant les majuscules et les minuscules.
0. Quitter.

Les modifications sont enregistrées dans `students.txt` dans le dossier courant. Le fichier est relu au démarrage. Si un enregistrement est incomplet, contient un identifiant en double ou une note invalide, le programme signale l’erreur et laisse le fichier intact. Corriger le fichier ou restaurer une copie valide avant de relancer l’application.

## Exécuter les tests

Après la compilation en ligne de commande, lancer depuis la racine du dépôt dans PowerShell 7.

```powershell
./tests/console.Tests.ps1 -Executable ./StudentManagement.exe
```

Les tests s’exécutent dans un dossier temporaire. Ils couvrent les saisies invalides, les recherches, la modification et la suppression, la conservation des notes au redémarrage et la protection du fichier en cas d’erreur.

## Documentation

- [Architecture](docs/architecture.md)
- [Diagramme UML](docs/diagramme.png)
- [Source PlantUML](docs/diagramme.puml)
