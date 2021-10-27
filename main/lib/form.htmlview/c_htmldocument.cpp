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
	bool _valid;
	int m_client_w;
	int m_client_h;
	
	GB_FUNCTION _func_draw_text;

public:
	
	void *_object;
	
	html_document(litehtml::context* html_context, void *object);
	virtual ~html_document();
	
	bool isValid() const { return _valid; }
	bool load(const char *html);
	bool render(int w, int h);
	void draw(int x, int y, int w, int h);
	int getWidth() const;
	int getHeight() const;
	
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
	m_client_w = 0;
	m_client_h = 0;
	_object = object;
	_valid = false;
	
	GB.GetFunction(&_func_draw_text, THIS, "DrawText", "sFont;iii", NULL)
	|| (_valid = true);
	
}

html_document::~html_document()
{
}

bool html_document::render(int w, int h)
{
	if (!m_html)
		return true;
	
	m_html->render(w);
	m_client_w = w;
	m_client_h = h;
	return false;
}

void html_document::draw(int x, int y, int w, int h)
{
	if (!m_html)
		return;
	
	litehtml::position pos;
	pos.x = x;
	pos.y = y;
	pos.width = w;
	pos.height = h;
	
	m_client_h = h;
	
	m_html->draw((litehtml::uint_ptr)this, 0, 0, &pos);
}

int html_document::getWidth() const
{
	return m_html ? m_html->width() : 0;
}

int html_document::getHeight() const
{
	return m_html ? m_html->height() : 0;
}

bool html_document::load(const char *html)
{
	m_html = litehtml::document::createFromUTF8(html, this, m_html_context);
	m_client_w = 0;
	m_client_h = 0;
	return (m_html == NULL);
}

litehtml::uint_ptr html_document::create_font(const litehtml::tchar_t* faceName, int size, int weight, litehtml::font_style italic, unsigned int decoration, litehtml::font_metrics* fm)
{
	GB_FUNCTION func;
	GB_VALUE *ret;
	GB_VALUE val;
	void *font;
	
	font = GB.New(GB.FindClass("Font"), NULL, NULL);
	
	val.type = GB_T_CSTRING;
	val._string.value.addr = (char *)faceName;
	val._string.value.start = 0;
	val._string.value.len = strlen(faceName);
	GB.SetProperty(font, "Name", &val);
	
	val.type = GB_T_FLOAT;
	val._float.value = size;
	GB.SetProperty(font, "Size", &val);
	
	val.type = GB_T_BOOLEAN;
	val._boolean.value = weight >= 550;
	GB.SetProperty(font, "Bold", &val);
	
	val.type = GB_T_BOOLEAN;
	val._boolean.value = italic == litehtml::fontStyleItalic;
	GB.SetProperty(font, "Italic", &val);
	
	val.type = GB_T_BOOLEAN;
	val._boolean.value = !!(decoration & litehtml::font_decoration_underline);
	GB.SetProperty(font, "Underline", &val);
	
	val.type = GB_T_BOOLEAN;
	val._boolean.value = !!(decoration & litehtml::font_decoration_linethrough);
	GB.SetProperty(font, "Strikeout", &val);
	
	ret = GB.GetProperty(font, "Ascent");
	fm->ascent = ret->_integer.value;
	
	ret = GB.GetProperty(font, "Descent");
	fm->descent = ret->_integer.value;
	
	ret = GB.GetProperty(font, "Height");
	fm->height = ret->_integer.value;
	
	GB.GetFunction(&func, font, "TextHeight", "s", "i");
	GB.Push(1, GB_T_STRING, "x", 1);
	ret = GB.Call(&func, 1, FALSE);
	fm->x_height = ret->_integer.value;
	
	GB.Ref(font);
	return (litehtml::uint_ptr)font;
}

void html_document::delete_font(litehtml::uint_ptr hFont)
{
	GB.Unref(POINTER(&hFont));
}

int html_document::text_width(const litehtml::tchar_t* text, litehtml::uint_ptr hFont)
{
	GB_VALUE *ret;
	GB_FUNCTION func;
	GB.GetFunction(&func, hFont, "TextWidth", "s", "i");
	GB.Push(1, GB_T_STRING, text, strlen(text));
	ret = GB.Call(&func, 1, FALSE);
	return ret->_integer.value;
}

void html_document::draw_text(litehtml::uint_ptr hdc, const litehtml::tchar_t* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos)
{
	GB.Push(5,
		GB_T_STRING, text, strlen(text),
		GB_T_OBJECT, hFont,
		GB_T_INTEGER, GB_COLOR_MAKE(color.red, color.green, color.blue, color.alpha),
		GB_T_INTEGER, pos.x,
		GB_T_INTEGER, pos.y);
	GB.Call(&_func_draw_text, 5, TRUE);
}

int html_document::pt_to_px(int pt)
{
	GB_VALUE *ret;
	GB_FUNCTION func;
	GB.GetFunction(&func, THIS, "PtToPx", "i", "i");
	GB.Push(1, GB_T_INTEGER, pt);
	ret = GB.Call(&func, 1, FALSE);
	return ret->_integer.value;
}

int html_document::get_default_font_size() const
{
	GB_VALUE *ret;
	
	ret = GB.GetProperty(THIS, "DefaultFont");
	void *font = ret->_object.value;
	ret = GB.GetProperty(font, "Size");
	return (int)ret->_float.value;
}

const litehtml::tchar_t* html_document::get_default_font_name() const
{
	GB_VALUE *ret;
	
	ret = GB.GetProperty(THIS, "DefaultFont");
	void *font = ret->_object.value;
	ret = GB.GetProperty(font, "Name");
	return GB.ToZeroString((GB_STRING *)ret);
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
	client.x = 0;
	client.y = 0;
	client.width = m_client_w;
	client.height = m_client_h;
}

std::shared_ptr<litehtml::element> html_document::create_element(const litehtml::tchar_t *tag_name, const litehtml::string_map &attributes, const std::shared_ptr<litehtml::document> &doc)
{
	return NULL;
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

BEGIN_METHOD(HtmlDocument_Render, GB_INTEGER w; GB_INTEGER h)

	THIS->doc->render(VARG(w), VARG(h));

END_METHOD

BEGIN_METHOD(HtmlDocument_Draw, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	THIS->doc->draw(VARG(x), VARG(y), VARG(w), VARG(h));

END_METHOD

BEGIN_PROPERTY(HtmlDocument_Width)

	GB.ReturnInteger(THIS->doc->getWidth());

END_PROPERTY

BEGIN_PROPERTY(HtmlDocument_Height)

	GB.ReturnInteger(THIS->doc->getHeight());

END_PROPERTY

//-------------------------------------------------------------------------

GB_DESC HtmlDocumentDesc[] = 
{
	GB_DECLARE("_HtmlDocument", sizeof(CHTMLDOCUMENT)),
	
	GB_METHOD("_new", NULL, HtmlDocument_new, NULL),
	GB_METHOD("_free", NULL, HtmlDocument_free, NULL),
	
	GB_METHOD("Load", NULL, HtmlDocument_Load, "(Html)s"),
	GB_METHOD("Render", NULL, HtmlDocument_Render, "(Width)i(Height)i"),
	GB_METHOD("Draw", NULL, HtmlDocument_Draw, "(X)i(Y)i(Width)i(Height)i"),
	
	GB_PROPERTY_READ("Width", "i", HtmlDocument_Width),
	GB_PROPERTY_READ("W", "i", HtmlDocument_Width),
	GB_PROPERTY_READ("Height", "i", HtmlDocument_Height),
	GB_PROPERTY_READ("H", "i", HtmlDocument_Height),
						 
	GB_END_DECLARE
};
