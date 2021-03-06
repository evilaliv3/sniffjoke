/*
 *   SniffJoke is a software able to confuse the Internet traffic analysis,
 *   developed with the aim to improve digital privacy in communications and
 *   to show and test some securiy weakness in traffic analysis software.
 *   
 *   Copyright (C) 2010 vecna <vecna@delirandom.net>
 *                      evilaliv3 <giovanni.pellerano@evilaliv3.org>
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

#ifndef SJ_PROCESS_H
#define SJ_PROCESS_H

#include "Utils.h"
#include "UserConf.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

class Process
{
private:
    sigset_t sig_nset;
    sigset_t sig_oset;

public:
    Process(void);
    ~Process(void);

    pid_t readPidfile(void);
    void writePidfile(void);
    void unlinkPidfile(void);

    void sigtrapSetup(void);
    void sigtrapEnable(void);
    void sigtrapDisable(void);

    void changedir(void);
    void background(void);
    void isolation(void);
};

#endif /* SJ_PROCESS_H */
