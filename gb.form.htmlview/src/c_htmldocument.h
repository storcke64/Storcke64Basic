/***************************************************************************

  c_htmldocument.h

  gb.form.htmlview component

  (c) Benoît Minisini <g4mba5@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#ifndef __C_HTMLDOCUMENT_H
#define __C_HTMLDOCUMENT_H

#include "html.h"
#include "document.h"
#include "html_tag.h"
#include "stylesheet.h"
#include "stylesheet.h"
#include "element.h"
#include "html_tag.h"

#include "main.h"

class html_document;

typedef
	struct {
		GB_BASE ob;
		void *font;
		int ascent;
		int descent;
		int height;
		int x_height;
	}
	CHTMLDOCUMENTFONT;
	
typedef
	struct {
		GB_BASE ob;
		html_document *doc;
	}
	CHTMLDOCUMENT;

#ifndef __C_HTMLDOCUMENT_CPP

extern GB_DESC HtmlDocumentFontDesc[];
extern GB_DESC HtmlDocumentDesc[];

#else

#define THIS ((CHTMLDOCUMENT *)_object)
#define THIS_FONT ((CHTMLDOCUMENTFONT *)_object)

#endif

#endif
