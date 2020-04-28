/*
** Zabbix
** Copyright (C) 2001-2020 Zabbix SIA
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**/

#include "common.h"
#include "zbxjson.h"
#include "log.h"
#include "module.h"

static int	discovery_desc(AGENT_REQUEST *request, AGENT_RESULT *result);

static ZBX_METRIC keys[] =
/*	KEY			FLAG		FUNCTION	TEST PARAMETERS */
{
	{"net.if.discovery.desc",	0,	discovery_desc,	NULL},
	{NULL}
};

int	zbx_module_api_version(void)
{
	return ZBX_MODULE_API_VERSION;
}

ZBX_METRIC	*zbx_module_item_list(void)
{
	return keys;
}

static int	discovery_desc(AGENT_REQUEST *request, AGENT_RESULT *result)
{
	char		line[MAX_STRING_LEN], path[MAX_STRING_LEN], *p;
	FILE		*fp, *fd;
	struct zbx_json	j;

	ZBX_UNUSED(request);

	if (NULL == (fp = fopen("/proc/net/dev", "r")))
	{
		SET_MSG_RESULT(result, zbx_dsprintf(NULL, "Cannot open /proc/net/dev: %s", zbx_strerror(errno)));
		return SYSINFO_RET_FAIL;
	}

	zbx_json_initarray(&j, ZBX_JSON_STAT_BUF_LEN);

	while (NULL != fgets(line, sizeof(line), fp))
	{
		if (NULL == (p = strstr(line, ":")))
			continue;

		*p = '\0';

		/* trim left spaces */
		for (p = line; ' ' == *p && '\0' != *p; p++)
		    ;

		zbx_json_addobject(&j, NULL);
		zbx_json_addstring(&j, "{#IFNAME}", p, ZBX_JSON_TYPE_STRING);

		zbx_snprintf(path, MAX_STRING_LEN, "/sys/class/net/%s/ifalias", p);
		if (NULL == (fd = fopen(path, "r")))
        {
        	zabbix_log(LOG_LEVEL_DEBUG,"%s cannot open %s: %s", __func__, path, zbx_strerror(errno));
        } else {
        	line[0] = '\0';

            if (NULL != fgets(line, sizeof(line), fd))
            {
           		/* trim \n */
           		line[strlen(line)-1] = '\0';
            }
           	zbx_json_addstring(&j, "{#IFDESC}", line, ZBX_JSON_TYPE_STRING);

            zbx_fclose(fd);
        }

		zbx_json_close(&j);
	}

	zbx_fclose(fp);

	zbx_json_close(&j);

	SET_STR_RESULT(result, strdup(j.buffer));

	zbx_json_free(&j);

	return SYSINFO_RET_OK;
}

int	zbx_module_init(void)
{
	return ZBX_MODULE_OK;
}

int	zbx_module_uninit(void)
{
	return ZBX_MODULE_OK;
}

