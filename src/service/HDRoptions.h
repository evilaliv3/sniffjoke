/*
 * SniffJoke is a software able to confuse the Internet traffic analysis,
 * developed with the aim to improve digital privacy in communications and
 * to show and test some securiy weakness in traffic analysis software.
 *    
 *      Copyright (C) 2010 vecna <vecna@delirandom.net>
 *                         evilaliv3 <giovanni.pellerano@evilaliv3.org>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HDROPTIONS_H
#define HDROPTIONS_H

#include "Packet.h"

#include <netinet/ip.h>
#include <netinet/tcp.h>

enum injector_t
{
    IPOPTS_INJECTOR = 0, TCPOPTS_INJECTOR = 1
};

class HDRoptions
{
private:
    injector_t type;
    bool corrupt;
    unsigned char *optptr;
    uint8_t &actual_opts_len; /* max value 40 on IP and TCP too */
    uint8_t &target_opts_len; /* max value 40 on IP and TCP too */
    uint8_t available_opts_len; /* max value 40 on IP and TCP too */

    /*
     * options we need to check the presence for;
     * some options are good but if repeated may corrupt the packet.
     */
    bool opt_ip_timestamp;
    bool opt_ip_rr;

    uint8_t m_IPOPT_NOOP(void);
    uint8_t m_IPOPT_TIMESTAMP(void);
    uint8_t m_IPOPT_LSRR(void);
    uint8_t m_IPOPT_RR(void);
    uint8_t m_IPOPT_RA(void);
    uint8_t m_IPOPT_CIPSO(void);
    uint8_t m_IPOPT_SEC(void);
    uint8_t m_IPOPT_SID(void);

    uint8_t m_TCPOPT_EOL(void);
    uint8_t m_TCPOPT_NOP(void);
    uint8_t m_TCPOPT_TIMESTAMP(void);
    uint8_t m_TCPOPT_MAXSEG(void);
    uint8_t m_TCPOPT_WINDOW(void);
    uint8_t m_TCPOPT_SACK_PERMITTED(void);
    uint8_t m_TCPOPT_SACK(void);

    bool checkIPOPTINJPossibility(void);
    bool checkTCPOPTINJPossibility(void);

public:
    /* used for internal definition of IP opt functions */
#define SJ_IPOPT_NOOP            0
#define SJ_IPOPT_TIMESTAMP        1
#define SJ_IPOPT_LSRR            2
#define SJ_IPOPT_RR            3
#define SJ_IPOPT_RA            4
#define SJ_IPOPT_CIPSO            5
#define SJ_IPOPT_SEC            6
#define SJ_IPOPT_SID            7

#define SJ_TCPOPT_EOL            0
#define SJ_TCPOPT_NOP            1
#define SJ_TCPOPT_TIMESTAMP        2
#define SJ_TCPOPT_MAXSEG        3
#define SJ_TCPOPT_WINDOW        4
#define SJ_TCPOPT_SACK_PERMITTED    5
#define SJ_TCPOPT_SACK            6

    /* used for internal definition of TCP opt functions */

    HDRoptions(injector_t, bool, unsigned char *, uint8_t &, uint8_t &);
    bool randomInjector();
};

#endif /* HDROPTIONS_H */