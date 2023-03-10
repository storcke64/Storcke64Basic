/***************************************************************************

  cprinter.cpp

  (c) 2000-2017 Benoît Minisini <benoit.minisini@gambas-basic.org>

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

#define __CPRINTER_CPP

#include <QEventLoop>
#include <QPrintDialog>
#include <QPrinterInfo>

#include "gb.form.print.h"
#include "gb.form.properties.h"
#include "cpaint_impl.h"
#include "cprinter.h"

DECLARE_EVENT(EVENT_Begin);
DECLARE_EVENT(EVENT_End);
DECLARE_EVENT(EVENT_Paginate);
DECLARE_EVENT(EVENT_Draw);

#if QT5

#define COPY_COUNT copyCount
#define SET_COPY_COUNT(_count) setCopyCount(_count)
#define ORIENTATION pageLayout().orientation
#define SET_ORIENTATION setPageOrientation
#define PAGE_FORMAT() pageLayout().pageSize().id()
#define SET_PAGE_FORMAT(_format) setPageSize(QPageSize(_format))
#define PAGE_SIZE(_unit) pageLayout().fullRect((QPageLayout::Unit)_unit).size()
#define SET_PAGE_SIZE(_size, _unit) setPageSize(QPageSize(_size, _unit))

#define PRINTER_ORIENTATION QPageLayout::Orientation
#define PRINTER_PORTRAIT QPageLayout::Portrait
#define PRINTER_LANDSCAPE QPageLayout::Landscape
#define PRINTER_PAGE_FORMAT QPageSize::PageSizeId

#define PAGE_FORMAT_ID(_format) QPageSize::_format
#define PAGE_UNIT(_unit) QPageSize::_unit

#else

#define COPY_COUNT numCopies
#define SET_COPY_COUNT(_count) setNumCopies(_count)
#define ORIENTATION orientation
#define SET_ORIENTATION setOrientation
#define PAGE_FORMAT paperSize
#define SET_PAGE_FORMAT(_format) setPaperSize(_format)
#define PAGE_SIZE(_unit) paperSize(_unit)
#define SET_PAGE_SIZE(_size, _unit) setPaperSize(_size, _unit)

#define PRINTER_ORIENTATION QPrinter::Orientation
#define PRINTER_PORTRAIT QPrinter::Portrait
#define PRINTER_LANDSCAPE QPrinter::Landscape
#define PRINTER_PAGE_FORMAT QPrinter::PaperSize

#define PAGE_FORMAT_ID(_format) QPrinter::_format
#define PAGE_UNIT(_unit) QPrinter::_unit

#endif

QSizeF CPRINTER_get_page_size(CPRINTER *_object)
{
	return PRINTER->PAGE_SIZE(PAGE_UNIT(Millimeter));
}

static bool configure_printer(CPRINTER *_object)
{
	QPrinter *printer = THIS->printer;
	
	QPrintDialog dialog(printer, qApp->activeWindow());
	return (dialog.exec() != QDialog::Accepted);
}

static bool run_printer(CPRINTER *_object)
{
	QEventLoop loop;
	bool ret = true;
	int page;
	int firstPage, lastPage;
	bool reverse;
	int num_copies_out, num_copies_in, repeat_out, repeat_in;
	
	THIS->cancel = false;
	
	THIS->printing = true;
	PAINT_begin(THIS);
	
	GB.Raise(THIS, EVENT_Begin, 0);
	
	if (GB.CanRaise(THIS, EVENT_Paginate))
	{
		while (!THIS->cancel && !THIS->page_count_set)
		{
			GB.Raise(THIS, EVENT_Paginate, 0);
			loop.processEvents();
		}
	}
	
	if (THIS->cancel)
		goto __CANCEL;
	
	if (PRINTER->fromPage() == 0)
	{
		firstPage = 1;
		lastPage = THIS->page_count;
	}
	else if (PRINTER->toPage() == 0)
	{
		firstPage = PRINTER->fromPage();
		lastPage = THIS->page_count;
	}
	else
	{
		firstPage = PRINTER->fromPage();
		lastPage = PRINTER->toPage();
	}
	
	if (firstPage > THIS->page_count)
		goto __CANCEL;
	if (lastPage > THIS->page_count)
		lastPage = THIS->page_count;
	
	reverse = PRINTER->pageOrder() == QPrinter::LastPageFirst;
	if (PRINTER->collateCopies())
	{
		num_copies_out = PRINTER->COPY_COUNT(); // Always return 1 if the driver manages it
		num_copies_in = 1;
	}
	else
	{
		num_copies_out = 1;
		num_copies_in = PRINTER->COPY_COUNT(); // Always return 1 if the driver manages it
	}
	
	for(repeat_out = 0; repeat_out < num_copies_out; repeat_out++)
	{
		for (page = firstPage; page <= lastPage; page++)
		{
			for(repeat_in = 0; repeat_in < num_copies_in; repeat_in++)
			{
				loop.processEvents();
				if (THIS->cancel)
					goto __CANCEL;
				
				if (reverse)
					THIS->page = firstPage + lastPage - page;
				else
					THIS->page = page;
				
				GB.Raise(THIS, EVENT_Draw, 0);
				
				if (page != lastPage)
					PRINTER->newPage();
			}
		}
	}

__CANCEL:

	GB.Raise(THIS, EVENT_End, 0);

	PAINT_end();
	
	THIS->page_count_set = false;

	THIS->printing = false;
	
	return ret;
}

static void update_duplex(CPRINTER *_object)
{
	QPrinter::DuplexMode duplex;

	switch(THIS->duplex)
	{
		case GB_PRINT_DUPLEX_HORIZONTAL:
			duplex = PRINTER->ORIENTATION() == PRINTER_PORTRAIT ? QPrinter::DuplexShortSide : QPrinter::DuplexLongSide;
			break;
		case GB_PRINT_DUPLEX_VERTICAL:
			duplex = PRINTER->ORIENTATION() == PRINTER_PORTRAIT ? QPrinter::DuplexLongSide : QPrinter::DuplexShortSide;
			break;
		case GB_PRINT_SIMPLEX:
		default:
			duplex = QPrinter::DuplexNone;
	}

	PRINTER->setDuplex(duplex);
}

BEGIN_METHOD_VOID(Printer_new)

	if (MAIN_CHECK_INIT())
		return;

	THIS->printer = new QPrinter(QPrinter::HighResolution);
	THIS->page_count = 1;

END_METHOD

BEGIN_METHOD_VOID(Printer_free)

	delete THIS->printer;

END_METHOD

BEGIN_METHOD_VOID(Printer_Print)

	GB.ReturnBoolean(run_printer(THIS));

END_METHOD

BEGIN_METHOD_VOID(Printer_Configure)

	GB.ReturnBoolean(configure_printer(THIS));

END_METHOD

BEGIN_METHOD_VOID(Printer_Cancel)

	THIS->cancel = true;
	PRINTER->abort();

END_METHOD

BEGIN_PROPERTY(Printer_Count)

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->page_count);
	else
	{
		THIS->page_count = VPROP(GB_INTEGER);
		THIS->page_count_set = true;
	}

END_PROPERTY

BEGIN_PROPERTY(Printer_Page)

	GB.ReturnInteger(THIS->page);

END_PROPERTY

BEGIN_METHOD(Printer_SetPage, GB_INTEGER page)

	THIS->page = VARG(page);

END_METHOD

BEGIN_METHOD_VOID(Printer_IsCountSet)

	GB.ReturnBoolean(THIS->page_count_set);
	THIS->page_count_set = FALSE;

END_METHOD

BEGIN_PROPERTY(Printer_Name)

	if (READ_PROPERTY)
		RETURN_NEW_STRING(PRINTER->printerName());
	else
		PRINTER->setPrinterName(QSTRING_PROP());

END_PROPERTY

BEGIN_PROPERTY(Printer_Orientation)

	if (READ_PROPERTY)
	{
		switch(PRINTER->ORIENTATION())
		{
			case PRINTER_LANDSCAPE: GB.ReturnInteger(GB_PRINT_LANDSCAPE); break;
			case PRINTER_PORTRAIT: default: GB.ReturnInteger(GB_PRINT_PORTRAIT);
		}
	}
	else
	{
		PRINTER_ORIENTATION orient;
		
		switch (VPROP(GB_INTEGER))
		{
			case GB_PRINT_LANDSCAPE: orient = PRINTER_LANDSCAPE; break;
			case GB_PRINT_PORTRAIT: default: orient = PRINTER_PORTRAIT; break;
		}
		
		PRINTER->SET_ORIENTATION(orient);
		update_duplex(THIS);
	}

END_PROPERTY

BEGIN_PROPERTY(Printer_Paper)

	if (READ_PROPERTY)
	{
		int val;
		
		switch(PRINTER->PAGE_FORMAT())
		{
			case PAGE_FORMAT_ID(A3): val = GB_PRINT_A3; break;
			case PAGE_FORMAT_ID(A4): val = GB_PRINT_A4; break;
			case PAGE_FORMAT_ID(A5): val = GB_PRINT_A5; break;
			case PAGE_FORMAT_ID(B5): val = GB_PRINT_B5; break;
			case PAGE_FORMAT_ID(Letter): val = GB_PRINT_LETTER; break;
			case PAGE_FORMAT_ID(Executive): val = GB_PRINT_EXECUTIVE; break;
			case PAGE_FORMAT_ID(Legal): val = GB_PRINT_LEGAL; break;
			default: val = GB_PRINT_CUSTOM;
		}
		
		GB.ReturnInteger(val);
	}
	else
	{
		PRINTER_PAGE_FORMAT paper;
		
		switch(VPROP(GB_INTEGER))
		{
			case GB_PRINT_A3: paper = PAGE_FORMAT_ID(A3); break;
			case GB_PRINT_A5: paper = PAGE_FORMAT_ID(A5); break;
			case GB_PRINT_B5: paper = PAGE_FORMAT_ID(B5); break;
			case GB_PRINT_LETTER: paper = PAGE_FORMAT_ID(Letter); break;
			case GB_PRINT_EXECUTIVE: paper = PAGE_FORMAT_ID(Executive); break;
			case GB_PRINT_LEGAL: paper = PAGE_FORMAT_ID(Legal); break;
			case GB_PRINT_A4: default: paper = PAGE_FORMAT_ID(A4);
		}
		
		PRINTER->SET_PAGE_FORMAT(paper);

		fprintf(stderr, "format = %d -> %d\n", VPROP(GB_INTEGER), (int)PRINTER->PAGE_FORMAT());
	}

END_PROPERTY

BEGIN_PROPERTY(Printer_PaperWidth)

	QSizeF size = CPRINTER_get_page_size(THIS);

	if (READ_PROPERTY)
		GB.ReturnFloat(floor((double)size.width() * 1E6) / 1E6);
	else
	{
		qreal width = (qreal)VPROP(GB_FLOAT);
		if (width != size.width())
		{
			size.setWidth(width);
			PRINTER->SET_PAGE_SIZE(size, PAGE_UNIT(Millimeter));
		}
	}

END_PROPERTY

BEGIN_PROPERTY(Printer_PaperHeight)

	QSizeF size = CPRINTER_get_page_size(THIS);
	
	if (READ_PROPERTY)
		GB.ReturnFloat(floor((double)size.height() * 1E6) / 1E6);
	else
	{
		qreal height = (qreal)VPROP(GB_FLOAT);
		if (height != size.height())
		{
			size.setHeight(height);
			PRINTER->SET_PAGE_SIZE(size, PAGE_UNIT(Millimeter));
		}
	}

END_PROPERTY

BEGIN_PROPERTY(Printer_CollateCopies)

	if (READ_PROPERTY)
		GB.ReturnBoolean(PRINTER->collateCopies());
	else
		PRINTER->setCollateCopies(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(Printer_ReverseOrder)

	if (READ_PROPERTY)
		GB.ReturnBoolean(PRINTER->pageOrder() == QPrinter::LastPageFirst);
	else
		PRINTER->setPageOrder(VPROP(GB_BOOLEAN) ? QPrinter::LastPageFirst : QPrinter::FirstPageFirst);

END_PROPERTY

BEGIN_PROPERTY(Printer_Duplex)

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->duplex);
	else
	{
		THIS->duplex = VPROP(GB_INTEGER);
		update_duplex(THIS);
	}

END_PROPERTY

BEGIN_PROPERTY(Printer_GrayScale)

	if (READ_PROPERTY)
		GB.ReturnBoolean(PRINTER->colorMode() == QPrinter::GrayScale);
	else
		PRINTER->setColorMode(VPROP(GB_BOOLEAN) ? QPrinter::GrayScale : QPrinter::Color);

END_PROPERTY

BEGIN_PROPERTY(Printer_NumCopies)

#if QT_VERSION >= QT_VERSION_CHECK(4, 7, 0)
	if (!PRINTER->supportsMultipleCopies())
		return;
#endif

	if (READ_PROPERTY)
		GB.ReturnInteger(PRINTER->COPY_COUNT());
	else
		PRINTER->SET_COPY_COUNT(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(Printer_Resolution)

	if (READ_PROPERTY)
		GB.ReturnInteger(PRINTER->resolution());
	else
		PRINTER->setResolution(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(Printer_FirstPage)

	if (READ_PROPERTY)
		GB.ReturnInteger(PRINTER->fromPage());
	else
		PRINTER->setFromTo(VPROP(GB_INTEGER), PRINTER->toPage());

END_PROPERTY

BEGIN_PROPERTY(Printer_LastPage)

	if (READ_PROPERTY)
		GB.ReturnInteger(PRINTER->toPage());
	else
		PRINTER->setFromTo(PRINTER->fromPage(), VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(Printer_FullPage)

	if (READ_PROPERTY)
		GB.ReturnBoolean(PRINTER->fullPage());
	else
		PRINTER->setFullPage(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(Printer_OutputFile)

	if (READ_PROPERTY)
		RETURN_NEW_STRING(PRINTER->outputFileName());
	else
		PRINTER->setOutputFileName(TO_QSTRING(GB.FileName(PSTRING(), PLENGTH())));

END_PROPERTY

BEGIN_PROPERTY(Printer_Default)

	QPrinterInfo info = QPrinterInfo::defaultPrinter();

	if (info.isNull())
		GB.ReturnNull();
	else
		RETURN_NEW_STRING(info.printerName());

END_PROPERTY

BEGIN_PROPERTY(Printer_List)

	GB_ARRAY array;
	QList<QPrinterInfo> list = QPrinterInfo::availablePrinters();

	GB.Array.New(&array, GB_T_STRING, list.length());
	for (int i = 0; i < list.length(); i++)
		*((char **)GB.Array.Get(array, i)) = NEW_STRING(list.at(i).printerName());

	GB.ReturnObject(array);

END_PROPERTY

GB_DESC PrinterDesc[] =
{
  GB_DECLARE("Printer", sizeof(CPRINTER)),

  GB_STATIC_PROPERTY_READ("Default", "s", Printer_Default),
  GB_STATIC_PROPERTY_READ("List", "String[]", Printer_List),

	GB_CONSTANT("Portrait", "i", GB_PRINT_PORTRAIT),
	GB_CONSTANT("Landscape", "i", GB_PRINT_LANDSCAPE),
	//GB_CONSTANT("ReversePortrait", "i", GB_PRINT_REVERSE_PORTRAIT),
	//GB_CONSTANT("ReverseLandscape", "i", GB_PRINT_REVERSE_LANDSCAPE),

	GB_CONSTANT("Custom", "i", GB_PRINT_CUSTOM),
	GB_CONSTANT("A3", "i", GB_PRINT_A3),
	GB_CONSTANT("A4", "i", GB_PRINT_A4),
	GB_CONSTANT("A5", "i", GB_PRINT_A5),
	GB_CONSTANT("B5", "i", GB_PRINT_B5),
	GB_CONSTANT("Letter", "i", GB_PRINT_LETTER),
	GB_CONSTANT("Executive", "i", GB_PRINT_EXECUTIVE),
	GB_CONSTANT("Legal", "i", GB_PRINT_LEGAL),

	GB_CONSTANT("Simplex", "i", GB_PRINT_SIMPLEX),
	GB_CONSTANT("Horizontal", "i", GB_PRINT_DUPLEX_HORIZONTAL),
	GB_CONSTANT("Vertical", "i", GB_PRINT_DUPLEX_VERTICAL),

	GB_METHOD("_new", NULL, Printer_new, NULL),
	GB_METHOD("_free", NULL, Printer_free, NULL),
	
	GB_METHOD("Print", "b", Printer_Print, NULL),
	GB_METHOD("_SetPage", NULL, Printer_SetPage, "(Page)i"),
	GB_METHOD("_IsCountSet", "b", Printer_IsCountSet, NULL),

	GB_METHOD("Configure", "b", Printer_Configure, NULL),
	GB_METHOD("Cancel", NULL, Printer_Cancel, NULL),
	
	GB_PROPERTY("Count", "i", Printer_Count),
	GB_PROPERTY_READ("Page", "i", Printer_Page),
	
	GB_PROPERTY("Name", "s", Printer_Name),
	GB_PROPERTY("Orientation", "i", Printer_Orientation),
	GB_PROPERTY("Paper", "i", Printer_Paper),
	GB_PROPERTY("PaperWidth", "f", Printer_PaperWidth),
	GB_PROPERTY("PaperHeight", "f", Printer_PaperHeight),
	GB_PROPERTY("CollateCopies", "b", Printer_CollateCopies),
	GB_PROPERTY("ReverseOrder", "b", Printer_ReverseOrder),
	GB_PROPERTY("Duplex", "i", Printer_Duplex),
	GB_PROPERTY("GrayScale", "b", Printer_GrayScale),
	GB_PROPERTY("NumCopies", "i", Printer_NumCopies),
	GB_PROPERTY("CopyCount", "i", Printer_NumCopies),
	GB_PROPERTY("Resolution", "i", Printer_Resolution),
	GB_PROPERTY("FirstPage", "i", Printer_FirstPage),
	GB_PROPERTY("LastPage", "i", Printer_LastPage),
	GB_PROPERTY("FullPage", "b", Printer_FullPage),
	GB_PROPERTY("OutputFile", "s", Printer_OutputFile),
	
	GB_EVENT("Begin", NULL, NULL, &EVENT_Begin),
	GB_EVENT("End", NULL, NULL, &EVENT_End),
	GB_EVENT("Paginate", NULL, NULL, &EVENT_Paginate),
	GB_EVENT("Draw", NULL, NULL, &EVENT_Draw),

  GB_INTERFACE("Paint", &PAINT_Interface),

	PRINTER_DESCRIPTION,

	GB_END_DECLARE
};
