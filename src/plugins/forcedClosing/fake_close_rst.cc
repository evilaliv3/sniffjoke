/*
 *   SniffJoke is a software able to confuse the Internet traffic analysis,
 *   developed with the aim to improve digital privacy in communications and
 *   to show and test some securiy weakness in traffic analysis software.
 *   
 *   Copyright (C) 2010, 2011 vecna <vecna@delirandom.net>
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

/*
 * HACK COMMENT:, every hacks require intensive comments because should cause 
 * malfunction, or KILL THE INTERNET :)
 * 
 * fake close is used because a sniffer could read a FIN like a session closing
 * tcp-flag, and stop the session monitoring/reassembly.
 *
 * SOURCE: phrack, deduction 
 * VERIFIED IN:
 * KNOW BUGS: fastweb network (italian wire provider), should not support this hack 
 * 	      because the HAG close the session at the first valid RST exiting
 * 	      from your box.
 */

#include "service/Plugin.h"

class fake_close_rst : public Plugin
{
private:
    pluginLogHandler pLH;
    PluginCache RSTcache;

#define PLUGIN_NAME "Fake RST"
#define PKT_LOG "plugin.fake_close_rst.log"
#define MIN_INJECTED_PKTS    4
#define MAX_INJECTED_PKTS    10

    /* every hack in "forcedClosing" will be useful "few times in a session", not for the
     * entire duration of the connections: for this reason is kept a cache record to count every
     * time a condition is returned "true"
     *
     * MIN_INJECTED_PKTS mean the minimum packets possibile, between MIN < x < MAX, the probability
     * to be true the condition use an inverted probability, until reach MAX, than will never be
     * injected again.
     *
     * this is implemented in the condition check and the useful generic method are implemented
     * in Plugin class, explanation useful will be found in ../PluginList.txt
     */

public:
    fake_close_rst() :
    Plugin(PLUGIN_NAME, AGG_PACKETS30PEEK),
    pLH(PLUGIN_NAME, PKT_LOG)
    {
    };

    virtual bool init(scrambleMask &configuredScramble, char *pluginOption, struct sjEnviron *sjE)
    {
        if(configuredScramble.willCorrupt())
        {
            supportedScrambles = configuredScramble;
            return true;
        }
        else
        {
            LOG_ALL("Plugin %s will not be loaded. Require some scramble supporting the corruption!", PLUGIN_NAME); 
            LOG_ALL("in the configuration line, the scramble [%s] are not enough. 'man sniffjoke-plugins.conf'",
                   configuredScramble.debug()
            );
            return false;
        }
    }

    virtual bool condition(const Packet &origpkt, scrambleMask & availableScrambles)
    {
        if (origpkt.chainflag == FINALHACK || origpkt.proto != TCP || origpkt.fragment == true)
            return false;

        pLH.completeLog("verifing condition for ip.id %d Sj#%u (dport %u) datalen %d total len %d",
                        ntohs(origpkt.ip->id), origpkt.SjPacketId, ntohs(origpkt.tcp->dest),
                        origpkt.tcppayloadlen, origpkt.pbuf.size());

        /* preliminar condition, TCP and fragment already checked */
        bool ret = (!origpkt.tcp->syn && !origpkt.tcp->rst && !origpkt.tcp->fin);

        if (!ret)
            return false;

        /* corruption is required to be available, perhaps usually will fall in the checksum
         * choosing, but this check is necessary */
        if(!availableScrambles.willCorrupt())
        {
            pLH.completeLog("scramble available don't support the pkts corruption [%s]", 
                            availableScrambles.debug());
            return false;
        }

        /* cache checking, using the methods provide in the section 'forcedClosing' of Plugin.cc */
        cacheRecord* matchRecord;

        if ((matchRecord = verifyIfCache(&(tupleMatch), &RSTcache, origpkt)) != NULL)
        {
            uint32_t *injectedYet = (uint32_t*)&(matchRecord->cached_data[0]);

            /* if is present, inverseProp, return true with decreasing probability up to MAX_INJ */
            ret = inverseProportionality(*injectedYet, MIN_INJECTED_PKTS, MAX_INJECTED_PKTS);

            if (ret)
            {
                ++(*injectedYet);

                pLH.completeLog("packets #%d in session %s:%u Sj.hack %s (min %d max %d)", *injectedYet,
                                inet_ntoa(*((struct in_addr *) &(origpkt.ip->daddr))), ntohs(origpkt.tcp->dest),
                                ret ? "TRUE" : "FALSE", MIN_INJECTED_PKTS, MAX_INJECTED_PKTS);
            }
        }

        return ret;
    }

    virtual void apply(const Packet &origpkt, scrambleMask & availableScrambles)
    {
        Packet * const pkt = new Packet(origpkt);

        pkt->randomizeID();

        pkt->tcp->psh = 0;
        pkt->tcp->rst = 1;
        pkt->tcp->seq = htonl(ntohl(pkt->tcp->seq) - pkt->tcppayloadlen + 1);

        pkt->tcppayloadResize(0);

        pkt->source = PLUGIN;

        pkt->position = ANTICIPATION;

        pkt->wtf = CORRUPTNEED;
    //        pkt->choosableScramble = (availableScrambles & supportedScrambles);

        pkt->chainflag = FINALHACK;

        pktVector.push_back(pkt);
    }

    virtual void mangleIncoming(Packet &inpkt)
    {
    }

};

extern "C" Plugin* createPluginObj()
{
    return new fake_close_rst();
}

extern "C" void deletePluginObj(Plugin *who)
{
    delete (fake_close_rst *) who;
}

extern "C" const char *versionValue()
{
    return SW_VERSION;
}
