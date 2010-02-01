#include "conf.h"
#include "privs.h"

#include <stdbool.h>

#define _DEFAULT_FORMAT "/etc/proftpd/%u.conf"

static const char * const MODULE_NAME = "mod_dynamic_include";
static char *path_format = _DEFAULT_FORMAT;

static bool is_secure_path(const char *path) {

    if(!path)
        return false;

    if(strstr(path, ".."))
        return false;

    return true;
}

MODRET set_dynamic_include(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_GLOBAL);

  /* 全プロセスで共有していいのでstaticな領域に確保 */
  path_format = pstrdup(cmd->server->pool, cmd->argv[1]);
  pr_log_debug(DEBUG3,
    "%s: set DynamicIncludePath '%s'", MODULE_NAME, path_format);

  return PR_HANDLED(cmd);
}

MODRET dynamic_include_post_pass(cmd_rec *cmd) {
  const char *conf = NULL;
  char *user = NULL;

  user = get_param_ptr(cmd->server->conf, C_USER, FALSE);

#ifdef _SUPPORT_SUBACCOUNT
  char *found;
  user = pstrdup(cmd->tmp_pool, user);
  if((found = strchr(user, '_'))) {
      user[found - user] = '\0';
  }
#endif

  /* todo ( if you want another formats string )*/
  conf = sreplace(cmd->tmp_pool, path_format,
              "%u", pstrdup(cmd->tmp_pool, user), NULL);

  pr_log_debug(DEBUG3,
    "%s: try to Include '%s'", MODULE_NAME, conf);

  if(!is_secure_path(conf)) {
    pr_log_pri(PR_LOG_WARNING,
      "%s: config path is insecure '%s'", MODULE_NAME, conf);
    pr_response_send(R_530, _("Login Denied."));
    end_login(0);
  }

  /* パスが不正なら読み込みしないでスルーする */
  if (pr_fs_valid_path(conf) < 0) {
    pr_log_pri(PR_LOG_NOTICE,
      "%s: unable to use path for configuration file '%s'", MODULE_NAME, conf);
    return PR_DECLINED(cmd);
  }

  /* pr_parser_prepare()呼び出しておかないとSIGSEGV出す */
  pr_parser_prepare(cmd->server->pool, NULL);
  if(pr_parser_parse_file(cmd->server->pool, conf, NULL, 0) == 0 ) {
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

  if (pr_parser_cleanup() < 0) {
      pr_log_pri(PR_LOG_ERR, "Fatal: error processing configuration file '%s': "
       "unclosed configuration section", conf);
      end_login(1);
  }

  return PR_DECLINED(cmd);
}

static conftable dynamic_include_conftab[] = {
  { "DynamicInclude", set_dynamic_include, NULL },
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
