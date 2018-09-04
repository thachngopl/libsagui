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

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "sg_macros.h"
#include "utlist.h"
#ifdef _WIN32
#include "sg_utils.h"
#endif
#include "sg_routes.h"
#include "sagui.h"

static void sg__route_free(struct sg_route *route);

static struct sg_route *sg__route_new(const char *pattern, char *errmsg, size_t errlen, sg_route_cb cb, void *cls) {
    struct sg_route *route;
    PCRE2_UCHAR *err;
    size_t off;
    int errnum;
    if (strstr(pattern, "\\K")) {
        strncpy(errmsg, _("\\K is not not allowed.\n"), errlen);
        return NULL;
    }
    sg__new(route);
    off = strlen(pattern) + 3;
    if (!(route->pattern = sg__malloc(off))) {
        sg__free(route);
        oom();
    }
    snprintf(route->pattern, off, "^%s$", pattern);
    if (!(route->re = pcre2_compile((PCRE2_SPTR) route->pattern, PCRE2_ZERO_TERMINATED, PCRE2_CASELESS,
                                    &errnum, &off, NULL))) {
#define SG__ERR_SIZE 120
        sg__alloc(err, SG__ERR_SIZE);
        if (pcre2_get_error_message(errnum, err, SG__ERR_SIZE) == PCRE2_ERROR_NOMEMORY)
            oom();
        snprintf(errmsg, errlen, _("Pattern compilation failed at offset %d: %s.\n"), (unsigned int) off, err);
        sg__free(err);
        sg__route_free(route);
        return NULL;
    }
#ifdef PCRE2_JIT_SUPPORT
    errnum = pcre2_jit_compile(route->re, PCRE2_JIT_COMPLETE);
    if (errnum < 0) {
        sg__alloc(err, SG__ERR_SIZE);
        if (pcre2_get_error_message(errnum, err, SG__ERR_SIZE) == PCRE2_ERROR_NOMEMORY)
            oom();
        snprintf(errmsg, errlen, _("JIT compilation failed: %s.\n"), err);
        sg__free(err);
        sg__route_free(route);
        return NULL;
    }
#endif
#undef SG__ERR_SIZE
    if (!(route->match_data = pcre2_match_data_create_from_pattern(route->re, NULL))) {
        strncpy(errmsg, _("Cannot allocate match data from the pattern.\n"), errlen);
        sg__route_free(route);
        return NULL;
    }
    route->cb = cb;
    route->cls = cls;
    return route;
}

static void sg__route_free(struct sg_route *route) {
    pcre2_match_data_free(route->match_data);
    pcre2_code_free(route->re);
    sg__free(route->pattern);
    sg__free(route);
}

void sg__routes_err_cb(__SG_UNUSED void *cls, const char *err) {
    if (isatty(fileno(stderr)) && (fprintf(stderr, "%s", err) > 0))
        fflush(stderr);
}

void *sg_route_handle(struct sg_route *route) {
    if (!route) {
        errno = EINVAL;
        return NULL;
    }
    return route->re;
}

const char *sg_route_pattern(struct sg_route *route) {
    if (!route) {
        errno = EINVAL;
        return NULL;
    }
    return route->pattern;
}

const char *sg_route_path(struct sg_route *route) {
    if (!route) {
        errno = EINVAL;
        return NULL;
    }
    return route->path;
}

int sg_route_get_segments(struct sg_route *route, sg_get_segments_cb cb, void *cls) {
    char *segment;
    size_t off;
    int r;
    if (!route || !cb)
        return EINVAL;
    if (route->rc < 0)
        return 0;
    route->ovector = pcre2_get_ovector_pointer(route->match_data);
    for (int i = 1; i < route->rc; i++) {
        r = i << 1;
        off = route->ovector[r];
        if (!(segment = strdup(route->path + off)))
            return ENOMEM;
        segment[route->ovector[r + 1] - off] = '\0';
        r = cb(cls, segment);
        sg__free(segment);
        if (r != 0)
            return r;
    }
    return 0;
}

int sg_route_get_vars(struct sg_route *route, sg_get_vars_cb cb, void *cls) {
    PCRE2_SPTR tbl, rec;
    char *val;
    size_t off;
    uint32_t cnt, len;
    int n, r;
    if (!route || !cb)
        return EINVAL;
    if (route->rc < 0)
        return 0;
    route->ovector = pcre2_get_ovector_pointer(route->match_data);
    pcre2_pattern_info(route->re, PCRE2_INFO_NAMECOUNT, &cnt);
    if (cnt == 0)
        return 0;
    pcre2_pattern_info(route->re, PCRE2_INFO_NAMETABLE, &tbl);
    pcre2_pattern_info(route->re, PCRE2_INFO_NAMEENTRYSIZE, &len);
    rec = tbl;
    for (uint32_t i = 0; i < cnt; i++) {
        n = (rec[0] << 8) | rec[1];
        r = n << 1;
        off = route->ovector[r];
        if (!(val = strndup(route->path + off, route->ovector[r + 1] - off)))
            oom();
        r = cb(cls, (const char *) rec + 2, val);
        sg__free(val);
        if (r != 0)
            return r;
        rec += len;
    }
    return 0;
}

void *sg_route_user_data(struct sg_route *route) {
    if (!route) {
        errno = EINVAL;
        return NULL;
    }
    return route->user_data;
}

int sg_routes_add2(struct sg_route **routes, const char *pattern, char *errmsg, size_t errlen,
                   sg_route_cb cb, void *cls) {
    struct sg_route *route;
    if (!routes || !pattern || !errmsg || (errlen < 1) || !cb)
        return EINVAL;
    LL_FOREACH(*routes, route) {
        if (strncmp(pattern, route->pattern + 1, strlen(pattern)) == 0)
            return EALREADY;
    }
    if (!(route = sg__route_new(pattern, errmsg, errlen, cb, cls)))
        return -1;
    LL_APPEND(*routes, route);
    return 0;
}

int sg_routes_add(struct sg_route **routes, const char *pattern, sg_route_cb cb, void *cls) {
    char *err;
    int ret;
#define SG__ERR_SIZE 256
    sg__alloc(err, SG__ERR_SIZE);
    if ((ret = sg_routes_add2(routes, pattern, err, SG__ERR_SIZE, cb, cls)) != 0)
#undef SG__ERR_SIZE
        sg__routes_err_cb(cls, err);
    sg__free(err);
    return ret;
}

int sg_routes_clear(struct sg_route **routes) {
    struct sg_route *route, *tmp;
    if (!routes)
        return EINVAL;
    LL_FOREACH_SAFE(*routes, route, tmp) {
        LL_DELETE(*routes, route);
        sg__route_free(route);
    }
    *routes = NULL;
    return 0;
}
