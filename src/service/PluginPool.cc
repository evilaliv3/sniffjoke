/*'
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

#include "PluginPool.h"
#include "UserConf.h"
#include "Scramble.h"

#include <dlfcn.h>

extern auto_ptr<UserConf> userconf;

PluginTrack::PluginTrack(const char *plugpath, scrambleMask &pluginSupportedScr, char *plugOpt)
{
    LOG_VERBOSE("constructor %s to %s option [%s]", __func__, plugpath, plugOpt);

    void *swapPtr;

    pluginHandler = dlopen(plugpath, RTLD_NOW);
    if (pluginHandler == NULL)
        RUNTIME_EXCEPTION("unable to load plugin %s: %s", plugpath, dlerror());

    /* 
     * Coder: we had used 
     * http://www.opengroup.org/onlinepubs/009695399/functions/dlsym.html as reference
     * for use dlsym() insice C++, but if you serch in the web the following string:
     *
     * ISO C++ forbids casting between pointer-to-function and pointer-to-object
     *
     * you will understand because the old way:
     * fp_DeletePluginObj = (destructor_f *) dlsym(pluginHandler, "deletePluginObj");
     * has been substituted at the moment with the less safe, but without warning,
     * usage of memcpy between the pointer returned by dlsym and the function pointer (fp_Blah)
     */

    swapPtr = forcedSymbolCopy("createPluginObj", plugpath);
    memcpy( (void *)&fp_CreatePluginObj, &swapPtr, sizeof(void *));

    swapPtr = forcedSymbolCopy("deletePluginObj", plugpath);
    memcpy( (void *)&fp_DeletePluginObj, &swapPtr, sizeof(void *));

    swapPtr = forcedSymbolCopy("versionValue", plugpath);
    memcpy( (void *)&fp_versionValue, &swapPtr, sizeof(void *));


    if (strlen(fp_versionValue()) != strlen(SW_VERSION) || strcmp(fp_versionValue(), SW_VERSION))
    {
        RUNTIME_EXCEPTION("loading %s incorred version (%s) with SniffJoke %s",
                          plugpath, fp_versionValue(), SW_VERSION);
    }

    selfObj = fp_CreatePluginObj();

    if (selfObj->pluginName == NULL)
        RUNTIME_EXCEPTION("Invalid implementation: %s lack of ->PluginName member", plugpath);

    configuredScramble = pluginSupportedScr;

    if(plugOpt != NULL)
        declaredOpt = strdup(plugOpt);
    else
        declaredOpt = NULL;

    LOG_ALL("Loading of %s: %s, scramble sets %s, acquired option [%s]",
            plugpath, selfObj->pluginName, configuredScramble.debug(), 
            plugOpt != NULL ? plugOpt : "NONE"
            );
}

void *PluginTrack::forcedSymbolCopy( const char *symName, const char *pap)
{
    void *obtainPtr = dlsym(pluginHandler, symName);

    if (obtainPtr == NULL )
        RUNTIME_EXCEPTION("plugin %s lack of the symbol %s mangling symbols", pap, symName);

    return obtainPtr;
}

void PluginPool::initializeAll(struct sjEnviron *autoptrList)
{
    uint32_t counter = 1;
    for (vector<PluginTrack *>::iterator it = pool.begin(); it != pool.end(); ++it)
    {
        PluginTrack* const plugin = *it;
        bool initval;

        initval = plugin->selfObj->init(plugin->configuredScramble, plugin->declaredOpt, autoptrList);

        if(initval == false)
        {
            RUNTIME_EXCEPTION("Unable to init %s whitin the current configuration context: scramble %s opt [%s]", 
                              plugin->selfObj->pluginName, 
                              plugin->configuredScramble.debug(), 
                              plugin->declaredOpt != NULL ? plugin->declaredOpt : "/" );
        }

        LOG_DEBUG("%d) Initialized %s successfull with complete configuration context: scramble %s opt [%s]", 
                  counter, plugin->selfObj->pluginName, 
                  plugin->configuredScramble.debug(), 
                  plugin->declaredOpt != NULL ? plugin->declaredOpt : "/" );

        counter++;
    }
}

/*
 * the constructor of PluginPool is called once; in the TCPTrack constructor the class member
 * plugin_pool is instanced. what we need here is to read the entire plugin list, open and fix the
 * list, keeping track in listOfPlugin variable
 *
 *    plugin_pool()
 *
 * (class TCPTrack).plugin_pool is the name of the unique PluginPool element
 */
PluginPool::PluginPool(void) :
globalEnabledScrambles()
{
    /* globalEnabledScrambles is set from the sum of each plugin configuration */
    if (userconf->runcfg.onlyplugin[0])
        parseOnlyPlugin();
    else
        parseEnablerFile();

    if (!pool.size())
        RUNTIME_EXCEPTION("fatal error: loaded correctly 0 plugins");
    else
        LOG_ALL("loaded correctly %d plugins", pool.size());

    LOG_ALL("Globally enabled scrambles: [%s]", globalEnabledScrambles.debug());
    LOG_ALL("SniffJoke will use this configuration to create confusion also on real packets");
}

PluginPool::~PluginPool(void)
{
    LOG_DEBUG("");

    /* call the distructor loaded from the plugins */
    for (vector<PluginTrack *>::iterator it = pool.begin(); it != pool.end(); it = pool.erase(it))
    {
        const PluginTrack *plugin = *it;

        LOG_DEBUG("calling %s destructor and closing plugin handler", plugin->selfObj->pluginName);

        plugin->fp_DeletePluginObj(plugin->selfObj);

        dlclose(plugin->pluginHandler);

        if(plugin->declaredOpt != NULL)
            free(plugin->declaredOpt);

        delete plugin;
    }
}

void PluginPool::importPlugin(const char *plugpath, scrambleMask &configuredScramble, char *pOpt)
{
    pool.push_back(new PluginTrack(plugpath, configuredScramble, pOpt));
}

bool PluginPool::parseScrambleOpt(char *list_str, scrambleMask *retval, char **opt)
{
    bool foundScramble = false;
    char copyStr[MEDIUMBUF] = {0};
    char *optParse = NULL;

    memcpy(copyStr, list_str, strlen(list_str));

    /* check if the option is used, the char used for separation is '+' 
     * optParse and copyStr are used for sanity check ONLY */
    if ((optParse = strchr(copyStr, '+')) != NULL)
    {
        (*optParse) = 0x00;
        (optParse)++;

        if (*optParse == 0x00)
        {
            LOG_ALL("no valid option passed after the control char '+': %s", list_str);
            return false;
        }

        /* no other symbol are accepted */
        if (!isalnum(*optParse))
        {
            LOG_ALL("invalid char after '+' only alphanumeric and digit accepted: %s", list_str);
            return false;
        }

        /* const assigment */
        (*opt) = strchr(list_str, '+');
        ++(*opt);
    }

    /*   the plugin_enable.conf.$LOCATION file has this format:
     *   plugin.so,SCRAMBLE1[,SCRAMBLE2][,SCRAMBLE3]         */
    for (uint32_t i = 0; i < SCRAMBLE_SUPPORTED; i++)
    {
        printf("%s %s\n", copyStr, sjImplementedScramble[i].keyword);
        if (strstr(copyStr, sjImplementedScramble[i].keyword))
        {
            foundScramble = true;
            (*retval) += sjImplementedScramble[i].scrambleBit;
        }
    }

    return foundScramble;
}

void PluginPool::parseOnlyPlugin(void)
{
    LOG_VERBOSE("onlyplugin [%s] forced to be applied ALWAYS", userconf->runcfg.onlyplugin);

    char *comma;
    char *pluginOpt = NULL;
    char onlyplugin_cpy[MEDIUMBUF] = {0};
    char plugpath[MEDIUMBUF] = {0};
    scrambleMask pluginEnabledScrambles;

    snprintf(onlyplugin_cpy, sizeof (onlyplugin_cpy), "%s", userconf->runcfg.onlyplugin);

    if ((comma = strchr(onlyplugin_cpy, ',')) == NULL)
        RUNTIME_EXCEPTION("invalid use of --only-plugin: (%s)", userconf->runcfg.onlyplugin);

    *comma = 0x00;
    comma++;

    snprintf(plugpath, sizeof (plugpath), "%s/plugins/%s.so", userconf->runcfg.base_dir, onlyplugin_cpy);

    if (!parseScrambleOpt(comma, &pluginEnabledScrambles, &pluginOpt))
        RUNTIME_EXCEPTION("invalid use of --only-plugin: (%s)", userconf->runcfg.onlyplugin);

    importPlugin(plugpath, pluginEnabledScrambles, pluginOpt);

    /* we keep track of enabled scramble to apply confusion on real good packets */
    globalEnabledScrambles += pluginEnabledScrambles;
}

void PluginPool::parseEnablerFile(void)
{
    char plugpath[MEDIUMBUF] = {0};
    char enablerentry[LARGEBUF] = {0};

    FILE *plugfile = fopen(FILE_PLUGINSENABLER, "r");
    if (plugfile == NULL)
        RUNTIME_EXCEPTION("unable to open in reading %s: %s", FILE_PLUGINSENABLER, strerror(errno));

    uint8_t line = 0;
    do
    {
        char *comma;
        char *pluginOpt = NULL;
        scrambleMask enabledScrambles;

        if( fgets(enablerentry, LARGEBUF, plugfile) == NULL)
            break;

        ++line;

        if (enablerentry[0] == '#' || enablerentry[0] == '\n' || enablerentry[0] == ' ')
            continue;

        /* C's chop() */
        enablerentry[strlen(enablerentry) - 1] = 0x00;

        /* 11 is the minimum length of a ?.so plugin, comma and strlen("GUILTY") the shortest keyword */
        if (strlen(enablerentry) < 11 || feof(plugfile))
        {
            RUNTIME_EXCEPTION("reading %s: imported %d plugins, matched interruption at line %d",
                              FILE_PLUGINSENABLER, pool.size(), line);
        }

        /* parsing of the file line, finding the first comma and make it a 0x00 */
        if ((comma = strchr(enablerentry, ',')) == NULL)
        {
            RUNTIME_EXCEPTION("reading %s at line %d lack the comma separator for scramble selection",
                              FILE_PLUGINSENABLER, line);
        }

        /* name,SCRAMBLE became name[NULL]SCRAMBLE, *comma point to "S" */
        *comma = 0x00;
        comma++;

        snprintf(plugpath, sizeof (plugpath), "%splugins/%s.so", userconf->runcfg.base_dir, enablerentry);

        if (!parseScrambleOpt(comma, &enabledScrambles, &pluginOpt))
        {
            RUNTIME_EXCEPTION("in line %d (%s), no valid scramble are present in %s",
                              line, enablerentry, FILE_PLUGINSENABLER);
        }

        LOG_VERBOSE("importing plugin [%s] enabled scrambles %s", enablerentry, enabledScrambles.debug());
        importPlugin(plugpath, enabledScrambles, pluginOpt);

        /* we keep track of enabled scramble to apply confusion on real good packets */
        globalEnabledScrambles += enabledScrambles;

    }
    while (!feof(plugfile));

    fclose(plugfile);
}
