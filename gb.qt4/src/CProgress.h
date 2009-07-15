/***************************************************************************

  CProgress.h

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __CPROGRESS_H
#define __CPROGRESS_H

#include <QWindowsStyle>

#include "gambas.h"

#include "CWidget.h"

#ifndef __CPROGRESS_CPP
extern GB_DESC CProgressDesc[];
#else

#define THIS    ((CPROGRESS *)_object)
#define WIDGET  ((QProgressBar *)((CWIDGET *)_object)->widget)

#endif

typedef
  struct {
    CWIDGET widget;
    }
  CPROGRESS;

class MyWindowsStyle : public QWindowsStyle
{
    Q_OBJECT

public:
    //MyCleanlooksStyle();
    //~MyCleanlooksStyle();

protected:
    void timerEvent(QTimerEvent *event);
};

void CPROGRESS_style_hack(void *_object);

#endif
