/*                         _
 *   ___  __ _  __ _ _   _(_)
 *  / __|/ _` |/ _` | | | | |
 *  \__ \ (_| | (_| | |_| | |
 *  |___/\__,_|\__, |\__,_|_|
 *             |___/
 *
 *   –– an ideal C library to develop cross-platform HTTP servers.
 *
 * Copyright (c) 2016-2018 Silvio Clecio <silvioprog@gmail.com>
 *
 * This file is part of Sagui library.
 *
 * Sagui library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Sagui library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Sagui library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sg_entrypoint.h"
#include "sg_entrypoints.h"

struct sg_entrypoints *sg_entrypoints_new(void) {
    return NULL;
}

int sg_entrypoints_add(struct sg_entrypoints *entrypoints, struct sg_router *router, const char *entrypoint) {
    (void) entrypoints;
    (void) router;
    (void) entrypoint;
    return 0;
}

void sg_entrypoints_free(struct sg_entrypoints *entrypoints) {
    (void) entrypoints;
}

struct sg_entrypoint *sg_entrypoints_find(const char *path) {
    (void) path;
    return NULL;
}
