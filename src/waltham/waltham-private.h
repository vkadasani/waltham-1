/*
 * Copyright © 2008-2011 Kristian Høgsberg
 * Copyright © 2011 Intel Corporation
 * Copyright © 2013 Jason Ekstrand
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef WALTHAM_PRIVATE_H
#define WALTHAM_PRIVATE_H

#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>

#include "waltham-object.h"
#include "waltham-connection.h"
#include "waltham-util.h"

void
wth_debug(const char *fmt, ...) WTH_PRINTF(1, 2);

void
wth_error(const char *fmt, ...) WTH_PRINTF(1, 2);

void
wth_abort(const char *fmt, ...) WTH_PRINTF(1, 2);

#define ARRAY_LENGTH(a) (sizeof (a) / sizeof (a)[0])

#define WTH_SERVER_ID_START 0xff000000

/* Flags for wth_map_insert_new and wth_map_insert_at.  Flags can be queried with
 * wth_map_lookup_flags.  The current implementation has room for 1 bit worth of
 * flags.  If more flags are ever added, the implementation of wth_map will have
 * to change to allow for new flags */
enum wth_map_entry_flags {
	WTH_MAP_ENTRY_LEGACY = (1 << 0)
};

struct wth_map {
	struct wth_array client_entries;
	struct wth_array server_entries;
	uint32_t side;
	uint32_t free_list;
};

typedef enum wth_iterator_result (*wth_iterator_func_t)(void *element,
							void *data);

void
wth_map_init(struct wth_map *map, uint32_t side);

void
wth_map_release(struct wth_map *map);

uint32_t
wth_map_insert_new(struct wth_map *map, uint32_t flags, void *data);

int
wth_map_insert_at(struct wth_map *map, uint32_t flags, uint32_t i, void *data);

int
wth_map_reserve_new(struct wth_map *map, uint32_t i);

void
wth_map_remove(struct wth_map *map, uint32_t i);

void *
wth_map_lookup(struct wth_map *map, uint32_t i);

uint32_t
wth_map_lookup_flags(struct wth_map *map, uint32_t i);

void
wth_map_for_each(struct wth_map *map, wth_iterator_func_t func, void *data);

/* wth_connection */
void
wth_connection_insert_new_object(struct wth_connection *conn,
    struct wth_object *obj);

void
wth_connection_insert_object_with_id(struct wth_connection *conn,
    struct wth_object *obj);

void
wth_connection_remove_object(struct wth_connection *conn,
    struct wth_object *obj);

struct wth_object *
wth_connection_get_object(struct wth_connection *conn, uint32_t id);

void
wth_connection_assert_side(struct wth_connection *conn,
			   const char *func,
			   enum wth_connection_side expected);

#define ASSERT_CLIENT_SIDE(conn) \
	wth_connection_assert_side((conn), __func__, \
				   WTH_CONNECTION_SIDE_CLIENT)

#define ASSERT_SERVER_SIDE(conn) \
	wth_connection_assert_side((conn), __func__, \
				   WTH_CONNECTION_SIDE_SERVER)

/* wth_object */
struct wth_object {
	struct wth_connection *connection;
	uint32_t id;

	void (**vfunc)(void);
	void *user_data;
};

/** Create a protocol object with given ID
 *
 * \param connection The Waltham connection.
 * \param id The object ID to reserve.
 * \return A new protocol object proxy with the given ID.
 *
 * \memberof wth_object
 * \private
 */
struct wth_object *
wth_object_new_with_id(struct wth_connection *connection, uint32_t id);

/** Create a protocol object
 *
 * \param connection The Waltham connection.
 * \return A new protocol object proxy.
 *
 * The object ID on the wire is automatically allocated.
 *
 * \memberof wth_object
 * \private
 */
struct wth_object *
wth_object_new(struct wth_connection *connection);

/** Send an error on a specific object
 *
 * \param conn The Waltham connection.
 * \param obj A Waltham object.
 * \param code The error code.
 * \param str The error message.
 *
 * This function wraps the wth_display_send_error display function, so
 * the server doesn’t have to use this private API.
 *
 * \memberof wth_connection
 * \server_api
 */
void
wth_connection_send_error(struct wth_connection *conn, struct wth_object *obj,
                          uint32_t code, const char *str);

#endif
