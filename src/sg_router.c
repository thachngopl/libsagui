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
#include <string.h>
#include <errno.h>
#include "sg_macros.h"
#include "utlist.h"
#include "sg_routes.h"
#include "sg_router.h"
#include "sagui.h"

struct sg_router *sg_router_new2(struct sg_route *routes, sg_err_cb err_cb, void *cls) {
    struct sg_router *router;
    if (!routes || !err_cb) {
        errno = EINVAL;
        return NULL;
    }
    sg__new(router);
    router->routes = routes;
    router->err_cb = err_cb;
    router->err_cls = cls;
    return router;
}

struct sg_router *sg_router_new(struct sg_route *routes) {
    return sg_router_new2(routes, sg__routes_err_cb, NULL);
}

void sg_router_free(struct sg_router *router) {
    if (!router)
        return;
    sg_routes_clear(&router->routes);
    sg__free(router);
}

int sg_router_dispatch(struct sg_router *router, const char *path, void *user_data) {
    struct sg_route *route;
    if (!router || !path || !router->routes)
        return EINVAL;
    LL_FOREACH(router->routes, route) {
        route->rc =
#ifdef PCRE2_JIT_SUPPORT
                    pcre2_jit_match
#else
                    pcre2_match
#endif
                                   (route->re, (PCRE2_SPTR) path, strlen(path), 0, 0, route->match_data, NULL);
        if (route->rc >= 0) {
            route->path = path;
            route->user_data = user_data;
            route->cb(route->cls, route);
            return 0;
        }
    }
    return ENOENT;
}
