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

/*
 * HACK COMMENT:, every hacks require intensive comments because should cause 
 * malfunction, or KILL THE INTERNET :)
 *
 * I didn't rembemer exactly, zero window TCP packet are used for stop the 
 * communication until resume is requested
 * 
 * SOURCE: deduction, whishful thinking
 * VERIFIED IN:
 * KNOW BUGS:
 * WRITTEN IN VERSION: 0.4.0
 */

#include "Packet.h"

class fake_zero_window : public HackPacket
{
#define HACK_NAME	"Fake 0-window"
public:
	virtual Packet *createHack(Packet &orig_packet)
	{
		orig_packet.selflog(HACK_NAME, "Original packet");
		Packet* ret = new Packet(orig_packet);

		ret->resizePayload(0);

		ret->tcp->syn = ret->tcp->fin = ret->tcp->rst = 0;
		ret->tcp->psh = ret->tcp->ack = 0;
		ret->tcp->window = 0;

		ret->position = ANY_POSITION;
		ret->wtf = RANDOMDAMAGE;

		ret->selflog(HACK_NAME, "Hacked packet");
		return ret;
	}

	fake_zero_window(int plugin_index) {
		track_index = plugin_index;
		hackName = HACK_NAME;
		hack_frequency = 5;
		prescription_probability = 93;
	}

};

extern "C"  HackPacket* CreateHackObject(int plugin_tracking_index) {
	return new fake_zero_window(plugin_tracking_index);
}

extern "C" void DeleteHackObject(HackPacket *who) {
	delete who;
}
