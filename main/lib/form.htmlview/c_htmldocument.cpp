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

#define GET_CURRENT() GB_PAINT *_paint = (GB_PAINT *)DRAW.Paint.GetCurrent()
#define PAINT _paint->desc
#define CURRENT _paint

//-------------------------------------------------------------------------

static int convert_color(const litehtml::web_color &color)
{
	return GB_COLOR_MAKE(color.red, color.green, color.blue, color.alpha);
}

static int get_font_ascent(void *font)
{
	GB_VALUE *ret = GB.GetProperty(font, "Ascent");
	return ret->_integer.value;
}


//-------------------------------------------------------------------------

struct clip_box
{
	typedef std::vector<clip_box> vector;
	litehtml::position	box;
	litehtml::border_radiuses radius;

	clip_box(const litehtml::position& vBox, litehtml::border_radiuses vRad)
	{
		box = vBox;
		radius = vRad;
	}

	clip_box(const clip_box& val)
	{
		box = val.box;
		radius = val.radius;
	}
	
	clip_box& operator=(const clip_box& val)
	{
		box = val.box;
		radius = val.radius;
		return *this;
	}
};


class html_document : public litehtml::document_container
{
private:
	
	litehtml::context *m_html_context;
	litehtml::document::ptr m_html;
	bool _valid;
	int m_client_w;
	int m_client_h;
	clip_box::vector m_clips;
	
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
	
	void rounded_rectangle(const litehtml::position &pos, const litehtml::border_radiuses &radius, bool keep = false, bool back = false);
	void begin_clip();
	void end_clip();
	void set_color(const litehtml::web_color &color) { DRAW.Paint.SetBackground(convert_color(color)); }
	void on_mouse_move(int x, int y);
	
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
	_valid = true;
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
	
	fm->ascent = get_font_ascent(font);
	
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
	GB.GetFunction(&func, (void *)hFont, "TextWidth", "s", "i");
	GB.Push(1, GB_T_STRING, text, strlen(text));
	ret = GB.Call(&func, 1, FALSE);
	return ret->_integer.value;
}

void html_document::draw_text(litehtml::uint_ptr hdc, const litehtml::tchar_t* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos)
{
	GET_CURRENT();
	
	begin_clip();
	PAINT->MoveTo(CURRENT, (float)pos.x, (float)(pos.y + get_font_ascent((void *)hFont)));
	PAINT->Font(CURRENT, TRUE, (void **)&hFont);
	set_color(color);
	PAINT->Text(CURRENT, text, strlen(text), -1, -1, GB_DRAW_ALIGN_DEFAULT, TRUE);
	end_clip();
}

int html_document::pt_to_px(int pt)
{
	GET_CURRENT();
	
	return pt * CURRENT->resolutionX / 72;
	
	/*GB_VALUE *ret;
	GB_FUNCTION func;
	GB.GetFunction(&func, THIS, "PtToPx", "i", "i");
	GB.Push(1, GB_T_INTEGER, pt);
	ret = GB.Call(&func, 1, FALSE);
	return ret->_integer.value;*/
}

int html_document::get_default_font_size() const
{
	return THIS->default_font_size ? THIS->default_font_size : 16;
}

const litehtml::tchar_t* html_document::get_default_font_name() const
{
	return THIS->default_font_name ? THIS->default_font_name : "Sans";
}

void html_document::draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker)
{
	GET_CURRENT();
	float lw;
	
	begin_clip();
	
	switch(marker.marker_type)
	{
		case litehtml::list_style_type_circle:
			
			PAINT->Ellipse(CURRENT, marker.pos.x, marker.pos.y, marker.pos.width, marker.pos.height, 0, M_PI * 2, FALSE);
			lw = 0.5;
			PAINT->LineWidth(CURRENT, TRUE, &lw);
			set_color(marker.color);
			PAINT->Stroke(CURRENT, FALSE);
			break;
			
		case litehtml::list_style_type_disc:
			
			PAINT->Ellipse(CURRENT, marker.pos.x, marker.pos.y, marker.pos.width, marker.pos.height, 0, M_PI * 2, FALSE);
			set_color(marker.color);
			PAINT->Fill(CURRENT, FALSE);
			break;
			
		case litehtml::list_style_type_square:
			
			PAINT->FillRect(CURRENT, marker.pos.x, marker.pos.y, marker.pos.width, marker.pos.height, convert_color(marker.color));
			break;
			
		default:
			/*do nothing*/
			break;
	}
	
	end_clip();
}

void html_document::load_image(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl, bool redraw_on_ready)
{
}

void html_document::get_image_size(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl, litehtml::size& sz)
{
}

void html_document::draw_background(litehtml::uint_ptr hdc, const litehtml::background_paint& bg)
{
	GET_CURRENT();
	
	begin_clip();

	rounded_rectangle(bg.border_box, bg.border_radius);
	PAINT->Clip(CURRENT, FALSE);

	PAINT->Rectangle(CURRENT, bg.clip_box.x, bg.clip_box.y, bg.clip_box.width, bg.clip_box.height);
	
	if (bg.color.alpha)
	{
		PAINT->Clip(CURRENT, TRUE);
		set_color(bg.color);
		PAINT->Fill(CURRENT, FALSE);
	}
	else
		PAINT->Clip(CURRENT, FALSE);

	end_clip();
}


void html_document::draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root)
{
	litehtml::position inner_pos;
	litehtml::border_radiuses inner_radius;
	bool btop, bright, bbottom, bleft;
	
	btop = borders.top.width > 0 && borders.top.style > litehtml::border_style_hidden; // && borders.top.color.alpha > 0;
	bright = borders.right.width > 0 && borders.right.style > litehtml::border_style_hidden; // && borders.right.color.alpha > 0;
	bbottom = borders.bottom.width > 0 && borders.bottom.style > litehtml::border_style_hidden; // && borders.bottom.color.alpha > 0;
	bleft = borders.left.width > 0 && borders.left.style > litehtml::border_style_hidden; // && borders.left.color.alpha > 0;
	
	if (!btop && !bright && !bbottom && !bleft)
		return;
	
	GET_CURRENT();
	
	begin_clip();

	inner_radius = borders.radius;
	
	inner_radius.top_left_y = inner_radius.top_left_x;
	inner_radius.top_right_y = inner_radius.top_right_x;
	inner_radius.bottom_left_y = inner_radius.bottom_left_x;
	inner_radius.bottom_right_y = inner_radius.bottom_right_x;
	
	inner_radius.top_left_x -= borders.left.width;
	inner_radius.top_left_y -= borders.top.width;
	inner_radius.top_right_x -= borders.right.width;
	inner_radius.top_right_y -= borders.top.width;
	inner_radius.bottom_left_x -= borders.left.width;
	inner_radius.bottom_left_y -= borders.bottom.width;
	inner_radius.bottom_right_x -= borders.right.width;
	inner_radius.bottom_right_y -= borders.bottom.width;
	inner_radius.fix_values();
	
	inner_pos = draw_pos;
	
	inner_pos.x += borders.left.width;
	inner_pos.width -= borders.left.width + borders.right.width;
	inner_pos.y += borders.top.width;
	inner_pos.height -= borders.top.width + borders.bottom.width;
	
	rounded_rectangle(draw_pos, borders.radius);
	if (inner_pos.width > 0 && inner_pos.height > 0)
		rounded_rectangle(inner_pos, inner_radius, true, true);
	
	set_color(borders.top.color);
	PAINT->Fill(CURRENT, FALSE);
	
	end_clip();
}


void html_document::set_caption(const litehtml::tchar_t* caption)
{
	GB_FUNCTION func;
	if (GB.GetFunction(&func, THIS, "SetTitle", "s", NULL))
		return;
	GB.Push(1, GB_T_STRING, caption, strlen(caption));
	GB.Call(&func, 1, TRUE);
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
	GB_FUNCTION func;
	if (GB.GetFunction(&func, THIS, "SetCursor", "s", NULL))
		return;
	GB.Push(1, GB_T_STRING, cursor, strlen(cursor));
	GB.Call(&func, 1, TRUE);
}

void html_document::transform_text(litehtml::tstring& text, litehtml::text_transform tt)
{
}

void html_document::import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl)
{
	GB_VALUE *ret;
	GB_FUNCTION func;
	if (GB.GetFunction(&func, THIS, "ImportCSS", "ss", "s"))
		return;
	
	GB.Push(2, GB_T_STRING, url.data(), url.length(), GB_T_STRING, baseurl.data(), baseurl.length());
	ret = GB.Call(&func, 2, FALSE);
	text.assign(ret->_string.value.addr + ret->_string.value.start, ret->_string.value.len);
}

void html_document::rounded_rectangle(const litehtml::position &pos, const litehtml::border_radiuses &radius, bool keep, bool back)
{
	float rtlx, rtlx2, rtrx, rtrx2, rblx, rblx2, rbrx, rbrx2;
	float rtly, rtly2, rtry, rtry2, rbly, rbly2, rbry, rbry2;
	float mrx, mry;
	
	float x = pos.x;
	float y = pos.y;
	float w = pos.width;
	float h = pos.height;
	
	if (w <= 0 || h <= 0)
		return;
	
	GET_CURRENT();

	if (radius.top_left_x == 0 && radius.top_left_y == 0 && radius.top_right_x == 0 && radius.top_right_y == 0
	    && radius.bottom_left_x == 0 && radius.bottom_left_y == 0 && radius.bottom_right_x == 0 && radius.bottom_right_y == 0)
	{
		if (back)
		{
			PAINT->MoveTo(CURRENT, x, y);
			PAINT->LineTo(CURRENT, x, y + h);
			PAINT->LineTo(CURRENT, x + w, y + h);
			PAINT->LineTo(CURRENT, x + w, y);
			PAINT->LineTo(CURRENT, x, y);
		}
		else
		{
			PAINT->MoveTo(CURRENT, x, y);
			PAINT->LineTo(CURRENT, x + w, y);
			PAINT->LineTo(CURRENT, x + w, y + h);
			PAINT->LineTo(CURRENT, x, y + h);
			PAINT->LineTo(CURRENT, x, y);
		}
		
		return;
	}
	
	if (false) //keep)
	{
		mrx = w / 2;
		mry = y / 2;
	}
	else
	{
		mrx = mry = Min(w, h) / 2;
	}
	
	rtlx = Min(radius.top_left_x, mrx);
	rtlx2 = rtlx * (1-0.55228475);
	
	if (keep)
	{
		rtly = Min(radius.top_left_y, mry);
		rtly2 = rtly * (1-0.55228475);
	}
	else
	{
		rtly = rtlx;
		rtly2 = rtlx2;
	}
	
	rtrx = Min(radius.top_right_x, mrx);
	rtrx2 = rtrx * (1-0.55228475);
	
	if (keep)
	{
		rtry = Min(radius.top_right_y, mry);
		rtry2 = rtry * (1-0.55228475);
	}
	else
	{
		rtry = rtrx;
		rtry2 = rtrx2;
	}
	
	rblx = Min(radius.bottom_left_x, mrx);
	rblx2 = rblx * (1-0.55228475);
	
	if (keep)
	{
		rbly = Min(radius.bottom_left_y, mry);
		rbly2 = rbly * (1-0.55228475);
	}
	else
	{
		rbly = rblx;
		rbly2 = rblx2;
	}
	
	rbrx = Min(radius.bottom_right_x, mrx);
	rbrx2 = rbrx * (1-0.55228475);

	if (keep)
	{
		rbry = Min(radius.bottom_right_y, mry);
		rbry2 = rbry * (1-0.55228475);
	}
	else
	{
		rbry = rbrx;
		rbry2 = rbrx2;
	}
	
	if (back)
	{
		PAINT->MoveTo(CURRENT, x + w - rtrx, y);
		PAINT->LineTo(CURRENT, x + rtlx, y);
		PAINT->CurveTo(CURRENT, x + rtlx2, y, x, y + rtly2, x, y + rtly);
		PAINT->LineTo(CURRENT, x, y + h - rbly);
		PAINT->CurveTo(CURRENT, x, y + h - rbly2, x + rblx2, y + h, x + rblx, y + h);
		PAINT->LineTo(CURRENT, x + w - rbrx, y + h);
		PAINT->CurveTo(CURRENT, x + w - rbrx2, y + h, x + w, y + h - rbry2, x + w, y + h - rbry);
		PAINT->LineTo(CURRENT, x + w, y + rtry);
		PAINT->CurveTo(CURRENT, x + w, y + rtry2, x + w - rtrx2, y, x + w - rtrx, y);
	}
	else
	{
		PAINT->MoveTo(CURRENT, x + rtlx, y);
		PAINT->LineTo(CURRENT, x + w - rtrx, y);
		PAINT->CurveTo(CURRENT, x + w - rtrx2, y, x + w, y + rtry2, x + w, y + rtry);
		PAINT->LineTo(CURRENT, x + w, y + h - rbry);
		PAINT->CurveTo(CURRENT, x + w, y + h - rbry2, x + w - rbrx2, y + h, x + w - rbrx, y + h);
		PAINT->LineTo(CURRENT, x + rblx, y + h);
		PAINT->CurveTo(CURRENT, x + rblx2, y + h, x, y + h - rbly2, x, y + h - rbly);
		PAINT->LineTo(CURRENT, x, y + rtly);
		PAINT->CurveTo(CURRENT, x, y + rtly2, x + rtlx2, y, x + rtlx, y);
	}
}

void html_document::set_clip( const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius, bool valid_x, bool valid_y )
{
	litehtml::position clip_pos = pos;
	litehtml::position client_pos;
	get_client_rect(client_pos);
	if(!valid_x)
	{
		clip_pos.x = client_pos.x;
		clip_pos.width = client_pos.width;
	}
	if(!valid_y)
	{
		clip_pos.y = client_pos.y;
		clip_pos.height = client_pos.height;
	}
	m_clips.emplace_back(clip_pos, bdr_radius);
}

void html_document::del_clip()
{
	if(!m_clips.empty())
		m_clips.pop_back();
}

void html_document::begin_clip()
{
	GET_CURRENT();
	
	PAINT->Save(CURRENT);
	
	for(const auto& clip_box : m_clips)
	{
		rounded_rectangle(clip_box.box, clip_box.radius);
		PAINT->Clip(CURRENT, FALSE);
	}
}

void html_document::end_clip()
{
	GET_CURRENT();
	
	PAINT->Restore(CURRENT);
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

void html_document::on_mouse_move(int x, int y)
{
	litehtml::position::vector redraw_boxes;
	
	if (m_html->on_mouse_over(x, y, x, y, redraw_boxes))
	{
		GB_FUNCTION func;
		if (GB.GetFunction(&func, THIS, "Refresh", "iiii", NULL))
			return;
	
		for(auto& pos : redraw_boxes)
		{
			GB.Push(4, GB_T_INTEGER, pos.x, GB_T_INTEGER, pos.y, GB_T_INTEGER, pos.width, GB_T_INTEGER, pos.height);
			GB.Call(&func, 4, TRUE);
		}
	}
}

//-------------------------------------------------------------------------

static void reload_document(void *_object)
{
	if (THIS->doc && THIS->html && *THIS->html && THIS->doc->load(THIS->html))
		GB.Error("Unable to parse HTML");
}

//-------------------------------------------------------------------------

BEGIN_METHOD_VOID(HtmlDocument_new)

	// Check needed interface

END_METHOD

BEGIN_METHOD_VOID(HtmlDocument_free)

	GB.FreeString(&THIS->html);
	GB.FreeString(&THIS->default_font_name);
	delete THIS->doc;
	delete THIS->context;
	
END_METHOD

BEGIN_PROPERTY(HtmlDocument_Html)

	if (READ_PROPERTY)
		GB.ReturnString(THIS->html);
	else
	{
		GB.StoreString(PROP(GB_STRING), &THIS->html);
		reload_document(THIS);
	}

END_PROPERTY

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

BEGIN_METHOD(HtmlDocument_LoadCss, GB_STRING css)

	if (THIS->context)
	{
		delete THIS->context;
		delete THIS->doc;
	}
	
	THIS->context = new litehtml::context;
	THIS->context->load_master_stylesheet(GB.ToZeroString(ARG(css)));

	THIS->doc = new html_document(THIS->context, THIS);
	
	reload_document(THIS);

END_METHOD

BEGIN_METHOD_VOID(HtmlDocument_Reload)

	reload_document(THIS);

END_METHOD

BEGIN_METHOD(HtmlDocument_SetDefaultFont, GB_OBJECT font)

	GB_VALUE *ret;
	GB_FONT font;
	
	font = VARG(font);
	if (GB.CheckObject(font))
		return;
	
	ret = GB.GetProperty(font, "Size");
	THIS->default_font_size = (int)ret->_float.value;
	
	ret = GB.GetProperty(font, "Name");
	GB.StoreString((GB_STRING *)ret, &THIS->default_font_name);

END_METHOD

BEGIN_METHOD(HtmlDocument_OnMouseMove, GB_INTEGER x; GB_INTEGER y)

	if (THIS->doc)
		THIS->doc->on_mouse_move(VARG(x), VARG(y));
	
END_METHOD

//-------------------------------------------------------------------------

GB_DESC HtmlDocumentDesc[] = 
{
	GB_DECLARE("_HtmlDocument", sizeof(CHTMLDOCUMENT)),
	
	GB_METHOD("_new", NULL, HtmlDocument_new, NULL),
	GB_METHOD("_free", NULL, HtmlDocument_free, NULL),
	
	GB_METHOD("Render", NULL, HtmlDocument_Render, "(Width)i(Height)i"),
	GB_METHOD("Draw", NULL, HtmlDocument_Draw, "(X)i(Y)i(Width)i(Height)i"),
	
	GB_PROPERTY("Html", "s", HtmlDocument_Html),
	GB_METHOD("LoadCss", NULL, HtmlDocument_LoadCss, "(Css)s"),
	GB_METHOD("SetDefaultFont", NULL, HtmlDocument_SetDefaultFont, "(Font)Font;"),
	GB_METHOD("Reload", NULL, HtmlDocument_Reload, NULL),
	
	GB_METHOD("OnMouseMove", NULL, HtmlDocument_OnMouseMove, "ii"),
	
	GB_PROPERTY_READ("Width", "i", HtmlDocument_Width),
	GB_PROPERTY_READ("W", "i", HtmlDocument_Width),
	GB_PROPERTY_READ("Height", "i", HtmlDocument_Height),
	GB_PROPERTY_READ("H", "i", HtmlDocument_Height),
						 
	GB_END_DECLARE
};
