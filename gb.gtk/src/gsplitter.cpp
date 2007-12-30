/***************************************************************************

  gsplitter.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>
  
  Gtkmae "GTK+ made easy" classes
  
  Realizado para la Junta de Extremadura. 
  Consejería de Educación Ciencia y Tecnología. 
  Proyecto gnuLinEx
  
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

#include "widgets.h"
#include "widgets_private.h"
#include "gapplication.h"
#include "gsplitter.h"


static void slt_notify(GtkPaned *paned, GParamSpec *arg1, gSplitter *data)
{
	if (!strcmp(arg1->name,"position"))
		data->emit(SIGNAL(data->onResize));
}


GtkPaned *gSplitter::next(GtkPaned *iter)
{
	GtkWidget *child;
	
	for(;;)
	{
		if (!iter)
			iter = GTK_PANED(border);
		else
			iter = (GtkPaned*)gtk_paned_get_child2(iter);
		
		if (!iter)
			return NULL;
			
		child = gtk_paned_get_child1(iter);
		if (child && gApplication::controlItem(child)->isVisible())
			return iter;
	}	
}

gSplitter::gSplitter(gContainer *parent, bool vert) : gContainer(parent)
{
	g_typ=Type_gSplitter;
	vertical = vert;
	
	border = vertical ? gtk_vpaned_new() : gtk_hpaned_new();
	curr = GTK_PANED(border);
	widget = border;
	
	onResize = NULL;
	
	realize();
	
	g_signal_connect(G_OBJECT(curr),"notify",G_CALLBACK(slt_notify),(gpointer)this);
}

void gSplitter::insert(gControl *child)
{	
	GtkWidget *w = child->border;
	GtkWidget *tmp;
	
	lock();
	
	if (!gtk_paned_get_child1(curr))
	{
		gtk_paned_pack1(curr, w, TRUE, TRUE);
	}
	else
	{	
		if (!vertical)
			tmp=gtk_hpaned_new();
		else
			tmp=gtk_vpaned_new();
			
		gtk_widget_show_all(tmp);
		gtk_paned_pack2(curr, tmp, TRUE, TRUE);
		curr=GTK_PANED(tmp);
		gtk_paned_pack1(curr, w, TRUE, TRUE);	
		g_signal_connect(G_OBJECT(curr),"notify",G_CALLBACK(slt_notify),(gpointer)this);
	}
	
	unlock();
	
	gContainer::insert(child);
	
	emit(SIGNAL(onResize));
}

void gSplitter::remove(gControl *child)
{
	gContainer::remove(child);
}

void gSplitter::setLayout(char *vl)
{
	char **split;
	char *sval;
	int i;
	long num;
	double factor;
	GtkPaned *iter;
	
	if (!vl || !*vl) return;
	
	//fprintf(stderr, "setLayout: %s\n", vl);
	
	split=g_strsplit((const char*)vl, ",", -1);
	if (!split) return;
	 
	factor = 0;
	for (i = 0;; i++)
	{
		sval = split[i];
		if (!sval)
			break;
		errno = 0;
		num = strtol(sval, NULL, 10);
		if (errno)
			num = 1;
		factor += num;
	}
	
	if (factor == 0)
		return;
	
	factor = (vertical ? height() : width()) / factor;
	
	lock();
	
	iter = next(NULL);
	
	for (i = 0;; i++)
	{
	 	if (!iter) break;
		
		sval = split[i];
		if (!sval)
			break;
			
		errno = 0;
		num=strtol(sval,NULL,10);
		if (errno)
			num = 1;
		gtk_paned_set_position(iter, (gint)(num * factor + 0.5));
		
		iter = next(iter);
	}
	
	g_strfreev(split);
	
	unlock();
	
	emit(SIGNAL(onResize));
	
	//fprintf(stderr, "setLayout: layout = %s\n", layout());
}


char* gSplitter::layout()
{
	GtkPaned *iter;
	int vl, sum;
	GString *ret = g_string_new("");
	char *l;
	
	iter = next(NULL);
	if (iter)
	{
		sum = 0;
		for(;;)
		{
			vl = gtk_paned_get_position(iter);
			iter = next(iter);
			if (!iter)
				break;
			sum += vl;
			g_string_append_printf(ret, "%d,", vl);
		}
		
		g_string_append_printf(ret, "%d", (vertical ? height() : width()) - sum);
	}
		
	l = g_string_free(ret, false);
	gt_free_later(l);
	
	//fprintf(stderr, "layout: %s\n", l);
	
	if (!*l)
		return NULL;
	else
		return l;
}

#if 0
int gSplitter::childCount()
{
	GtkPaned *iter;
	int ret=0;
	
	if ( !gtk_paned_get_child1(GTK_PANED(border)) ) return 0;
	iter=GTK_PANED(border);
	
	while (iter)
	{
		ret++;
		iter=(GtkPaned*)gtk_paned_get_child2(iter);
	}
	
	return ret;
}

gControl* gSplitter::child(int index)
{
	GtkPaned *iter;
	int ret=0;
	GtkWidget *element=NULL;
	
	if (index<0) return NULL;
	
	if ( !gtk_paned_get_child1(GTK_PANED(border)) ) return NULL;
	iter=GTK_PANED(border);
	
	while (iter)
	{
		if (ret==index)
		{
			element=gtk_paned_get_child1(iter);
			break;
		}
		ret++;
		iter=(GtkPaned*)gtk_paned_get_child2(iter);
	}
	
	return gApplication::controlItem(element);
}
#endif

void gSplitter::performArrange()
{
	long bucle,nchd;
	gControl *chd;

	nchd=childCount();
	for (bucle=0;bucle<nchd;bucle++)
	{
		chd=child(bucle);
		//chd->resize(chd->border->allocation.width, chd->border->allocation.height);
		chd->bufW = chd->border->allocation.width;
		chd->bufH = chd->border->allocation.height;
		if (chd->isContainer())
			((gContainer*)chd)->performArrange();
	}
}

void gSplitter::resize(int w, int h)
{
	char *l;
	
	if (w == width() && h == height())
		return;
	
	//l = layout();	
	gControl::resize(w, h);
	//setLayout(l);
}


