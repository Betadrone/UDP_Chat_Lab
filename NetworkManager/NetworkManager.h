#pragma once
#pragma comment (lib, "ws2_32.lib")

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinSock2.h>

class NetworkManager
{
public:
	static NetworkManager* GetInstance()
	{
		if (instance == nullptr)
		{
			instance = new NetworkManager();
		}
		return instance;
	}

	void Init();
	void Shutdown();
	void CreateUDPSockets();
	void CreateTCPSockets();
	void BindUDP();
	void BindTCP();
	void SetRemoteDataUDP();
	void SendDataUDP(const char* message);
	int RecieveDataUDP(char* message);

	void ListenTCP(); // server
	void ConnectTCP(); // client
	void AcceptConnectionsTCP();
	void SendDataTCP(const char* message, bool isServer);
	int recieveDataTCP(char* message, bool isServer);

	int GetNumConnections() { return numConnections; }

	static const int MAX_MESSAGE = 1500;

private:
	NetworkManager();
	~NetworkManager();

	SOCKET UDPSocketIn;
	SOCKET UDPSocketOut;

	SOCKET TCPSocketIn; // right click and change name to TCPSocketListen
	SOCKET TCPSocketOut;
	SOCKET TCPSocketClient;

	SOCKADDR_IN UDPoutAddr;
	SOCKADDR_IN UDPinAddr;

	SOCKADDR_IN TCPOutAddr;
	SOCKADDR_IN TCPInAddr;
	SOCKADDR_IN TCPClientAddr;

	int numConnections = 0;

	static NetworkManager* instance;
};

