#include "yconfig.h"

#include <string>
#include <map>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define LEN_MAX  4096

using namespace std;


typedef enum _yconfig_value_type_s {
    YCONFIG_VALUE_TYPE_STRING = 0x01,
    YCONFIG_VALUE_TYPE_INT    = 0x02,
} yconfig_value_type_t;

typedef struct _yconfig_val_s {
    yconfig_value_type_t type;
    void *val;
} yconfig_val_t;

struct _yconfig_s {
    map<string, map<string, yconfig_val_t> > vals;
    string sname;
};



static int check_key_char(char c)
{
    if (c == '.' || c == '_' || c == '-' || isalnum(c))
        return 1;
    return 0;
}



static int check_value_int(const char *l, const char *r)
{
    for (const char *p = l; p != r; ++p)
        if (!isdigit(*p))
            return 1;
    return 0;
}



static void line_strip_space(const char **l, const char **r)
{
    const char *p = NULL;

    // skip left spaces
    for (p = *l; p != *r; ++p)
        if (!isspace(*p)) {
            *l = p;
            break;
        }
    if (p == *r) {
        *l = p;
        return;
    }

    // skip right spaces
    for (p = *r - 1; p >= *l; --p)
        if (!isspace(*p)) {
            *r = p + 1;
            break;
        }
}



// return comment
static const char *line_strip_comment(const char **l, const char **r, int *len)
{
    const char *comment = NULL;
    const char *r_tmp = NULL;

    line_strip_space(l, r);

    for (const char *p = *l; p != *r; ++p)
        if (*p == '#') {
            r_tmp = p;
            break;
        }

    if (r_tmp) {
        for (const char *p = r_tmp; p != *r; ++p)
            if (*p != '#' && !isspace(*p)) {
                comment = p;
                *len = *r - p;
                break;
            }
        *r = r_tmp;
    }

    return comment;
}



// return section name
static const char *line_strip_section(const char **l, const char **r, int *len)
{
    const char *sname = NULL;

    line_strip_space(l, r);

    if (*r - *l <= 2)
        return sname;
    if (**l != '[' || *(*r-1) != ']')
        return sname;

    ++*l;
    --*r;
    line_strip_space(l, r);

    if (*l >= *r)
        return sname;

    sname = *l;
    *len = *r - *l;

    return sname;
}



// return key
static const char *line_strip_key(const char **l, const char **r, int *len)
{
    const char *key = NULL;
    const char *p = NULL;

    line_strip_space(l, r);

    if (!isalnum(**l))
        return key;

    for (p = *l; p != *r; ++p)
        if (!check_key_char(*p)) {
            if (p - *l > 0) {
                key = *l;
                *len = p - *l;
            }
            break;
        }

    if (key)
        *l = p;

    return key;
}



// return 0 if has delimiter
static int line_strip_delimiter(const char **l, const char **r)
{
    line_strip_space(l, r);

    if (**l != '=')
        return 1;

    ++*l;

    return 0;
}



// return NULL if error
static void *line_strip_value(const char **l, const char **r, yconfig_value_type_t *val_type)
{
    void *ret = NULL;

    line_strip_space(l, r);

    if (*r <= *l)
        return ret;

    if (**l == '"') {
        if (*(*r-1) != '"')
            return ret;
        *val_type = YCONFIG_VALUE_TYPE_STRING;
        char *s = (char *)calloc(1, *r - *l - 1);
        memcpy(s, *l + 1, *r - *l - 2);
        ret = s;
    }
    else if (!check_value_int(*l, *r)) {
        *val_type = YCONFIG_VALUE_TYPE_INT;
        int *val = (int *)malloc(sizeof(int));
        char *s = (char *)calloc(1, *r - *l + 1);
        memcpy(s, *l, *r - *l);
        *val = atoi(s);
        free(s);
        ret = val;
    }

    return ret;
}



static int line_insert(yconfig_t *yc, const string& sname, yconfig_value_type_t type, const string& key, void *val)
{
    if (sname.empty()) {
        free(val);
        return 1;
    }

    if (yc->vals.find(sname) != yc->vals.end()
            && yc->vals[sname].find(key) != yc->vals[sname].end())
        free(yc->vals[sname][key].val);

    yconfig_val_t v;
    v.type = type;
    v.val = val;
    yc->vals[sname][key] = v;

    return 0;
}



static int line_to_tree(yconfig_t *yc, const char *line)
{
    int ret = 0;
    yconfig_value_type_t val_type;
    const char *sname = NULL;
    const char *key = NULL;
    void *val = NULL;
    const char *l = NULL;
    const char *r = NULL;
    int line_len = 0;
    int ele_len = 0;

    line_len = strlen(line);
    if (!line_len)
        return ret;

    l = line;
    r = line + line_len;

    // ignore comment
    line_strip_comment(&l, &r, &ele_len);

    sname = line_strip_section(&l, &r, &ele_len);
    if (sname)
        yc->sname = string(sname, ele_len);
    else {
        key = line_strip_key(&l, &r, &ele_len);
        if (!key)
            goto quit;

        if (line_strip_delimiter(&l, &r)) {
            ret = 1;
            goto quit;
        }

        val = line_strip_value(&l, &r, &val_type);
        if (!val) {
            ret = 1;
            goto quit;
        }

        ret = line_insert(yc, yc->sname, val_type, string(key, ele_len), val);
    }

    return ret;

quit:
    return ret;
}



extern "C" yconfig_t *yconfig_init(void)
{
    yconfig_t *yc = new yconfig_t();
    return yc;
}



extern "C" void yconfig_destroy(yconfig_t *yc)
{
    if (yc) {
        for (map<string, map<string, yconfig_val_t> >::iterator it
                = yc->vals.begin(); it != yc->vals.end(); ++it)
            for (map<string, yconfig_val_t>::iterator it2
                    = it->second.begin(); it2 != it->second.end(); ++it2)
                free(it2->second.val);
        delete yc;
    }
}



extern "C" int yconfig_parse(yconfig_t *yc, const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp)
        return 1;

    yc->sname.clear();

    char *buf = (char *)malloc(LEN_MAX);
    for (int i = 1; fgets(buf, LEN_MAX, fp); ++i)
        if (line_to_tree(yc, buf))
            printf("Error: line %d, skip it.\n", i);
    free(buf);

    fclose(fp);

    return 0;
}



extern "C" int yconfig_file(yconfig_t *yc, const char *filename)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp)
        return 1;

    for (map<string, map<string, yconfig_val_t> >::const_iterator it
            = yc->vals.begin(); it != yc->vals.end(); ++it) {
        fprintf(fp, "[%s]\n", it->first.c_str());
        for (map<string, yconfig_val_t>::const_iterator it2
                = it->second.begin(); it2 != it->second.end(); ++it2) {
            if (it2->second.type == YCONFIG_VALUE_TYPE_STRING)
                fprintf(fp, "%s = \"%s\"\n", it2->first.c_str(),
                        (const char *)it2->second.val);
            else if (it2->second.type == YCONFIG_VALUE_TYPE_INT)
                fprintf(fp, "%s = %d\n", it2->first.c_str(),
                        *(const int *)it2->second.val);
        }
    }

    fclose(fp);

    return 0;
}



extern "C" int yconfig_query_int(yconfig_t *yc, const char *sname, const char *name, int *val)
{
    if (yc->vals.find(sname) != yc->vals.end()
            && yc->vals[sname].find(name) != yc->vals[sname].end())
        if (yc->vals[sname][name].type == YCONFIG_VALUE_TYPE_INT) {
            *val = *(int *)yc->vals[sname][name].val;
            return 0;
        }

    return 1;
}



extern "C" const char *yconfig_query_string(yconfig_t *yc, const char *sname, const char *name)
{
    if (yc->vals.find(sname) != yc->vals.end()
            && yc->vals[sname].find(name) != yc->vals[sname].end())
        if (yc->vals[sname][name].type == YCONFIG_VALUE_TYPE_STRING)
            return (const char *)yc->vals[sname][name].val;

    return NULL;
}



extern "C" int yconfig_set_int(yconfig_t *yc, const char *sname, const char *name, int val)
{
    int *data = (int *)malloc(sizeof(int));
    *data = val;
    return line_insert(yc, sname, YCONFIG_VALUE_TYPE_INT, name, data);
}



extern "C" int yconfig_set_string(yconfig_t *yc, const char *sname, const char *name, const char *val)
{
    char *data = strdup(val);
    return line_insert(yc, sname, YCONFIG_VALUE_TYPE_STRING, name, data);
}
