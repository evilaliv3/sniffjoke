/*
 *   SniffJoke is a software able to confuse the Internet traffic analysis,
 *   developed with the aim to improve digital privacy in communications and
 *   to show and test some securiy weakness in traffic analysis software.
 *
 *   Copyright (C) 2011, 2010 vecna <vecna@delirandom.net>
 *                            evilaliv3 <giovanni.pellerano@evilaliv3.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SJ_CONF_H
#define SJ_CONF_H

#include "Utils.h"
#include "PortConf.h"
#include "IPList.h"
#include "SessionTrack.h"
#include "TTLFocus.h"
#include "HDRoptions.h"
#include "internalProtocol.h"

#include <net/ethernet.h>

struct sj_cmdline_opts
{
    /* these date are not present in sj_config because the
     * effective configuration directory is generated as private
     * data inside UserConf classes */
    char basedir[MEDIUMBUF];
    char location[MEDIUMBUF];

    /* START OF COMMON PART WITH sj_config THAT WILL BE SAVED IN CONF FILE */
    char admin_address[MEDIUMBUF];
    uint16_t admin_port;
    char janus_address[MEDIUMBUF];
    uint16_t janus_portin;
    uint16_t janus_portout;
    bool no_tcp;
    bool no_udp;
    bool chaining;
    bool use_whitelist;
    bool use_blacklist;
    bool active;
    bool go_foreground;
    bool dump_packets;
    uint16_t debug_level;
    char onlyplugin[MEDIUMBUF];
    uint16_t max_ttl_probe;
    /* END OF COMMON PART WITH sj_config THAT WILL BE SAVED IN CONF FILE */

    bool force_restart;
};

/* this is the struct keeping the sniffjoke variables, is loaded
 * by the configuration file, when a command line option is specified
 * the command line override the loaded data. the data here present will
 * be dumped overriding the previous config file
 */
struct sj_config
{
    float MAGIC; /* integrity check for saved binary configuration */
    char version[SMALLBUF]; /* SW_VERSION from hardcoded-defines.h */

    char base_dir[LARGEBUF];
    char working_dir[LARGEBUF];
    char pidabspath[LARGEBUF];
    char location[MEDIUMBUF];

    /* START OF COMMON PART WITH sj_cmdline_opts THAT WILL BE SAVED IN CONF FILE */
    char admin_address[MEDIUMBUF];
    uint16_t admin_port;
    char janus_address[MEDIUMBUF];
    uint16_t janus_portin;
    uint16_t janus_portout;
    bool no_tcp;
    bool no_udp;
    bool chaining;
    bool use_whitelist;
    bool use_blacklist;
    bool active;
    bool go_foreground;
    bool dump_packets;
    uint16_t debug_level;
    char onlyplugin[MEDIUMBUF];
    uint16_t max_ttl_probe;
    /* END OF COMMON PART WITH sj_cmdline_opts THAT WILL BE SAVED IN CONF FILE */

    /* mangling policies */
    uint16_t portconf[PORTSNUMBER];
    IPListMap *whitelist;
    IPListMap *blacklist;
};

class UserConf
{
private:

    void parseOnlyParam(const char*);

    /* config file load/dump support*/
    bool parseKeyword(FILE *, char *, const char *);
    void parseMatch(char *, const char *, FILE *, const char *, const char *);
    void parseMatch(uint16_t &, const char *, FILE *, uint16_t, uint16_t);
    void parseMatch(bool &, const char *, FILE *, bool, bool);
    uint32_t dumpIfPresent(FILE *, const char *, char *, const char *);
    uint32_t dumpIfPresent(FILE *, const char *, uint16_t, uint16_t);
    uint32_t dumpIfPresent(FILE *, const char *, bool, bool);

    /* import of the file containing the port range settings, and load the
     * default if not present, it use portLine class in portConfParsing.cc */
    void loadAggressivity(void);
    void initOptionLoader(const char *);
    void delOptionLoader();

    bool syncPortsFiles(void);
    bool syncIPListsFiles(void);

public:
    const struct sj_cmdline_opts &cmdline_opts;
    struct sj_config runcfg; /* the running configuration is accessible by other class */

    UserConf(const struct sj_cmdline_opts &);
    ~UserConf(void);

    /* call all the private method for setup networking -- in future need to be modify
     * for multi OS supports */
    void networkSetup(void);

    /* config file load/dump interface*/
    bool loadDiskConfiguration(void);
    bool syncDiskConfiguration(void);
};

#endif /* SJ_CONF_H */
