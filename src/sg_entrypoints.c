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

#include <stdlib.h>
#include "sg_macros.h"
#include "sg_entrypoint.h"
#include "sg_entrypoints.h"
#include "sagui.h"

static int sg__entrypoints_add(struct sg_entrypoints *entrypoints, struct sg_entrypoint *entrypoint, void *user_data) {
    if (bsearch(entrypoint, entrypoints->list, entrypoints->count, sizeof(struct sg_entrypoint), sg__entrypoint_cmp))
        return EALREADY;
    if (!(entrypoints->list = sg__realloc(entrypoints->list, (entrypoints->count + 1) * sizeof(struct sg_entrypoint))))
        return ENOMEM;
    sg__entrypoint_prepare(entrypoints->list + entrypoints->count++, entrypoint->name, user_data);
    qsort(entrypoints->list, entrypoints->count, sizeof(struct sg_entrypoint), sg__entrypoint_cmp);
    return 0;
}

static int sg__entrypoints_find(struct sg_entrypoints *entrypoints, struct sg_entrypoint *key,
                                struct sg_entrypoint **entrypoint) {
    if ((entrypoints->count > 0) && (*entrypoint = bsearch(key, entrypoints->list, entrypoints->count,
                                                           sizeof(struct sg_entrypoint), sg__entrypoint_cmp)))
        return 0;
    return ENOENT;
}

struct sg_entrypoints *sg_entrypoints_new(void) {
    return sg_alloc(sizeof(struct sg_entrypoints));
}

void sg_entrypoints_free(struct sg_entrypoints *entrypoints) {
    sg_entrypoints_clear(entrypoints);
    sg__free(entrypoints);
}

int sg_entrypoints_add2(struct sg_entrypoints *entrypoints, const char *name, void *user_data) {
    struct sg_entrypoint entrypoint;
    int ret;
    if (!entrypoints || !name)
        return EINVAL;
    if (!(entrypoint.name = strdup(name)))
        return ENOMEM;
    if ((ret = sg__entrypoints_add(entrypoints, &entrypoint, user_data) != 0))
        sg__free(entrypoint.name);
    return ret;
}

int sg_entrypoints_add(struct sg_entrypoints *entrypoints, const char *path, void *user_data) {
    struct sg_entrypoint entrypoint;
    int ret;
    if (!entrypoints || !path)
        return EINVAL;
    if (!(entrypoint.name = sg_extract_entrypoint(path)))
        return ENOMEM;
    if ((ret = sg__entrypoints_add(entrypoints, &entrypoint, user_data) != 0))
        sg__free(entrypoint.name);
    return ret;
}

int sg_entrypoints_clear(struct sg_entrypoints *entrypoints) {
    if (!entrypoints)
        return EINVAL;
    for (unsigned int i = 0; i < entrypoints->count; i++)
        sg__free((entrypoints->list + i)->name);
    sg__free(entrypoints->list);
    entrypoints->list = NULL;
    entrypoints->count = 0;
    return 0;
}

int sg_entrypoints_find2(struct sg_entrypoints *entrypoints, struct sg_entrypoint **entrypoint, const char *name) {
    struct sg_entrypoint key;
    int ret;
    if (!entrypoints || !entrypoint || !name)
        return EINVAL;
    if (!(key.name = strdup(name)))
        return ENOMEM;
    ret = sg__entrypoints_find(entrypoints, &key, entrypoint);
    sg__free(key.name);
    return ret;
}

int sg_entrypoints_find(struct sg_entrypoints *entrypoints, struct sg_entrypoint **entrypoint, const char *path) {
    struct sg_entrypoint key;
    int ret;
    if (!entrypoints || !entrypoint || !path)
        return EINVAL;
    if (!(key.name = sg_extract_entrypoint(path)))
        return ENOMEM;
    ret = sg__entrypoints_find(entrypoints, &key, entrypoint);
    sg__free(key.name);
    return ret;
}
