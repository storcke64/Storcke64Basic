/***************************************************************************

  (c) 2012 Adrien Prokopowicz <prokopy@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#ifndef READER_H
#define READER_H

#include "main.h"

#define NODE_ELEMENT 1
#define NODE_TEXT 2
#define NODE_COMMENT 3
#define NODE_CDATA 4
#define NODE_ATTRIBUTE 5
#define READ_END_CUR_ELEMENT 6
#define READ_ERR_EOF 7
#define READ_ATTRIBUTE 8
#define FLAGS_COUNT 9

//Niveau de lecture des tags spéciaux (commentaires, CDATA, prologue ...)
//Commentaire <!-- -->
#define COMMENT_TAG_STARTCHAR_1 1 //Caractère !
#define COMMENT_TAG_STARTCHAR_2 2 //Caractère -
#define COMMENT_TAG_STARTCHAR_3 3 //Caractère - (2e)
#define COMMENT_TAG_ENDCHAR_1 4 //Caractère -
#define COMMENT_TAG_ENDCHAR_2 5 //Caractère - (2e)

//Prologue <? ?>
#define PROLOG_TAG_ENDCHAR 6//Caractère ?

class Document;
class Node;
class Element;
class Attribute;

class Reader
{
public :

    Reader(){InitReader();}
    ~Reader(){ClearReader();}

    Document *storedDocument;//Document stocké
    Node *curNode;//Nœud en cours de lecture
    Node *foundNode;//Noœud dont on vient de finir la lecture
    Element *curElmt;//Nœud en cours de lecture
    bool keepMemory;//Si on doit garder toute l'arborescence en mémoire
    int pos;//Position dans le flux
    bool inTag;//Si on est en train de lire un tag incomplet
    bool inAttr;//Si on est en train de lire un attribut
    bool inAttrName;//Si on est en train de lire un nom d'attribut
    bool inAttrVal;//Si on est en train de lire une valeur d'attribut
    bool inEndTag;//Si on est dans un tag de fin
    bool inNewTag;//Si on est dans un nouveau tag
    bool inTagName;//Si on est dans le nom d'un tag
    bool waitClosingElmt;//Si on attend la fermeture d'un tag (auto-fermant)
    bool inCommentTag;//Si on est dans un tag commentaire
    bool inXMLProlog;//Si on est dans un prologue xml
    unsigned char specialTagLevel;//Niveau de lecture d'un tag spécial
    bool inComment;//Si on est dans un commentaire
    int depth;//Profondeur du nœud courant
    char *attrName;//Nom de l'attribut en cours de lecture
    size_t lenAttrName;
    char *attrVal;//Valeur de l'attribut en cours de lecture
    size_t lenAttrVal;
    char *content;//Contenu du nœud texte en cours de lecture
    size_t lenContent;
    
    Node* *storedElements;
    size_t lenStoredElements;
    
    void InitReader();//Intitialise le lecteur
    int ReadChar(char car);//Lit un caractère
    void ClearReader();//Réinitialise le lecteur
    void DestroyReader();//Détruit le lecteur
    bool flags[FLAGS_COUNT];//Flags de lecture
    
    Attribute *curAttrEnum;
};

#endif
