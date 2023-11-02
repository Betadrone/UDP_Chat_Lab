#include <string>
#include <iostream>
#include "NetworkManager.h"
using namespace std;

int main()
{
	NetworkManager::GetInstance()->Init();

	NetworkManager::GetInstance()->CreateTCPSockets();

	const int SERVER = 1;

	int userChoice = 0;

	cout << "choose your role: " << endl;
	cout << "\t1) Server" << endl;
	cout << "\t2) Client" << endl;
	cin >> userChoice;

	if (userChoice == SERVER)
	{
		NetworkManager::GetInstance()->BindTCP();
		NetworkManager::GetInstance()->ListenTCP();
	}
	else
	{
		NetworkManager::GetInstance()->ConnectTCP();
	}

	
	while(NetworkManager::GetInstance()->GetNumConnections() <= 0)
	{
		if (userChoice == SERVER)
		{
			NetworkManager::GetInstance()->AcceptConnectionsTCP();
		}
	}

	string myMsg;

	while (true)
	{
		getline(cin, myMsg);
		if (myMsg.length() > 0)
		{
			NetworkManager::GetInstance()->SendDataTCP(myMsg.c_str(), userChoice == SERVER);
		}

		char rcvMsg[NetworkManager::MAX_MESSAGE];
		int recSize = NetworkManager::GetInstance()->recieveDataTCP(rcvMsg, userChoice == SERVER);
		if (recSize > 0)
		{
			cout << rcvMsg << endl;
		}
	}

	NetworkManager::GetInstance()->Shutdown();

	return 0;
}

