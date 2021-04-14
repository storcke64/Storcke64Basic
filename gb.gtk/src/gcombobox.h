/***************************************************************************

  gcombobox.h

  (c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

#ifndef __GCOMBOBOX_H
#define __GCOMBOBOX_H

#include "gtextbox.h"
#include "gtree.h"

class gComboBox : public gTextBox
{
public:
	gComboBox(gContainer *parent);
	~gComboBox();

	int count() const;
	int index();
	char* itemText(int ind);
	virtual int length();
	//char** list();
	virtual bool isReadOnly() const;
	bool isSorted() const;
	virtual char *text();

	void setIndex(int vl);
	void setItemText(int ind, const char *txt);
	//void setList(char **vl);
	virtual void setReadOnly(bool vl);
	void setSorted(bool vl);
	virtual void setText(const char *vl);
	
	void setBorder(bool v);

//"Methods"
	void popup();
	void add(const char *vl, int pos = -1);
	virtual void clear();
	int find(const char *ptr);
	void remove(int pos);
	
#ifdef GTK3
	virtual GtkWidget *getStyleSheetWidget();
	virtual const char *getStyleSheetColorNode();
	virtual const char *getStyleSheetFontNode();
	virtual void customStyleSheet(GString *css);
	virtual void setForeground(gColor color);
#else
	virtual void setRealBackground(gColor vl);
	virtual void setRealForeground(gColor vl);
#endif
	virtual void setFocus();
	virtual bool canFocus() const;
	void updateBorder();
	virtual void setDesign(bool ignore = false);
	
//"Signals"
	void (*onClick)(gComboBox *sender);

//"Private"
	GtkCellRenderer *cell;
	virtual int minimumHeight();
	gTree *tree;
	bool _model_dirty;
	int _last_key;
	GtkWidget *_button;
	int _model_dirty_timeout;
	unsigned _sort : 1;
	
	virtual void updateFont();
	void updateModel();
	void updateSort();
	char *indexToKey(int index);
	char* find(GtkTreePath *path) { return tree->pathToKey(path, false); }
	void checkIndex();
	void updateFocusHandler();
	void create(bool readOnly);
};

#endif
