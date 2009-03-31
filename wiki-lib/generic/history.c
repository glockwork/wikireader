/*
 *  Copyright (c) 2009 Holger Hans Peter Freyther <zecke@openmoko.org>
 *  Copyright (c) 2009 Matt Hsu <matt_hsu@openmoko.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <wikilib.h>
#include <guilib.h>
#include <glyph.h>
#include <lsearcher.h>
#include <list.h>
#include <search.h>

#include <stdlib.h>

#include "history.h"

#define RESULT_START 28
#define RESULT_HEIGHT 10

#define HISTORY_MAX_ITEM	100

struct history_item {
	struct wl_list list;
	char title[MAXSTR];
	char target[TARGET_SIZE];
};

struct history_item head, free_list;
struct history_item pool[HISTORY_MAX_ITEM];

unsigned int list_size = 0;
static int history_current = -1;

// Copy and pasted form search.c.... Find something better but I don't
// want to use structs to have these variable..
static void __invert_selection(int old_pos, int new_pos)
{
	int start = RESULT_START - RESULT_HEIGHT + 2;

	guilib_fb_lock();

	if (old_pos != -1) {
		guilib_invert(start + old_pos * RESULT_HEIGHT, RESULT_HEIGHT);
	}

	if (new_pos != -1 ) {
		guilib_invert(start + new_pos * RESULT_HEIGHT, RESULT_HEIGHT);
	}

	guilib_fb_unlock();
}

void history_select_down(void)
{
	/* bottom reached, not wrapping around */
	if (history_current + 1 == (int) list_size)
		return;

	__invert_selection(history_current, history_current + 1);
	++history_current;
}

void history_select_up(void)
{
	/* top reached, not wrapping around */
	if (history_current <= 0)
		return;

	__invert_selection(history_current, history_current - 1);
	--history_current;
}


void history_display(void)
{
	unsigned int i;

	guilib_fb_lock();

	guilib_clear();
	render_string(0, 1, 14, "History", 7);

	if (list_size == 0) {
		render_string(0, 1, 100, "No history.", 11);
	} else {
		int y_pos = RESULT_START;

		for (i = 0; i <= list_size && y_pos < FRAMEBUFFER_HEIGHT; i++) {
			const char *p = history_get_item_title(i);
			render_string(0, 1, y_pos, p, strlen(p)- (TARGET_SIZE+1));
			y_pos += RESULT_HEIGHT;
		}
	}

	guilib_fb_unlock();
}

void history_reset(void)
{
	history_current = -1;
}

const char *history_current_target(void)
{
	return history_get_item_target(history_current);
}

static int history_item_comp(const void *value, unsigned int offset, const struct wl_list *node)
{
	char *p = (void *)(node)+sizeof(struct wl_list)+offset;

	return strcmp(p, (char *)value);
}

struct history_item *history_find_item_title(const char *title)
{
	return (struct history_item *)
		(wl_list_search(&head.list, title, 0, history_item_comp));
}

struct history_item *history_find_item_target(const char *target)
{
	return (struct history_item *)
		(wl_list_search(&head.list, target, (sizeof(char)*MAXSTR), history_item_comp));
}

void history_add(const char *title, const char *target)
{
	struct history_item *node = NULL;

	/* check this title is existed or not */
	if ((node = history_find_item_title(title))){
		wl_list_move2_first(&head.list, &node->list);
		return;
	}

	/* check the list size, if it's over 100,kill one entry then */
	if (list_size >= HISTORY_MAX_ITEM) {
		node = (struct history_item *)wl_list_remove_last(&head.list);
		wl_list_insert_after(&free_list.list, &node->list);
	}

	/* linked to the head list */
	node = (struct history_item *)wl_list_remove_last(&free_list.list);

	strcpy(node->title, title);
	strcpy(node->target, target);

	wl_list_insert_after(&head.list, &node->list);

	list_size++;
}

void history_move_current_to_top(void)
{
}

const char *history_get_top_target(void)
{
	return history_get_item_target(0);
}

struct history_item * __history_get_item(unsigned int index)
{
	if (index > list_size)
		return NULL;

	return (struct history_item *)(wl_list_find_nth_node(&head.list, index));
}

const char *history_get_item_title(unsigned int index)
{
	struct history_item *p = __history_get_item(index);

	if (p)
		return p->title;
	else
		return NULL;
}

const char *history_get_item_target(unsigned int index)
{
	struct history_item *p = __history_get_item(index);

	if (p)
		return p->target;
	else
		return NULL;
}

unsigned int history_item_size(void)
{
	return wl_list_size(&head.list);
}

unsigned int history_free_item_size(void)
{
	return wl_list_size(&free_list.list);
}

void history_list_init()
{
	int i;

	wl_list_init(&head.list);
	wl_list_init(&free_list.list);

	for (i = 0; i < HISTORY_MAX_ITEM; i++){
		wl_list_init(&pool[i].list);
		wl_list_insert_after(&free_list.list, &pool[i].list);
	}
}

