#ifndef SCRAMBLEMASK_H
#define SCRAMBLEMASK_H

/* this is the base enum used for keep track of which scramble has been used/requested */
enum scramble_t
{   /* -- Scramble.h has to be updated whenever a new value here is addedd -- */
    NOSCRAMBLESET = 0, INNOC = 1, TTL = 2, IPTCPOPT = 4, CKSUM = 8, FINRST = 16, FRAGMENT = 32, TCPOVERLAP = 64
};

#define NO_ONE_SCRAMBLE "NO_SCRAMBLE_SET"
#define INN_SCRAMBLE_N "INNOCENT"
#define TTL_SCRAMBLE_N  "TTL"
#define OPT_SCRAMBLE_N  "IPTCPOPT"
#define CKS_SCRAMBLE_N  "CHECKSUM"
#define FNR_SCRAMBLE_N  "FINRST"
#define FRAG_SCRAMBLE_N "IPFRAGMENT"
#define TCPO_SCRAMBLE_N "TCPOVERLAP"

/* the scramble_t is defined in Packet.h */
struct implementedScramble {
    const char *keyword;
    scramble_t scrambleBit;
    // TODO if the scramble will corrupt, mistyfy, be good or whatever, must be written here; 
    // this data will be used by "willCorrupt" method
};

#define SCRAMBLE_SUPPORTED  7

/* a global variable called by PluginPool.cc and [fixme] */
const struct implementedScramble sjImplementedScramble[SCRAMBLE_SUPPORTED] =  {
    { INN_SCRAMBLE_N, INNOC }, 
    { TTL_SCRAMBLE_N, TTL },
    { OPT_SCRAMBLE_N, IPTCPOPT },
    { CKS_SCRAMBLE_N, CKSUM },
    { FNR_SCRAMBLE_N, FINRST },
    { FRAG_SCRAMBLE_N, FRAGMENT },
    { TCPO_SCRAMBLE_N, TCPOVERLAP }
};


class scrambleMask
{
private:
    static char scrambleList[ SCRAMBLE_SUPPORTED * 14 ];

public:
    uint8_t innerMask;

    scrambleMask & operator+=(const scramble_t);
    scrambleMask & operator-=(const scramble_t);
    scrambleMask & operator=(const scramble_t);

    scrambleMask & operator+=(const scrambleMask);
    scrambleMask & operator-=(const scrambleMask);
    scrambleMask & operator=(const scrambleMask);

    bool operator!(void);

    const scrambleMask getShared(const scrambleMask &);
    bool isScrambleSet(const scramble_t);
    bool willCorrupt(void) const;

    const char *debug(void) const;

    scrambleMask(void);
    scrambleMask(scramble_t);
    scrambleMask(const scrambleMask &);

    ~scrambleMask(void);
};

#endif // SCRAMBLE_MASK_H
