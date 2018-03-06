#include <winsock2.h>
#include <iostream>

class InitWinSocket {
public:
	//в╟ть
	InitWinSocket() {
		WORD wVersionRequested;
		WSADATA wsaData;
        wVersionRequested = MAKEWORD(2, 2);
        		int err = WSAStartup(wVersionRequested, &wsaData);
		if (err != 0) {
			std::cout << "Init WinSock Failed!" << std::endl;
		}
		if (LOBYTE( wsaData.wVersion) != 2 || HIBYTE( wsaData.wVersion) != 2) {
			std::cout << "Init WinSock Failed!" << std::endl;
			WSACleanup();
        }

    }
    //п╤ть
    ~InitWinSocket(){
      WSACleanup();
    }
};