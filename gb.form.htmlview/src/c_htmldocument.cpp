/***************************************************************************

  c_htmldocument.cpp

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

#define __C_HTMLDOCUMENT_CPP

#include "c_htmldocument.h"

//-------------------------------------------------------------------------

char master_css[] = 
{
#include "master.css.h"
,0
};

static litehtml::context *_html_context = NULL;

class html_document : public litehtml::document_container
{
private:
	
	litehtml::context *m_html_context;
	litehtml::document::ptr m_html;
	int m_render_w;
	
	bool _valid;
	GB_FUNCTION _func_create_font;

public:
	
	void *_object;
	
	html_document(litehtml::context* html_context, void *object);
	virtual ~html_document();
	
	bool isValid() const { return _valid; }
	bool load(const char *html);
	void refresh();
	void draw(int x, int y, int w, int h, int sx, int sy, int cw, int ch);
	
protected:
	
	virtual litehtml::uint_ptr create_font(const litehtml::tchar_t* faceName, int size, int weight, litehtml::font_style italic, unsigned int decoration, litehtml::font_metrics* fm) override;
	virtual void delete_font(litehtml::uint_ptr hFont) override;
	virtual int text_width(const litehtml::tchar_t* text, litehtml::uint_ptr hFont) override;
	virtual void draw_text(litehtml::uint_ptr hdc, const litehtml::tchar_t* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos) override;
	virtual int pt_to_px(int pt) override;
	virtual int get_default_font_size() const override;
	virtual const litehtml::tchar_t* get_default_font_name() const override;
	virtual void draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker) override;
	virtual void load_image(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl, bool redraw_on_ready) override;
	virtual void get_image_size(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl, litehtml::size& sz) override;
	virtual void draw_background(litehtml::uint_ptr hdc, const litehtml::background_paint& bg) override;
	virtual void draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root) override;

	virtual void set_caption(const litehtml::tchar_t* caption) override;
	virtual void set_base_url(const litehtml::tchar_t* base_url) override;
	virtual void link(const std::shared_ptr<litehtml::document>& doc, const litehtml::element::ptr& el) override;
	virtual void on_anchor_click(const litehtml::tchar_t* url, const litehtml::element::ptr& el) override;
	virtual	void set_cursor(const litehtml::tchar_t* cursor) override;
	virtual	void transform_text(litehtml::tstring& text, litehtml::text_transform tt) override;
	virtual void import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl) override;
	virtual void set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius, bool valid_x, bool valid_y) override;
	virtual void del_clip() override;
	virtual void get_client_rect(litehtml::position& client) const override;
	virtual std::shared_ptr<litehtml::element> create_element(const litehtml::tchar_t *tag_name, const litehtml::string_map &attributes, const std::shared_ptr<litehtml::document> &doc) override;

	virtual void get_media_features(litehtml::media_features& media) const override;
	virtual void get_language(litehtml::tstring& language, litehtml::tstring & culture) const override;
	virtual litehtml::tstring	resolve_color(const litehtml::tstring& color) const override;
};

//-------------------------------------------------------------------------

html_document::html_document(litehtml::context *html_context, void *object)
{
	m_html_context = html_context;
	m_html = NULL;
	m_render_w = 0;
	_object = object;
	_valid = false;
	
	GB.GetFunction(&_func_create_font, THIS, "CreateFont", "sibbbb", "_HtmlDocumentFont")
	|| (_valid = true);
	
}

html_document::~html_document()
{
}

void html_document::draw(int x, int y, int w, int h, int sx, int sy, int cw, int ch)
{
	if (!m_html)
		return;
	
	litehtml::position pos;
	pos.x = x;
	pos.y = y;
	pos.width = w;
	pos.height = h;
	
	if (cw != m_render_w)
	{
		m_html->render(cw);
		m_render_w = cw;
	}
	
	m_html->draw((litehtml::uint_ptr)this, 0, 0, &pos);
}

void html_document::refresh()
{
	m_render_w = 0;
}

bool html_document::load(const char *html)
{
	m_html = litehtml::document::createFromString(html, this, m_html_context);
	return (m_html == NULL);
}

litehtml::uint_ptr html_document::create_font(const litehtml::tchar_t* faceName, int size, int weight, litehtml::font_style italic, unsigned int decoration, litehtml::font_metrics* fm)
{
	GB_VALUE *ret;
	CHTMLDOCUMENTFONT *html_font;
	void *font;
	
	GB.Push(6,
		GB_T_STRING, faceName, strlen(faceName),
		GB_T_INTEGER, size,
		GB_T_BOOLEAN, weight >= 550,
		GB_T_BOOLEAN, italic == litehtml::fontStyleItalic,
		GB_T_BOOLEAN, !!(decoration & litehtml::font_decoration_linethrough),
		GB_T_BOOLEAN, !!(decoration & litehtml::font_decoration_underline)
	);
	
	ret = GB.Call(&_func_create_font, 6, FALSE);
	html_font = (CHTMLDOCUMENTFONT *)ret->_object.value;
	
	fm->ascent = html_font->ascent;
	fm->descent = html_font->descent;
	fm->height = html_font->height;
	fm->x_height = html_font->x_height;
	font = html_font->font;
	
	GB.Ref(font);
	return (litehtml::uint_ptr)font;
}

void html_document::delete_font(litehtml::uint_ptr hFont)
{
	GB.Unref(POINTER(&hFont));
}

int html_document::text_width(const litehtml::tchar_t* text, litehtml::uint_ptr hFont)
{
}

void html_document::draw_text(litehtml::uint_ptr hdc, const litehtml::tchar_t* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos)
{
}

int html_document::pt_to_px(int pt)
{
}

int html_document::get_default_font_size() const
{
}

const litehtml::tchar_t* html_document::get_default_font_name() const
{
}

void html_document::draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker)
{
}

void html_document::load_image(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl, bool redraw_on_ready)
{
}

void html_document::get_image_size(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl, litehtml::size& sz)
{
}

void html_document::draw_background(litehtml::uint_ptr hdc, const litehtml::background_paint& bg)
{
}

void html_document::draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root)
{
}

void html_document::set_caption(const litehtml::tchar_t* caption)
{
}

void html_document::set_base_url(const litehtml::tchar_t* base_url)
{
}

void html_document::link(const std::shared_ptr<litehtml::document>& doc, const litehtml::element::ptr& el)
{
}

void html_document::on_anchor_click(const litehtml::tchar_t* url, const litehtml::element::ptr& el)
{
}

void html_document::set_cursor(const litehtml::tchar_t* cursor)
{
}

void html_document::transform_text(litehtml::tstring& text, litehtml::text_transform tt)
{
}

void html_document::import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl)
{
}

void html_document::set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius, bool valid_x, bool valid_y)
{
}

void html_document::del_clip()
{
}

void html_document::get_client_rect(litehtml::position& client) const
{
}

std::shared_ptr<litehtml::element> html_document::create_element(const litehtml::tchar_t *tag_name, const litehtml::string_map &attributes, const std::shared_ptr<litehtml::document> &doc)
{
}

void html_document::get_media_features(litehtml::media_features& media) const
{
}

void html_document::get_language(litehtml::tstring& language, litehtml::tstring & culture) const
{
}

litehtml::tstring	html_document::resolve_color(const litehtml::tstring& color) const
{
	return litehtml::tstring();
}


//-------------------------------------------------------------------------

BEGIN_METHOD_VOID(HtmlDocument_new)

	if (!_html_context)
	{
		_html_context = new litehtml::context;
		_html_context->load_master_stylesheet(master_css);
	}

	THIS->doc = new html_document(_html_context, THIS);
	if (!THIS->doc->isValid())
		GB.Error("Invalid interface");

END_METHOD

BEGIN_METHOD_VOID(HtmlDocument_free)

	delete THIS->doc;
	
END_METHOD

BEGIN_METHOD(HtmlDocument_Load, GB_STRING html)

	if (THIS->doc->load(GB.ToZeroString(ARG(html))))
		GB.Error("Unable to parse HTML");

END_METHOD

BEGIN_METHOD(HtmlDocument_Draw, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER sx; GB_INTEGER sy; GB_INTEGER cw; GB_INTEGER ch)

	THIS->doc->draw(VARG(x), VARG(y), VARG(w), VARG(h), VARG(sx), VARG(sy), VARG(cw), VARG(ch));

END_METHOD

BEGIN_METHOD_VOID(HtmlDocument_Refresh)

	THIS->doc->refresh();

END_METHOD

//-------------------------------------------------------------------------

BEGIN_METHOD_VOID(HtmlDocumentFont_free)

	GB.Unref(&THIS_FONT->font);

END_METHOD

BEGIN_PROPERTY(HtmlDocumentFont_Font)

	if (READ_PROPERTY)
		GB.ReturnObject(THIS_FONT->font);
	else
		GB.StoreObject(PROP(GB_OBJECT), &THIS_FONT->font);

END_PROPERTY

#define IMPLEMENT_FONT_PROPERTY(_name, _prop) \
BEGIN_PROPERTY(HtmlDocumentFont_##_name) \
 \
	if (READ_PROPERTY) \
		GB.ReturnInteger(THIS_FONT->_prop); \
	else \
		THIS_FONT->_prop = VPROP(GB_INTEGER); \
 \
END_PROPERTY

IMPLEMENT_FONT_PROPERTY(Ascent, ascent)
IMPLEMENT_FONT_PROPERTY(Descent, descent)
IMPLEMENT_FONT_PROPERTY(Height, height)
IMPLEMENT_FONT_PROPERTY(XHeight, x_height)

//-------------------------------------------------------------------------

GB_DESC HtmlDocumentFontDesc[] = 
{
	GB_DECLARE("_HtmlDocumentFont", sizeof(CHTMLDOCUMENTFONT)),
	
	GB_METHOD("_free", NULL, HtmlDocumentFont_free, NULL),
						 
	GB_PROPERTY("Font", "Font", HtmlDocumentFont_Font),
	GB_PROPERTY("Ascent", "i", HtmlDocumentFont_Ascent),
	GB_PROPERTY("Descent", "i", HtmlDocumentFont_Descent),
	GB_PROPERTY("Height", "i", HtmlDocumentFont_Height),
	GB_PROPERTY("XHeight", "i", HtmlDocumentFont_XHeight),
	
	GB_END_DECLARE
};

GB_DESC HtmlDocumentDesc[] = 
{
	GB_DECLARE("_HtmlDocument", sizeof(CHTMLDOCUMENT)),
	
	GB_METHOD("_new", NULL, HtmlDocument_new, NULL),
	GB_METHOD("_free", NULL, HtmlDocument_free, NULL),
	
	GB_METHOD("Load", NULL, HtmlDocument_Load, "(Html)s"),
	GB_METHOD("Draw", NULL, HtmlDocument_Draw, "(X)i(Y)i(Width)i(Height)i(ScrollX)i(ScrollY)i(ClientW)i(ClientH)i"),
	GB_METHOD("Refresh", NULL, HtmlDocument_Refresh, NULL),
						 
	GB_END_DECLARE
};
