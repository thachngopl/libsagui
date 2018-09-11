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

struct sg_entrypoints *sg_entrypoints_new(void) {
    return sg_alloc(sizeof(struct sg_entrypoints));
}

void sg_entrypoints_free(struct sg_entrypoints *entrypoints) {
    sg_entrypoints_clear(entrypoints);
    sg__free(entrypoints);
}

int sg_entrypoints_add(struct sg_entrypoints *entrypoints, const char *path, void *user_data) {
    struct sg_entrypoint key;
    int ret;
    if (!entrypoints || !path)
        return EINVAL;
    if (!(key.name = sg_extract_entrypoint(path)))
        return ENOMEM;
    if (bsearch(&key, entrypoints->list, entrypoints->count, sizeof(struct sg_entrypoint), sg__entrypoint_cmp)) {
        ret = EALREADY;
        goto fail;
    }
    if (!(entrypoints->list = sg__realloc(entrypoints->list,
                                          (entrypoints->count + 1) * sizeof(struct sg_entrypoint)))) {
        ret = ENOMEM;
        goto fail;
    }
    sg__entrypoint_prepare(entrypoints->list + entrypoints->count++, key.name, user_data);
    qsort(entrypoints->list, entrypoints->count, sizeof(struct sg_entrypoint), sg__entrypoint_cmp);
    return 0;
fail:
    sg__free(key.name);
    if (ret == ENOMEM)
        oom();
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

int sg_entrypoints_find(struct sg_entrypoints *entrypoints, struct sg_entrypoint **entrypoint, const char *path) {
    struct sg_entrypoint key;
    int ret = ENOENT;
    if (!entrypoints || !entrypoint || !path)
        return EINVAL;
    if (!(key.name = sg_extract_entrypoint(path)))
        return ENOMEM;
    if ((entrypoints->count > 0) &&
        (*entrypoint = bsearch(&key, entrypoints->list, entrypoints->count, sizeof(struct sg_entrypoint),
                               sg__entrypoint_cmp)))
        ret = 0;
    sg__free(key.name);
    return ret;
}
