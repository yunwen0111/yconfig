#ifndef __YCONFIG_H__
#define __YCONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */


typedef struct _yconfig_s yconfig_t;


yconfig_t  *yconfig_init(void);
void        yconfig_destroy(yconfig_t *yc);
int         yconfig_parse(yconfig_t *yc, const char *filename);
int         yconfig_file(yconfig_t *yc, const char *filename);

int         yconfig_query_int(yconfig_t *yc, const char *sname, const char *name, int *val);
const char *yconfig_query_string(yconfig_t *yc, const char *sname, const char *name);

int         yconfig_set_int(yconfig_t *yc, const char *sname, const char *name, int val);
int         yconfig_set_string(yconfig_t *yc, const char *sname, const char *name, const char *val);


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* __YCONFIG_H__ */
