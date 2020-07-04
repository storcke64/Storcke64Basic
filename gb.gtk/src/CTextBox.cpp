/***************************************************************************

	CTextBox.cpp

	(c) 2004-2005 - Daniel Campos Fernández <dcamposf@gmail.com>

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

#define __CTEXTBOX_CPP

#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "gambas.h"
#include "widgets.h"
#include "CTextBox.h"
#include "CWidget.h"
#include "CContainer.h"


DECLARE_EVENT(EVENT_Change);
DECLARE_EVENT(EVENT_Activate);
DECLARE_EVENT(EVENT_Click);

/*static void txt_post_change(void *_object)
{
	GB.Raise(THIS, EVENT_Change, 0);
	GB.Unref(POINTER(&_object));
}*/

static void txt_raise_change(gTextBox *sender)
{
	CWIDGET *_object = GetObject((gControl*)sender);
	GB.Raise(THIS, EVENT_Change, 0);
}

static void txt_raise_activate(gTextBox *sender)
{
	CWIDGET *_object = GetObject((gControl*)sender);
	GB.Raise(THIS, EVENT_Activate, 0);
}

/***************************************************************************

	TextBox

***************************************************************************/

#define CHECK_COMBOBOX() \
	if (!TEXTBOX->hasEntry()) \
	{ \
		GB.Error("ComboBox is read-only"); \
		return; \
	}

BEGIN_METHOD(TextBox_new, GB_OBJECT parent)

	InitControl(new gTextBox(CONTAINER(VARG(parent))), (CWIDGET*)THIS);
	TEXTBOX->onChange = txt_raise_change;
	TEXTBOX->onActivate = txt_raise_activate;
	
END_METHOD


BEGIN_METHOD_VOID(TextBox_Clear)

	TEXTBOX->clear();

END_METHOD


BEGIN_METHOD(TextBox_Insert, GB_STRING text)

	CHECK_COMBOBOX();
	TEXTBOX->insert(STRING(text),LENGTH(text));

END_METHOD


BEGIN_PROPERTY(TextBox_Text)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TEXTBOX->text());
	else
		TEXTBOX->setText(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_PROPERTY(TextBox_Placeholder)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TEXTBOX->placeholder());
	else
		TEXTBOX->setPlaceholder(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_PROPERTY(TextBox_Length)

	GB.ReturnInteger(TEXTBOX->length());

END_PROPERTY


BEGIN_PROPERTY(TextBox_Alignment)

	if (READ_PROPERTY) { GB.ReturnInteger(TEXTBOX->alignment()); return; }
	TEXTBOX->setAlignment(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(TextBox_Pos)

	CHECK_COMBOBOX();
	if (READ_PROPERTY) { GB.ReturnInteger(TEXTBOX->position()); return; }
	TEXTBOX->setPosition(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(TextBox_ReadOnly)

	if (READ_PROPERTY) { GB.ReturnBoolean(TEXTBOX->isReadOnly()); return; }
	TEXTBOX->setReadOnly(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(TextBox_Border)

	if (READ_PROPERTY) { GB.ReturnBoolean(TEXTBOX->hasBorder()); return; }
	TEXTBOX->setBorder(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(TextBox_Password)

	CHECK_COMBOBOX();
	if (READ_PROPERTY) { GB.ReturnBoolean(TEXTBOX->password()); return; }
	TEXTBOX->setPassword(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(TextBox_MaxLength)

	CHECK_COMBOBOX();
	if (READ_PROPERTY) { GB.ReturnInteger(TEXTBOX->maxLength()); return; }
	TEXTBOX->setMaxLength(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_METHOD_VOID(TextBox_Selected)

	CHECK_COMBOBOX();
	GB.ReturnBoolean(TEXTBOX->isSelected());

END_METHOD


BEGIN_METHOD(TextBox_CursorAt, GB_INTEGER pos)

	int x, y;
	
	CHECK_COMBOBOX();
	TEXTBOX->getCursorPos(&x, &y, VARGOPT(pos, -1));
	GB.ReturnObject(GEOM.CreatePoint(x, y));
	
END_PROPERTY

/***************************************************************************

	.TextBox.Selection

***************************************************************************/

BEGIN_PROPERTY(TextBox_Selection_Text)

	char *buf;
	
	CHECK_COMBOBOX();
	
	if (READ_PROPERTY)
	{
		buf=TEXTBOX->selText();
		GB.ReturnNewZeroString(buf);
		g_free(buf);
		return;
	}
	
	buf=GB.ToZeroString(PROP(GB_STRING));
	TEXTBOX->setSelText(buf,strlen(buf));

END_PROPERTY


BEGIN_PROPERTY(TextBox_Selection_Length)

	CHECK_COMBOBOX();
	GB.ReturnInteger(TEXTBOX->selLength());

END_PROPERTY


BEGIN_PROPERTY(TextBox_Selection_Start)

	CHECK_COMBOBOX();
	GB.ReturnInteger(TEXTBOX->selStart());

END_PROPERTY


BEGIN_METHOD_VOID(TextBox_Unselect)

	CHECK_COMBOBOX();
	TEXTBOX->selClear();

END_METHOD

BEGIN_METHOD_VOID(TextBox_SelectAll)

	CHECK_COMBOBOX();
	TEXTBOX->selectAll();

END_METHOD

BEGIN_METHOD(TextBox_Select, GB_INTEGER start; GB_INTEGER length)

	CHECK_COMBOBOX();
	TEXTBOX->select(VARG(start),VARG(length));

END_METHOD



/***************************************************************************

	ComboBox

***************************************************************************/

#undef THIS
#define THIS ((CCOMBOBOX *)_object)

static void cmb_raise_click(gComboBox *sender)
{
	CWIDGET *_object = GetObject((gControl*)sender);
	if (THIS->click)
		return;
	THIS->click = true;
	GB.Raise(THIS, EVENT_Click, 0);
	THIS->click = false;
}


BEGIN_METHOD(ComboBox_new, GB_OBJECT parent)

	InitControl(new gComboBox(CONTAINER(VARG(parent))), (CWIDGET*)THIS);
	
	COMBOBOX->onClick = cmb_raise_click;
	COMBOBOX->onChange = txt_raise_change;
	COMBOBOX->onActivate = txt_raise_activate;

END_METHOD


BEGIN_PROPERTY(ComboBox_Text)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(COMBOBOX->text());
	else
		COMBOBOX->setText(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_METHOD_VOID(ComboBox_Popup)

	COMBOBOX->popup();

END_METHOD


BEGIN_METHOD(ComboBox_get, GB_INTEGER index)

	int index = VARG(index);

	if (index < 0 || index >= COMBOBOX->count())
	{
		GB.Error("Bad index");
		return;
	}

	THIS->index = index;
	RETURN_SELF();

END_METHOD


BEGIN_PROPERTY(ComboBox_Item_Text)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(COMBOBOX->itemText(THIS->index));
	else	
		COMBOBOX->setItemText(THIS->index,GB.ToZeroString(PROP(GB_STRING)));


END_PROPERTY


BEGIN_METHOD(ComboBox_Add, GB_STRING item; GB_INTEGER pos)

	int Pos;
	char *Item=GB.ToZeroString(ARG(item));

	if (MISSING(pos)) Pos=COMBOBOX->count();
	else Pos=VARG(pos);
	
	COMBOBOX->add(Item,Pos);

END_METHOD


BEGIN_METHOD(ComboBox_Remove, GB_INTEGER pos)

	COMBOBOX->remove(VARG(pos));

END_METHOD


BEGIN_PROPERTY(ComboBox_Sorted)

	if (READ_PROPERTY) { GB.ReturnBoolean(COMBOBOX->isSorted()); return; }
	COMBOBOX->setSorted(VPROP(GB_BOOLEAN));

END_METHOD


BEGIN_PROPERTY(ComboBox_Count)

	GB.ReturnInteger(COMBOBOX->count());

END_PROPERTY


BEGIN_PROPERTY(ComboBox_Index)

	if (READ_PROPERTY) { GB.ReturnInteger(COMBOBOX->index()); return; }
	COMBOBOX->setIndex(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(ComboBox_Current)

	if (!COMBOBOX->count()) { GB.ReturnNull(); return; }
	THIS->index = COMBOBOX->index();
	RETURN_SELF();

END_PROPERTY

BEGIN_METHOD(ComboBox_Find, GB_STRING item)

	GB.ReturnInteger(COMBOBOX->find(GB.ToZeroString(ARG(item))));

END_METHOD


BEGIN_PROPERTY(ComboBox_List)

	GB_ARRAY array;
	int i;
	
	if (READ_PROPERTY)
	{
		GB.Array.New(&array, GB_T_STRING, COMBOBOX->count());
		for (i = 0; i < COMBOBOX->count(); i++)
		{
			*((char **)GB.Array.Get(array, i)) = GB.NewZeroString(COMBOBOX->itemText(i));
		}
		
		GB.ReturnObject(array);
	}
	else
	{
		char *text = GB.NewZeroString(COMBOBOX->text());
		
		array = VPROP(GB_OBJECT);
		COMBOBOX->lock();
		COMBOBOX->clear();
		if (array)
		{
			for (i = 0; i < GB.Array.Count(array); i++)
				COMBOBOX->add(*((char **)GB.Array.Get(array, i)));
		}
		COMBOBOX->setText(text);
		COMBOBOX->unlock();

		GB.FreeString(&text);
		
		if (COMBOBOX->isReadOnly())
		{
			if (COMBOBOX->index() < 0 && COMBOBOX->count() > 0)
				COMBOBOX->setIndex(0);
		}
	}

END_PROPERTY

BEGIN_PROPERTY(ComboBox_Border)

	if (READ_PROPERTY)
		GB.ReturnBoolean(COMBOBOX->hasBorder());
	else
		COMBOBOX->setBorder(VPROP(GB_BOOLEAN));

END_PROPERTY

/***************************************************************************

	Descriptions

***************************************************************************/

GB_DESC CTextBoxSelectionDesc[] =
{
	GB_DECLARE(".TextBox.Selection", 0), GB_VIRTUAL_CLASS(),

	GB_PROPERTY("Text", "s", TextBox_Selection_Text),
	GB_PROPERTY_READ("Length", "i", TextBox_Selection_Length),
	GB_PROPERTY_READ("Start", "i", TextBox_Selection_Start),
	GB_PROPERTY_READ("Pos", "i", TextBox_Selection_Start),

	GB_METHOD("Hide", 0, TextBox_Unselect, 0),

	GB_END_DECLARE
};

GB_DESC CTextBoxDesc[] =
{
	GB_DECLARE("TextBox", sizeof(CTEXTBOX)), GB_INHERITS("Control"),

	GB_METHOD("_new", 0, TextBox_new, "(Parent)Container;"),

	GB_PROPERTY("Text", "s", TextBox_Text),
	GB_PROPERTY("Alignment", "i", TextBox_Alignment),
	GB_PROPERTY_READ("Length", "i", TextBox_Length),
	GB_PROPERTY("Pos", "i", TextBox_Pos),
	GB_PROPERTY("ReadOnly", "b", TextBox_ReadOnly),
	GB_PROPERTY("Border", "b", TextBox_Border),
	GB_PROPERTY("Password", "b", TextBox_Password),
	GB_PROPERTY("MaxLength", "i", TextBox_MaxLength),
	GB_PROPERTY("Placeholder", "s", TextBox_Placeholder),

	GB_PROPERTY_SELF("Selection", ".TextBox.Selection"),
	GB_METHOD("Select", 0, TextBox_Select, "[(Start)i(Length)i]"),
	GB_METHOD("SelectAll", 0, TextBox_SelectAll, 0),
	GB_METHOD("Unselect", 0, TextBox_Unselect, 0),
	GB_PROPERTY_READ("Selected", "b", TextBox_Selected),

	GB_METHOD("Clear", 0, TextBox_Clear, 0),
	GB_METHOD("Insert", 0, TextBox_Insert, "(Text)s"),

	GB_METHOD("CursorAt", "Point", TextBox_CursorAt, "[(Pos)i]"),

	GB_EVENT("Change", 0, 0, &EVENT_Change),
	GB_EVENT("Activate", 0, 0, &EVENT_Activate),
	
	TEXTBOX_DESCRIPTION,

	GB_END_DECLARE
};



GB_DESC CComboBoxItemDesc[] =
{
	GB_DECLARE(".ComboBox.Item", 0), GB_VIRTUAL_CLASS(),

	GB_PROPERTY("Text", "s", ComboBox_Item_Text),

	GB_END_DECLARE
};


GB_DESC CComboBoxDesc[] =
{
	GB_DECLARE("ComboBox", sizeof(CCOMBOBOX)), GB_INHERITS("Control"),

	GB_METHOD("_new", 0, ComboBox_new, "(Parent)Container;"),
	GB_METHOD("_get", ".ComboBox.Item", ComboBox_get, "(Index)i"),
	GB_METHOD("Popup", 0, ComboBox_Popup, 0),
	GB_METHOD("Clear", 0, TextBox_Clear, 0),
	GB_METHOD("Insert", 0, TextBox_Insert, "(Text)s"),
	GB_METHOD("Add", 0, ComboBox_Add, "(Item)s[(Index)i]"),
	GB_METHOD("Remove", 0, ComboBox_Remove, "(Index)i"),
	GB_METHOD("Find", "i", ComboBox_Find, "(Item)s"),

	GB_PROPERTY("Text", "s", ComboBox_Text),
	GB_PROPERTY_READ("Length", "i", TextBox_Length),
	GB_PROPERTY("Pos", "i", TextBox_Pos),
	GB_PROPERTY("ReadOnly", "b", TextBox_ReadOnly),
	GB_PROPERTY("Password", "b", TextBox_Password),
	GB_PROPERTY("MaxLength", "i", TextBox_MaxLength),
	GB_PROPERTY("Border", "b", ComboBox_Border),
	GB_PROPERTY("Placeholder", "s", TextBox_Placeholder),
	
	GB_PROPERTY_SELF("Selection", ".TextBox.Selection"),
	GB_METHOD("Select", 0, TextBox_Select, "[(Start)i(Length)i]"),
	GB_METHOD("SelectAll", 0, TextBox_SelectAll, 0),
	GB_METHOD("Unselect", 0, TextBox_Unselect, 0),
	GB_PROPERTY_READ("Selected", "b", TextBox_Selected),

	GB_PROPERTY("Sorted", "b", ComboBox_Sorted),
	GB_PROPERTY("List", "String[]", ComboBox_List),
	GB_PROPERTY_READ("Count", "i", ComboBox_Count),
	GB_PROPERTY_READ("Current", ".ComboBox.Item", ComboBox_Current),
	GB_PROPERTY("Index", "i", ComboBox_Index),

	GB_METHOD("CursorAt", "Point", TextBox_CursorAt, "[(Pos)i]"),
	
	GB_EVENT("Change", 0, 0, &EVENT_Change),
	GB_EVENT("Activate", 0, 0, &EVENT_Activate),
	GB_EVENT("Click", 0, 0, &EVENT_Click),

	COMBOBOX_DESCRIPTION,

	GB_END_DECLARE
};


