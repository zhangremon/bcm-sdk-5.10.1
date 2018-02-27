/*
 * $Id: regex.c,v 1.4.6.1 2011/05/05 07:22:41 mlarsen Exp $
 * $Copyright: (c) 2005 Broadcom Corp.
 * All Rights Reserved.$
 *
 * File:        txrx.c
 * Purpose:
 * Requires:    
 */

#ifdef INCLUDE_REGEX
#include <sal/types.h>
#include <sal/core/time.h>
#include <sal/core/thread.h>
#include <sal/core/libc.h>
#include <sal/appl/sal.h>
#include <sal/appl/io.h>

#include <soc/types.h>
#include <soc/debug.h>

#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/net.h>
#include <linux/in.h>
#else
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#endif

#include <bcm/port.h>
#include <bcm/error.h>

#include <appl/diag/system.h>
#include <bcm/bregex.h>

char cmd_regex_usage[] = 
"Parameters\n"
"\tfile -- load the patterns from file\n";

typedef struct xd_s {  
    pbmp_t      pbmp; 
    char        *xd_file;
} xd_t;

typedef struct regex_app_info_s {
    char                app_name[64];
    bcm_regex_match_t   match;
    bcm_regex_engine_t  engine;
    struct regex_app_info_s *next;
} regex_app_info_t;

static regex_app_info_t *app_list[BCM_MAX_NUM_UNITS];

static cmd_result_t
regex_parse_args(int u, args_t *a, xd_t *xd)
{
    parse_table_t       pt;
    char                *xfile = NULL;

    parse_table_init(u, &pt);

    parse_table_add(&pt, "Filename", PQ_DFL|PQ_STRING,0, &xfile,NULL);

    if (parse_arg_eq(a, &pt) < 0) {
        sal_printf("Error: Unknown option: %s\n", ARG_CUR(a));
        parse_arg_eq_done(&pt);
        return(CMD_FAIL);
    }

    if (xfile) {
        xd->xd_file = sal_strdup(xfile);
    } else {
        xd->xd_file = NULL;
    }

    parse_arg_eq_done(&pt);
    return(0);
}

static int _read_line(FILE *fp, char *b, int *len)
{
    int c;
    int l = 0;  

    c = getc(fp);
    while (!((c == EOF) || (c == '\n') || (c == '\r'))) {
        b[l++] = c;
        c = getc(fp);
    }

    *len = l;
    b[l] = '\0';
    if (l) {
        return 0;
    }

    return (c == EOF) ? -1 : 0;
}

static char* _strip_front(char *l, int len)
{
    int i=0;

    /* skip leading spaces. */
    while (i < len) {
        if ((l[i] == ' ') || (l[i] == '\t')) {
            i++;
            continue;
        }
        return &l[i];
    }
    return NULL;
}

static int decode_app_name(char* s, regex_app_info_t *app)
{
    sal_strcpy(app->app_name, s);
    return 0;
}

static int decode_match_id(char* s, regex_app_info_t *app)
{
    app->match.match_id = atoi(s);
    return 0;
}

static int decode_pattern(char* s, regex_app_info_t *app)
{
    int j = 0, esc = 0;

    if (s[0] != '/') {
        return -1;
    }

    s++;
    sal_memset(app->match.pattern, 0, BCM_REGEX_MAX_PATTERN_SIZE);
    while (*s) {
        if (!esc && (*s == '/')) {
            return 0;
        }
        app->match.pattern[j] = *s;
        if (*s == '\\') {
            esc = !!esc;
        }
        j++;
        s++;
    }

    if (app->match.pattern[j-1] == '/') {
        app->match.pattern[j-1] = '\0';
        return 0;
    }

    return 1;
}

typedef struct regex_cmd_str_to_flag_s {
    char *name;
    unsigned int flag;
} regex_cmd_str_to_flag_t;

static regex_cmd_str_to_flag_t *
regex_flag_get(char *str, regex_cmd_str_to_flag_t *tbl)
{
    while (tbl->name) {
        if (!sal_strcasecmp(str, tbl->name)) {
            return tbl;
        }
        tbl++;
    }
    return NULL;
}

static int decode_flags_cmn(char* s, regex_app_info_t *app, 
                            regex_cmd_str_to_flag_t *tbl, 
                            unsigned int *pflag)
{
    char *ts;
    int done = 0;
    regex_cmd_str_to_flag_t *ps2f;

    *pflag = 0;

    ts = s;
    while (!done) {
        /* skip leading spaces */
        while (*s && ((*s == ' ') || (*s == '\t'))) {
            s++;
        }
        if (!*s) {
            return 0;
        }
        ts = s;
        while (*s && (!((*s == ' ') || (*s == ',')))) {
            s++;
        }
        if (!*s) {
            done = 1;
        } else {
            *s = '\0';
            s++;
        }
        ps2f = regex_flag_get(ts, tbl);
        if (!ps2f) {
            printk("unknown regex action: %s\n", ts);
            return -1;
        }
        *pflag |= ps2f->flag;
    }
    return 0;
}

static regex_cmd_str_to_flag_t flow_options[] = {
    { "to_server", BCM_REGEX_MATCH_TO_SERVER },
    { "to_client", BCM_REGEX_MATCH_TO_CLIENT },
    { "case", BCM_REGEX_MATCH_CASE_INSENSITIVE },
    { "none", 0 },
    { NULL, 0 }
};

static int decode_flowopt(char* s, regex_app_info_t *app)
{
    int rv;
    
    rv = decode_flags_cmn( s, app, flow_options, &app->match.flags);
    if (rv) {
        return CMD_FAIL;
    }
    return 0;
}

static regex_cmd_str_to_flag_t match_actions[] = {
    { "ignore", BCM_REGEX_MATCH_ACTION_IGNORE },
    { "none", 0 },
    { NULL, 0 }
};

static int decode_action(char* s, regex_app_info_t *app)
{
    int rv;
    
    rv = decode_flags_cmn( s, app, match_actions, &app->match.action_flags);
    if (rv) {
        return CMD_FAIL;
    }
    return 0;
}

static int decode_previous_tag(char* s, regex_app_info_t *app)
{
    app->match.requires = atoi(s);
    return 0;
}

static int decode_next_tag(char* s, regex_app_info_t *app)
{
    app->match.provides = atoi(s);
    return 0;
}

typedef int (*decode_tag_value)(char *buf, regex_app_info_t *app);

typedef struct regex_xml_data_s {
    char            *tag_name;
    decode_tag_value decode_value;
} regex_xml_data_t;

static regex_xml_data_t regex_match_info[] = {
    { "signature", NULL},
    { "application", decode_app_name },
    { "matchid", decode_match_id },
    { "pattern", decode_pattern },
    { "flowopt", decode_flowopt },
    { "previous_match_tag", decode_previous_tag },
    { "next_match_tag", decode_next_tag },
    { "action", decode_action},
    { NULL, NULL },
};

static regex_xml_data_t *regex_cmd_get_xml_tag(char *name)
{
    regex_xml_data_t *ptag = regex_match_info;
    char *s = name;

    while(*s) {
        if (*s == '>') {
            *s = '\0';
        }
        s++;
    }

    while (ptag->tag_name) {
        if (!sal_strcasecmp(name, ptag->tag_name)) {
            return ptag;
        }
        ptag++;
    }
    return NULL;
}

/* each input line should be in the format:
 *  /pattern/id/action/
 */
static int _parse_a_xml_line(char *l, int len, 
                             regex_app_info_t *app,
                             regex_xml_data_t **pptag, int *tag_start)
{
    char *b;
    regex_xml_data_t *ptag;

    b = _strip_front(l, len);
    if (!b) { 
        return 0;
    }

    /* check if the line is commented */
    if (b[0] == '#') {
        return 0; /* line is commenetd */
    }

    if (b[0] == '<') {
        b++;
        *tag_start = 1;
        if (*b == '/') {
            *tag_start = 0;
            b++;
        }
        ptag = regex_cmd_get_xml_tag(b);
        if (!ptag) {
            printk("Unknown XML tag: %s\n", b);
            return -1;
        }
        *pptag = ptag;
        return 0;
    }

    ptag = *pptag;
    if (!ptag) {
        printk("Invalid tag");
        return -1;
    }

    /* decode the line accouding to provided tag */
    if (ptag->decode_value && (ptag->decode_value(b, app))) {
        printk("Failed to decode the line: %s\n", b);
        return -1;
    }

    return 0;
}

static cmd_result_t
cmd_regex_install(int unit, args_t *a)
{
    FILE                *fp;
    char                line[512];
    int                 llen, rv;
    xd_t                xd;
    regex_app_info_t    apps[64], *papp;
    bcm_regex_match_t   match[64];
    int                 num_match = 0;
    regex_xml_data_t *ptag;
    int              tag1, port, i, line_num = 0;
    bcm_regex_config_t  recfg;
    bcm_regex_engine_t  reid;
    bcm_regex_engine_config_t eng_cfg;

    if (!SOC_UNIT_VALID(unit)) {
        return (CMD_FAIL);
    }
    sal_memset(&xd, 0, sizeof(xd_t));
    if (CMD_OK != (rv = regex_parse_args(unit, a, &xd))) {
        return(rv);
    }

    if (!xd.xd_file) {
        return CMD_FAIL;
    }

    if ((fp = sal_fopen(xd.xd_file, "r")) == NULL) {
        return CMD_FAIL;
    }

    while(1) {
        /* parse the regex patterns from the file */
        line_num++;
        if (_read_line(fp, line, &llen)) {
            break;
        }
        if (_parse_a_xml_line(line, llen, &apps[num_match], &ptag,
                              &tag1)) {
            printk("Error on line : %d\n", line_num);
            return CMD_FAIL;
        }
        if (ptag && (!sal_strcasecmp(ptag->tag_name, "signature")) &&
            tag1 == 0) {
            num_match++;
            ptag = NULL;
            tag1 = 1;
        }
    }

    /* Set default configuration */
    recfg.flags = BCM_REGEX_CONFIG_ENABLE | BCM_REGEX_CONFIG_IP4 |
                  BCM_REGEX_CONFIG_IP6 | BCM_REGEX_REPORT_MATCHED;
    recfg.max_flows = -1; /* max flows */
    recfg.payload_depth = -1;
    recfg.inspect_num_pkt = -1;
    recfg.inactivity_timeout_usec = 100;
    recfg.dst_mac[0] = 0x00;
    for (i=1;i<6;i++) {
        recfg.dst_mac[i] = 0xa0 + i;
    }
    recfg.src_mac[0] = 0x00;
    for (i=1;i<6;i++) {
        recfg.src_mac[i] = 0xb0 + i;
    }
    recfg.ethertype = 0x0531;
    rv = bcm_regex_config_set(unit, &recfg);
    if (rv) {
        return CMD_FAIL;
    }

    /* Enable regex on the ports */
    BCM_PBMP_ITER(xd.pbmp, port) {
        bcm_port_control_set(unit, port, bcmPortControlRegex, 1);
    }

    /* create an engine */
    eng_cfg.flags = 0; /* FIXME */
    rv = bcm_regex_engine_create(unit, &eng_cfg, &reid);
    if (rv) {
        printk("Failed to create regex engine : %d", rv);
        return CMD_FAIL;
    }
    
    /* Install the patterens */
    for (i = 0; i < num_match; i++) {
        sal_memcpy(&match[i], &apps[i].match, sizeof(bcm_regex_match_t));
    }
    rv = bcm_regex_match_set(unit, reid, num_match, match);
    if (rv) {
        printk("Failed to install patterns : %d", rv);
        return CMD_FAIL;
    }

    /* store the match objects */
    for (i = 0; i< num_match; i++) {
        papp = sal_alloc(sizeof(regex_app_info_t), "apps info");
        sal_memcpy(papp, &apps[i], sizeof(regex_app_info_t));
        papp->engine = reid;
        papp->next = NULL;
        /* add to list  to corelate the event notifications */
        papp->next = app_list[unit];
        app_list[unit] = papp;
    }

    if (xd.xd_file) {
        sal_free(xd.xd_file);
    }
    return (CMD_OK);
}

static int _regex_remove_apps_for_engine(int unit, bcm_regex_engine_t engid)
{
    int rv;
    regex_app_info_t **ppapp, *papp;

    rv = bcm_regex_engine_destroy(unit, engid);
    if (rv) {
        printk("Failed to destroy engine: %d\n", engid);
        return CMD_FAIL;
    }
    /* remove all the apps installed on this engine */
    ppapp = &app_list[unit];

    while(*ppapp) {
        papp = *ppapp;
        if (papp->engine == engid) {
            *ppapp = papp->next;
            sal_free(papp);
        }
    }
    return CMD_OK;
}

static cmd_result_t
cmd_regex_cleardb(int unit)
{
    while (app_list[unit]) {
        _regex_remove_apps_for_engine(unit, app_list[unit]->engine);
    }
    return CMD_OK;
}

static cmd_result_t
cmd_regex_monitor(int unit, args_t *a)
{
    /* FIXME : start the application monitoring thread/task */
    return CMD_OK;
}

static int regex_cmd_engine_dump(int unit, 
                                 bcm_regex_engine_t engine, 
                                 bcm_regex_engine_config_t *config, 
                                 void *user_data)
{
    printk("------------------------------------------\n");
    printk("\tID: 0x%x\n", engine);
    if (config->flags & BCM_REGEX_ENGINE_CONFIG_MULTI_PACKET) {
        printk("\tMode : Multipacket");
    }
    printk("------------------------------------------\n");
    return 0;
}

static int dump_regex(int unit)
{
    bcm_regex_config_t recfg;
    int rv;

    rv = bcm_regex_config_get(unit, &recfg);
    if (!rv) {
        return CMD_FAIL;
    }

    /* display configuration */
    if (recfg.flags & BCM_REGEX_CONFIG_ENABLE) {
        printk("Regex : Enabled\n");
    } else {
        printk("Regex : Disabled\n");
        return CMD_OK;
    }

    /* iterate over all the engines and display their configuration */
    bcm_regex_engine_traverse(unit, regex_cmd_engine_dump, NULL);
    return CMD_OK;
}

cmd_result_t cmd_regex(int unit, args_t *a)
{
    char		*subcmd;

    if ((subcmd = ARG_GET(a)) == NULL) {
	return CMD_USAGE;
    }

    if (sal_strcasecmp(subcmd, "install") == 0) {
        return cmd_regex_install(unit, a);
    } else if (sal_strcasecmp(subcmd, "cleardb") == 0) {
        return cmd_regex_cleardb(unit);
    } else if (sal_strcasecmp(subcmd, "monitor") == 0) {
        return cmd_regex_monitor(unit, a);
    } else if (sal_strcasecmp(subcmd, "show") == 0) {
        return dump_regex(unit);
    } else {
        return dump_regex(unit);
    }
    return CMD_USAGE;
}
#else
int _diag_esw_regex_not_empty;
#endif

