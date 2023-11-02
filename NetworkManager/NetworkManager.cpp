#include "NetworkManager.h"
#include <iostream>
#include <WS2tcpip.h>
using namespace std;

NetworkManager* NetworkManager::instance = nullptr;

NetworkManager::NetworkManager()
{
	cout << "NetworkManager Constructed" << endl;
	UDPSocketIn = INVALID_SOCKET;
	UDPSocketOut = INVALID_SOCKET;
	TCPSocketIn = INVALID_SOCKET;
	TCPSocketOut = INVALID_SOCKET;
	TCPSocketClient = INVALID_SOCKET;

	UDPoutAddr = { 0 };
	UDPinAddr = { 0 };

	TCPOutAddr = { 0 };
	TCPInAddr = { 0 };
}

NetworkManager::~NetworkManager() 
{

}

void NetworkManager::Init()
{
	cout << "NetworkManager Initialized" << endl;
	WSADATA lpWSAData;
	int error = WSAStartup(MAKEWORD(2, 2), &lpWSAData);

	if (error != 0)
	{
		cout << "WSAData failed with error code" << error << endl;
	}
}

void NetworkManager::BindUDP()
{
	UDPinAddr.sin_family = AF_INET;
	UDPinAddr.sin_port = htons(8889);
	UDPinAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int bindError = bind(UDPSocketIn, reinterpret_cast<SOCKADDR*>(&UDPinAddr), sizeof(UDPinAddr));

	if (bindError == SOCKET_ERROR)
	{
		cout << "[ERROR] binding failed" << endl;

		Shutdown();
	}
}

void NetworkManager::BindTCP()
{
	TCPInAddr.sin_family = AF_INET;
	TCPInAddr.sin_port = htons(7777);
	TCPInAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int bindError = bind(TCPSocketIn, reinterpret_cast<SOCKADDR*>(&TCPInAddr), sizeof(TCPInAddr));

	if (bindError == SOCKET_ERROR)
	{
		cout << "[ERROR] binding TCP failed" << endl;

		Shutdown();
	}
}

void NetworkManager::SetRemoteDataUDP()
{
	UDPoutAddr.sin_family = AF_INET;
	UDPoutAddr.sin_port = htons(8889);
	inet_pton(AF_INET, "127.0.0.1", &UDPoutAddr.sin_addr);
}

void NetworkManager::SendDataUDP(const char* message)
{
	int totalBytes = sendto(UDPSocketOut, message, strlen(message) + 1,
		0, reinterpret_cast<SOCKADDR*>(&UDPoutAddr), sizeof(UDPoutAddr));
	

	if (totalBytes == SOCKET_ERROR)
	{
		cout << "[ERROR] Failed to send UDP data" << endl;

		Shutdown();
	}
	else
	{
		cout << "Sent : " << totalBytes << " of data across the network!" << endl;
	}
}

int NetworkManager::RecieveDataUDP(char* message)
{
	int bytesRecieved = 0;
	int inAddrSize = sizeof(UDPinAddr);

	bytesRecieved = recvfrom(UDPSocketIn, message, MAX_MESSAGE,
		0, reinterpret_cast<SOCKADDR*>(&UDPinAddr), &inAddrSize);

	if (bytesRecieved == SOCKET_ERROR)
	{
		cout << "[ERROR] recieving UDP message" << endl;

		Shutdown();
	}

	return bytesRecieved;
}

void NetworkManager::ListenTCP()
{
	listen(TCPSocketIn, SOMAXCONN);
}

void NetworkManager::ConnectTCP()
{
	cout << "Please enter server port number: ";
	short serverPort;
	cin >> serverPort;
	cout << endl;

	cout << "Please enter server IP: ";
	char serverIP[15];
	cin.getline(serverIP, 15);
	cout << endl;

	TCPOutAddr.sin_family = AF_INET;
	TCPOutAddr.sin_port = htons(serverPort); // 7777
	inet_pton(AF_INET, serverIP, &TCPOutAddr.sin_addr); // 127.0.0.1

	int connectStatus = connect(TCPSocketOut, reinterpret_cast<sockaddr*>(&TCPOutAddr), sizeof(TCPOutAddr));
	
	if (connectStatus == SOCKET_ERROR)
	{
		cout << "[ERROR] Failed to connect to the TCP Server" << endl;
		Shutdown();
	}


	numConnections++;
	unsigned long bit = 1;
	ioctlsocket(TCPSocketOut, FIONBIO, &bit);
}

void NetworkManager::AcceptConnectionsTCP()
{
	int clientSize = sizeof(TCPClientAddr);

	TCPSocketClient = accept(TCPSocketIn, reinterpret_cast<SOCKADDR*>(&TCPClientAddr), &clientSize);
	// lab will have us connect to multiple clients. so maybe an array? <hint...sort of>

	if (TCPSocketClient != INVALID_SOCKET)
	{
		char clientIp[32];
		inet_ntop(AF_INET, &TCPClientAddr.sin_addr, clientIp, 32);

		cout << "Client's IP: " << clientIp << ". IP just connected." << endl;

		numConnections++;
	}

	unsigned long mode = 1;
	ioctlsocket(TCPSocketIn, FIONBIO, &mode);
	ioctlsocket(TCPSocketClient, FIONBIO, &mode);
}

void NetworkManager::SendDataTCP(const char* message, bool isServer)
{
	int totalBytes = 0;
	if (isServer)
	{
		totalBytes = send(TCPSocketClient, message, MAX_MESSAGE, 0);
	}
	else
	{
		totalBytes = send(TCPSocketOut, message, MAX_MESSAGE, 0);
	}

	/*
	The above code can be written via the ternary operator:
	totalBytes = send(isServer? TCPSocketClient : TCPSocketOut, message, MAX_MESSAGE, 0);
	*/

	if (totalBytes == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		if (error == WSAEWOULDBLOCK)
		{
			cout << "Sent data across: " << message << ". size was: " << totalBytes << endl;
		}
		else
		{
			cout << "[ERROR] Failed to send TCP message" << endl;
		}
	}
}

int NetworkManager::recieveDataTCP(char* message, bool isServer)
{
	int bytesReceived = recv(isServer? TCPSocketClient: TCPSocketOut, message, MAX_MESSAGE, 0);

	if (bytesReceived == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		if (error != WSAEWOULDBLOCK)
		{
			cout << "[ERROR] Failed to receive TCP data/message" << endl;
			Shutdown();
		}
	}
	return bytesReceived;
}

void NetworkManager::Shutdown()
{
	cout << "NetworkManager Shutdown" << endl;

	int error = WSAGetLastError();
	if (error)
	{
		cout << "Forced shutdown due to WSAError#: " << error << endl;
	}

	if (UDPSocketIn != INVALID_SOCKET)
	{
		if (closesocket(UDPSocketIn))
		{
			cout << "[ERROR] error closing UDP In socket" << endl;
		}
	}

	if (UDPSocketOut != INVALID_SOCKET)
	{
		if (closesocket(UDPSocketOut))
		{
			cout << "[ERROR] error closing UDP Out socket" << endl;
		}
	}

	if (TCPSocketIn != INVALID_SOCKET)
	{
		if (closesocket(TCPSocketIn))
		{
			cout << "[ERROR] error closing TCP In socket" << endl;
		}
	}

	if (TCPSocketOut != INVALID_SOCKET)
	{
		if (closesocket(TCPSocketOut))
		{
			cout << "[ERROR] error closing TCP Out socket" << endl;
		}
	}

	if (TCPSocketClient != INVALID_SOCKET)
	{
		if (closesocket(TCPSocketClient))
		{
			cout << "[ERROR] error closing TCP Client socket" << endl;
		}
	}

	WSACleanup();
	exit(0);
}

void NetworkManager::CreateUDPSockets()
{
	cout << "NetworkManager CreateUDPSockets" << endl;

	UDPSocketIn = socket(AF_INET, SOCK_DGRAM, 0);
	if (UDPSocketIn == INVALID_SOCKET)
	{
		cout << "Failed to create UDP in socket" << endl;
		Shutdown();
	}

	UDPSocketOut = socket(AF_INET, SOCK_DGRAM, 0);
	if (UDPSocketOut == INVALID_SOCKET)
	{
		cout << "Failed to create UDP out socket" << endl;
		Shutdown();
	}

}

void NetworkManager::CreateTCPSockets()
{
	cout << "NetworkManager CreateTCPSockets" << endl;

	TCPSocketIn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (TCPSocketIn == INVALID_SOCKET)
	{
		cout << "Failed to create TCP in socket" << endl;
		Shutdown();
	}

	TCPSocketOut = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (TCPSocketOut == INVALID_SOCKET)
	{
		cout << "Failed to create TCP out socket" << endl;
		Shutdown();
	}

	TCPSocketClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (TCPSocketOut == INVALID_SOCKET)
	{
		cout << "Failed to create TCP Client socket" << endl;
		Shutdown();
	}
}
