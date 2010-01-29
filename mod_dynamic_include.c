#include "conf.h"
#include "privs.h"

#define _DEFAULT_FORMAT "/etc/proftpd/%u.conf"

static const char * const MODULE_NAME = "mod_dynamic_include";
static char *path_format = _DEFAULT_FORMAT;

MODRET set_dynamic_include_path(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_GLOBAL);

  /* 全プロセスで共有していいのでstaticな領域に確保 */
  path_format = pstrdup(main_server->pool, cmd->argv[1]);
  pr_log_debug(DEBUG3,
               "%s: set DynamicIncludePath '%s'", MODULE_NAME, path_format);
  
  return PR_HANDLED(cmd);
}

MODRET dynamic_include_post_pass(cmd_rec *cmd) {
  const char *conf = NULL;
  const char *user = NULL;

  /* todo */
  user = get_param_ptr(cmd->server->conf, C_USER, FALSE);
  conf = sreplace(cmd->tmp_pool, path_format, "%u", user, NULL);


  pr_log_debug(DEBUG3,
               "%s: try to Include '%s'", MODULE_NAME, conf);

  /* パスが不正なら読み込みしないでスルーする */
  if (pr_fs_valid_path(conf) < 0) {
    pr_log_pri(PR_LOG_WARNING,
               "unable to use path for configuration file '%s'", conf);
    return PR_DECLINED(cmd);
  }

  /* pr_parser_prepare()呼び出しておかないとSIGSEGV出す */
  pr_parser_prepare(main_server->pool, NULL);
  if(pr_parser_parse_file(main_server->pool, conf, NULL, 0) == 0 ) {
      pr_log_debug(DEBUG3,
                   "%s: OK parse_config_path('%s')", MODULE_NAME, conf);
  } else {
    if (errno != EINVAL)
      pr_log_pri(PR_LOG_WARNING, "warning: unable to include '%s': %s",
        conf, strerror(errno));
    else {
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "error including '", conf,
        "': ", strerror(errno), NULL));
    }
  }

  return PR_DECLINED(cmd);
}

static conftable dynamic_include_conftab[] = {
  { "DynamicIncludePath", set_dynamic_include_path, NULL },
  { NULL },
};

static cmdtable dynamic_include_cmdtab[] = {
  { POST_CMD, C_USER, G_NONE, dynamic_include_post_pass, FALSE, FALSE },
  { 0, NULL }
};

module dynamic_include_module = {
  NULL, NULL,

  /* Module API version */
  0x20,

  /* Module name */
  "dynamic_include",

  /* Module configuration directive table */
  dynamic_include_conftab,

  /* Module command handler table */
  dynamic_include_cmdtab,

  /* Module authentication handler table */
  NULL,

  /* Module initialization function */
  NULL,

  /* Session initialization function */
  NULL,
};
