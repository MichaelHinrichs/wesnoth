/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef WIDGET_MENU_HPP_INCLUDED
#define WIDGET_MENU_HPP_INCLUDED

#include <map>
#include <set>
#include <string>
#include <vector>


#include "../events.hpp"
#include "../sdl_utils.hpp"

#include "scrollarea.hpp"

#include "SDL.h"

namespace gui {

class menu : public scrollarea
{
public:

	enum ROW_TYPE { NORMAL_ROW, SELECTED_ROW, HEADING_ROW };
	//basic menu style
	class style
	{
	public:
		style();
		virtual ~style();
		virtual void init() {}

		virtual SDL_Rect item_size(const std::string& item) const;
		virtual void draw_row_bg(menu& menu_ref, const size_t row_index, const SDL_Rect& rect, ROW_TYPE type);
		virtual void draw_row(menu& menu_ref, const size_t row_index, const SDL_Rect& rect, ROW_TYPE type);

		size_t get_font_size() const;
		size_t get_cell_padding() const;
		size_t get_thickness() const;

	protected:
		size_t font_size_;
		size_t cell_padding_;
		size_t thickness_;  //additional cell padding for style use only

		int normal_rgb_, selected_rgb_, heading_rgb_;
		double normal_alpha_, selected_alpha_, heading_alpha_;
	};

	//image-border selction style
	class imgsel_style : public style
	{
	public:
		imgsel_style(const std::string &img_base, bool has_bg,
								 int normal_rgb, int selected_rgb, int heading_rgb,
								 double normal_alpha, double selected_alpha, double heading_alpha);
		virtual ~imgsel_style();

		virtual SDL_Rect item_size(const std::string& item) const;
		virtual void draw_row_bg(menu& menu_ref, const size_t row_index, const SDL_Rect& rect, ROW_TYPE type);
		virtual void draw_row(menu& menu_ref, const size_t row_index, const SDL_Rect& rect, ROW_TYPE type);

		virtual void init() { load_images(); }
		bool load_images();

	protected:
		const std::string img_base_;
		std::map<std::string,surface> img_map_;

	private:
		bool load_image(const std::string &img_sub);
		bool has_background_;
		bool initialized_;
		bool load_failed_;
		int normal_rgb2_, selected_rgb2_, heading_rgb2_;
		double normal_alpha2_, selected_alpha2_, heading_alpha2_;
	};
	friend class style;
	friend class imgsel_style;
	static style default_style;
	static imgsel_style slateborder_style;
	static imgsel_style bluebg_style;

	struct item
	{
		item() : id(0)
		{}

		item(const std::vector<std::string>& fields, size_t id)
			: fields(fields), id(id)
		{}

		std::vector<std::string> fields;
		std::vector<std::string> help;
		size_t id;
	};

	class sorter
	{
	public:
		virtual ~sorter() {}
		virtual bool column_sortable(int column) const = 0;
		virtual bool less(int column, const item& row1, const item& row2) const = 0;
	};

	class basic_sorter : public sorter
	{
	public:
		basic_sorter();
		virtual ~basic_sorter() {}

		basic_sorter& set_alpha_sort(int column);
		basic_sorter& set_numeric_sort(int column);
		basic_sorter& set_id_sort(int column);
		basic_sorter& set_redirect_sort(int column, int to);
		basic_sorter& set_position_sort(int column, const std::vector<int>& pos);
	protected:
		virtual bool column_sortable(int column) const;
		virtual bool less(int column, const item& row1, const item& row2) const;

	private:
		std::set<int> alpha_sort_, numeric_sort_, id_sort_;
		std::map<int,int> redirect_sort_;
		std::map<int,std::vector<int> > pos_sort_;
	};

	menu(CVideo& video, const std::vector<std::string>& items,
	     bool click_selects=false, int max_height=-1, int max_width=-1,
		 const sorter* sorter_obj=NULL, style *menu_style=NULL);

	int selection() const;

	enum SELECTION_MOVE_VIEWPORT { MOVE_VIEWPORT, NO_MOVE_VIEWPORT };
	void move_selection(size_t id, SELECTION_MOVE_VIEWPORT move_viewport=MOVE_VIEWPORT);
	void reset_selection();

	// allows user to change_item while running (dangerous)
	void change_item(int pos1,int pos2,const std::string& str);

	void erase_item(size_t index);

	void set_heading(const std::vector<std::string>& heading);

	/// Set new items to show and redraw/recalculate everything. If
	/// strip_spaces is false, spaces will remain at the item edges. If
	/// keep_viewport is true, the menu tries to keep the selection at
	/// the same position as it were before the items were set.
	void set_items(const std::vector<std::string>& items, bool strip_spaces=true,
				   bool keep_viewport=false);

	/// Set a new max height for this menu. Note that this does not take
	/// effect immediately, only after certain operations that clear
	/// everything, such as set_items().
	void set_max_height(const int new_max_height);
	void set_max_width(const int new_max_width);

	size_t nitems() const { return items_.size(); }

	int process();

	bool double_clicked();

	void set_click_selects(bool value);
	void set_numeric_keypress_selection(bool value);

	void scroll(unsigned int pos);

	void sort_by(int column);

protected:
	bool item_ends_with_image(const std::string& item) const;
	void handle_event(const SDL_Event& event);
	void set_inner_location(const SDL_Rect& rect);

	bool requires_event_focus() const { return true; };
	const std::vector<int>& column_widths() const;
	virtual void draw_row(const size_t row_index, const SDL_Rect& rect, ROW_TYPE type);

	style *style_;
private:
	size_t max_items_onscreen() const;

	size_t heading_height() const;

	int max_height_, max_width_;
	mutable int max_items_, item_height_;

	void adjust_viewport_to_selection();
	void key_press(SDLKey key);

	std::vector<item> items_;
	std::vector<size_t> item_pos_;

	std::vector<std::string> heading_;
	mutable int heading_height_;

	void create_help_strings();
	void process_help_string(int mousex, int mousey);

	std::pair<int,int> cur_help_;
	int help_string_;

	mutable std::vector<int> column_widths_;

	size_t selected_;
	bool click_selects_;
	bool previous_button_;
	//std::set<size_t> undrawn_items_;

	bool show_result_;

	bool double_clicked_;

	void column_widths_item(const std::vector<std::string>& row, std::vector<int>& widths) const;

	void clear_item(int item);
	void draw_contents();
	void draw();
	int hit(int x, int y) const;

	std::pair<int,int> hit_cell(int x, int y) const;
	int hit_column(int x) const;

	int hit_heading(int x, int y) const;

	mutable std::map<int,SDL_Rect> itemRects_;

	SDL_Rect get_item_rect(int item) const;
	SDL_Rect get_item_rect_internal(size_t pos) const;
	size_t get_item_height_internal(const std::vector<std::string>& item) const;
	size_t get_item_height(int item) const;
	int items_start() const;

	int items_end() const;
	int items_height() const;

	void update_scrollbar_grip_height();

	///variable which determines whether a numeric keypress should
	///select an item on the dialog
	bool num_selects_;
	// These two variables are used to get the correct double click
	// behavior so that a click that causes one double click wont be
	// counted as a first click in the "next" double click.
	bool ignore_next_doubleclick_;
	bool last_was_doubleclick_;

	const sorter* sorter_;
	int sortby_;
	bool sortreversed_;
	int highlight_heading_;

	/// Set new items to show. If strip_spaces is false, spaces will
	/// remain at the item edges.
	void fill_items(const std::vector<std::string>& items, bool strip_spaces);

	void do_sort();
	void recalculate_pos();
	void assert_pos();

	void update_size();
	void set_selection_pos(size_t pos, SELECTION_MOVE_VIEWPORT move_viewport=MOVE_VIEWPORT);
	void move_selection_up(size_t dep);
	void move_selection_down(size_t dep);

	void invalidate_row(size_t id);
	void invalidate_row_pos(size_t pos);
	void invalidate_heading();

	std::set<int> invalid_;
};



}

#endif
