/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "SDL_net.h"
#include "common/textconsole.h"
#include "common/str.h"

#include "base/version.h"

namespace Common {

#define kInBufLen 8192
#define kHostName "www.google-analytics.com"

void sendGoogleAnalytics() {
	return;

	if (SDLNet_Init() == -1)
		error("SDLNet_Init: %s", SDLNet_GetError());

	IPaddress ip;

	if (SDLNet_ResolveHost(&ip, kHostName, 80) == -1) {
		warning("Cannot resolve address");

		return;
	}

	TCPsocket tcpsock;

	tcpsock = SDLNet_TCP_Open(&ip);

	if (!tcpsock) {
		warning("SDLNet_TCP_Open: %s\n", SDLNet_GetError());

		return;
	}

	char request[1024];

	snprintf(request, 1024, "GET / HTTP/1.0\n"
			"Host: %s\n"
			"Connection: close\n"
			"User-Agent: ScummVM %s\n"
			"\n", kHostName, gScummVMFullVersion);

	int len = strlen(request) + 1;

	if (SDLNet_TCP_Send(tcpsock, request, len) < len) {
		warning("SDLNet_TCP_Send: %s", SDLNet_GetError());

		return;
	}

	char inBuf[kInBufLen];
	Common::String res;
	int inbytes;

	do {
		inbytes = SDLNet_TCP_Recv(tcpsock, inBuf, kInBufLen);

		res += inBuf;
	} while (inbytes);

	warning("%s", res.c_str());

	SDLNet_TCP_Close(tcpsock);

	SDLNet_Quit();
}

} // End of namespace Common

