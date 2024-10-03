#ifndef CONFIG_H
#define CONFIG_H

// TODO: dyn
#define TABLOID_VERSION     "0.0.1"

#define DEFAULT_TAB_SZ      8
#define DEFAULT_LINE_PREFIX "~"

typedef struct {
  unsigned short tab_sz;
  char*          ln_prefix;
} config_t;

#endif /* CONFIG_H */
