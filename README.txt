Dans le dossier dazibFiles se trouvent tous les fichiers sources et headers nécessaires au fonctionnement
de l'application dazibao. Un Makefile est également présent afin de compiler l'ensemble des fichiers
en un exécutable "dazibao".
Il est nécessaire cependant pour pouvoir compiler de posséder certaines librairies gtk.
Pour cela, il faut installer le package libgtk2.0-dev :
# apt-get install libgtk2.0-dev

Une fois l'ensemble des fichiers sources compilés, on peut également faire un "make clean"
afin de supprimer tous les fichiers *.o dans le répertoire.
La première fenêtre va permettre soit de quitter, soit d'ouvrir ou de créer un fichier dazibao.
La fonction créer va générer un fichier "new_dazibao.dzb" dans le répertoire courant s'il n'existe pas déjà,
et ne va rien faire sinon.


Dans le dossier notifications, les fichiers sources client.c et serveur.c peuvent être trouvés.
Ils peuvent également être compilés grâce au Makefile présent dans le répertoire. 
L'exécutable "serveur" prend au moins un argument : 
Si le premier argument est "-f", il va considérer que le second est un fichier, qu'il tentera alors
d'ouvrir. Le fichier doit contenir une liste de path vers des dazibaos, à raison d'un path par ligne.
Si le premier argument n'est pas "-f", alors le serveur va considérer que l'ensemble des arguments
sont des chemins vers des dazibaos. Le serveur surveillera chacun des dazibaos qui lui sont attribués.
Il n'est pas possible avec un même serveur de joindre un fichier et de rajouter des dazibaos en paramètre.
Les chemin vers les dazibaos peuvent être relatifs ou absolus.
Un fichier contenant le chemin du dazibao "dazib.dzb" est disponible.

Note : Si un chemin relatif est donné (comme c'est le cas dans le fichier exemple),
    il est relatif par rapport au working directory où l'on se trouve lorsqu'on lance le
    programme, et non par rapport au répertoire du programme lui même.

L'exécutable "client" se lance sans argument, et va se connecter automatiquement au serveur s'il existe.

Dans la racine du projet se trouvent 3 autres fichiers :
dazib.dzb : Le fichier dazibao qui est rempli d'un certain nombre de message
exemple.dzb : Le fichier dazibao donné par Juliusz Chroboczek
initDazib.sh : A l'exécution, va simplement réinitialiser dazib.dzb avec le contenu de exemple.dzb
C'est particulièrement utile si l'on souhaite à revenir à un dazibao relativement rempli rapidement.
Il est donc conseillé de ne pas modifier exemple, et de plutôt s'amuser sur dazib.dzb.

Dans l'ajout d'un compound (ce n'est pas toujours instinctif la première fois) : 
La première fenêtre demande le nombre de TLV total qu'aura le compound.
Ensuite, c'est un enchainement entre une demande du type de TLV et ce qui est nécessaire pour ce TLV
(si c'est un texte, il faudra écrire un message, l'image en choisir une, etc.).


Projet réalisé par : 
    Adeline Hirsch
    Cynthia Legrand
    Chloé Dequeker (Colin)
