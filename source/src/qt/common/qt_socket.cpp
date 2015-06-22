/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.27 -

	[ Qt socket ]
*/

#include "emu.h"
#include "vm/vm.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

void EMU::initialize_socket()
{
#if 0
	// init winsock
	WSADATA wsaData;
	WSAStartup(0x0101, &wsaData);
	
	// init sockets
	for(int i = 0; i < SOCKET_MAX; i++) {
		soc[i] = INVALID_SOCKET;
		socket_delay[i] = 0;
		recv_r_ptr[i] = recv_w_ptr[i] = 0;
	}
#else
	for(int i = 0; i < SOCKET_MAX; i++) {
		soc[i] = -1;
		socket_delay[i] = 0;
		recv_r_ptr[i] = recv_w_ptr[i] = 0;
	}
#endif
}

void EMU::release_socket()
{
	// release sockets
#if 0
	for(int i = 0; i < SOCKET_MAX; i++) {
		if(soc[i] != INVALID_SOCKET) {
			shutdown(soc[i], 2);
			closesocket(soc[i]);
		}
	}
	
	// release winsock
	WSACleanup();
#endif   
}

void EMU::socket_connected(int ch)
{
	// winmain notify that network is connected
	vm->network_connected(ch);
}

void EMU::socket_disconnected(int ch)
{
	// winmain notify that network is disconnected
	if(!socket_delay[ch]) {
		socket_delay[ch] = 1;//56;
	}
}

void EMU::update_socket()
{
	for(int i = 0; i < SOCKET_MAX; i++) {
		if(recv_r_ptr[i] < recv_w_ptr[i]) {
			// get buffer
			int size0, size1;
			uint8* buf0 = vm->get_recvbuffer0(i, &size0, &size1);
			uint8* buf1 = vm->get_recvbuffer1(i);
			
			int size = recv_w_ptr[i] - recv_r_ptr[i];
			if(size > size0 + size1) {
				size = size0 + size1;
			}
			char* src = &recv_buffer[i][recv_r_ptr[i]];
			recv_r_ptr[i] += size;
			
			if(size <= size0) {
				memcpy(buf0, src, size);
			} else {
				memcpy(buf0, src, size0);
				memcpy(buf1, src + size0, size - size0);
			}
			vm->inc_recvbuffer_ptr(i, size);
		} else if(socket_delay[i] != 0) {
			if(--socket_delay[i] == 0) {
				vm->network_disconnected(i);
			}
		}
	}
}

bool EMU::init_socket_tcp(int ch)
{
	is_tcp[ch] = true;
	
#if 0
	if(soc[ch] != INVALID_SOCKET) {
		disconnect_socket(ch);
	}
	if((soc[ch] = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		return false;
	}
	if(WSAAsyncSelect(soc[ch], main_window_handle, WM_SOCKET0 + ch, FD_CONNECT | FD_WRITE | FD_READ | FD_CLOSE) == SOCKET_ERROR) {
		closesocket(soc[ch]);
		soc[ch] = INVALID_SOCKET;
		return false;
	}
	return true;
#endif
	soc[ch] = -1;
	recv_r_ptr[ch] = recv_w_ptr[ch] = 0;
	return false;
}

bool EMU::init_socket_udp(int ch)
{
	is_tcp[ch] = false;
	
#if 0
	disconnect_socket(ch);
	if((soc[ch] = socket(PF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
		return false;
	}
	if(WSAAsyncSelect(soc[ch], main_window_handle, WM_SOCKET0 + ch, FD_CONNECT | FD_WRITE | FD_READ | FD_CLOSE) == SOCKET_ERROR) {
		closesocket(soc[ch]);
		soc[ch] = INVALID_SOCKET;
		return false;
	}
	return true;
#endif
	soc[ch] = -1;
	recv_r_ptr[ch] = recv_w_ptr[ch] = 0;
	return false;
}

bool EMU::connect_socket(int ch, uint32 ipaddr, int port)
{
#if 0
	struct sockaddr_in tcpaddr;
	tcpaddr.sin_family = AF_INET;
	tcpaddr.sin_addr.s_addr = ipaddr;
	tcpaddr.sin_port = htons((unsigned short)port);
	memset(tcpaddr.sin_zero, (int)0, sizeof(tcpaddr.sin_zero));
	
	if(connect(soc[ch], (struct sockaddr *)&tcpaddr, sizeof(tcpaddr)) == SOCKET_ERROR) {
		if(WSAGetLastError() != WSAEWOULDBLOCK) {
			return false;
		}
	}
	return true;
#endif   
	return true;
}

void EMU::disconnect_socket(int ch)
{
#if 0
	if(soc[ch] != INVALID_SOCKET) {
		shutdown(soc[ch], 2);
		closesocket(soc[ch]);
		soc[ch] = INVALID_SOCKET;
	}
#endif
	soc[ch] = -1;
	vm->network_disconnected(ch);
}

bool EMU::listen_socket(int ch)
{
	return false;
}

void EMU::send_data_tcp(int ch)
{
#if 0
	if(is_tcp[ch]) {
		send_data(ch);
	}
#endif
}

void EMU::send_data_udp(int ch, uint32 ipaddr, int port)
{
#if 0
	if(!is_tcp[ch]) {
		udpaddr[ch].sin_family = AF_INET;
		udpaddr[ch].sin_addr.s_addr = ipaddr;
		udpaddr[ch].sin_port = htons((unsigned short)port);
		memset(udpaddr[ch].sin_zero, (int)0, sizeof(udpaddr[ch].sin_zero));
		
		send_data(ch);
	}
#endif
}

void EMU::send_data(int ch)
{
#if 0
	// loop while send buffer is not emupty or not WSAEWOULDBLOCK
	while(1) {
		// get send buffer and data size
		int size;
		uint8* buf = vm->get_sendbuffer(ch, &size);
		
		if(!size) {
			return;
		}
		if(is_tcp[ch]) {
			if((size = send(soc[ch], (char *)buf, size, 0)) == SOCKET_ERROR) {
				// if WSAEWOULDBLOCK, WM_SOCKET* and FD_WRITE will come later
				if(WSAGetLastError() != WSAEWOULDBLOCK) {
					disconnect_socket(ch);
					socket_disconnected(ch);
				}
				return;
			}
		} else {
			if((size = sendto(soc[ch], (char *)buf, size, 0, (struct sockaddr *)&udpaddr[ch], sizeof(udpaddr[ch]))) == SOCKET_ERROR) {
				// if WSAEWOULDBLOCK, WM_SOCKET* and FD_WRITE will come later
				if(WSAGetLastError() != WSAEWOULDBLOCK) {
					disconnect_socket(ch);
					socket_disconnected(ch);
				}
				return;
			}
		}
		vm->inc_sendbuffer_ptr(ch, size);
	}
#endif
}

void EMU::recv_data(int ch)
{
#if 0
	if(is_tcp[ch]) {
		int size = SOCKET_BUFFER_MAX - recv_w_ptr[ch];
		char* buf = &recv_buffer[ch][recv_w_ptr[ch]];
		if((size = recv(soc[ch], buf, size, 0)) == SOCKET_ERROR) {
			disconnect_socket(ch);
			socket_disconnected(ch);
			return;
		}
		recv_w_ptr[ch] += size;
	} else {
		SOCKADDR_IN addr;
		int len = sizeof(addr);
		int size = SOCKET_BUFFER_MAX - recv_w_ptr[ch];
		char* buf = &recv_buffer[ch][recv_w_ptr[ch]];
		
		if(size < 8) {
			return;
		}
		if((size = recvfrom(soc[ch], buf + 8, size - 8, 0, (struct sockaddr *)&addr, &len)) == SOCKET_ERROR) {
			disconnect_socket(ch);
			socket_disconnected(ch);
			return;
		}
		size += 8;
		buf[0] = size >> 8;
		buf[1] = size;
		buf[2] = (char)addr.sin_addr.s_addr;
		buf[3] = (char)(addr.sin_addr.s_addr >> 8);
		buf[4] = (char)(addr.sin_addr.s_addr >> 16);
		buf[5] = (char)(addr.sin_addr.s_addr >> 24);
		buf[6] = (char)addr.sin_port;
		buf[7] = (char)(addr.sin_port >> 8);
		recv_w_ptr[ch] += size;
	}
#endif
}
