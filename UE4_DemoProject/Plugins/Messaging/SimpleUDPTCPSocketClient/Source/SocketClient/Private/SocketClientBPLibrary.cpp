// Copyright 2017-2019 David Romanski (Socke). All Rights Reserved.

#include "SocketClientBPLibrary.h"
#include "SocketClient.h"

//USocketClientBPLibrary* USocketClientBPLibrary::socketClientBPLibrary;

USocketClientBPLibrary::USocketClientBPLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {

	//socketClientBPLibrary = this;
	/*if (USocketClientHandler::socketClientHandler->getSocketClientTarget() == nullptr) {
		USocketClientHandler::socketClientHandler->addTCPClientToMap("0.0.0.0",0,this);
	 }*/

	if (!FSocketClientModule::isShuttingDown) {
		//Delegates
		onsocketClientConnectionEventDelegate.AddDynamic(this, &USocketClientBPLibrary::socketClientConnectionEventDelegate);
		onreceiveTCPMessageEventDelegate.AddDynamic(this, &USocketClientBPLibrary::receiveTCPMessageEventDelegate);
		onsocketClientUDPConnectionEventDelegate.AddDynamic(this, &USocketClientBPLibrary::socketClientUDPConnectionEventDelegate);
		onreceiveUDPMessageEventDelegate.AddDynamic(this, &USocketClientBPLibrary::receiveUDPMessageEventDelegate);
	}
}

/*Delegate functions*/
void USocketClientBPLibrary::socketClientConnectionEventDelegate(const bool success, const FString message, const FString clientConnectionIDP) {}
void USocketClientBPLibrary::receiveTCPMessageEventDelegate(const FString message, const TArray<uint8>& byteArray, const FString clientConnectionIDP) {}
void USocketClientBPLibrary::socketClientUDPConnectionEventDelegate(const bool success, const FString message) {}
void USocketClientBPLibrary::receiveUDPMessageEventDelegate(const FString message, const TArray<uint8>& byteArray, const FString IP, const int32 Port) {}


FString USocketClientBPLibrary::getLocalIP() {
	bool canBind = false;
	TSharedRef<FInternetAddr> localIp = USocketClientBPLibrary::getSocketSubSystemClientHandler()->GetLocalHostAddr(*GLog, canBind);

	if (localIp->IsValid()) {
		FString localIP = localIp->ToString(false);
		if (localIP.Equals("127.0.0.1")) {
			UE_LOG(LogTemp, Error, TEXT("Could not detect the local IP."));
			return "0.0.0.0";
		}
		return localIp->ToString(false);
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Could not detect the local IP."));
	}
	return "0.0.0.0";
}


USocketClientBPLibrary::~USocketClientBPLibrary() {
	closeSocketClientConnection();
}

//USocketClientBPLibrary * USocketClientBPLibrary::getSocketClientTarget(){
//	return socketClientBPLibrary;
//}

void USocketClientBPLibrary::socketClientSendTCPMessage(FString message, TArray<uint8> byteArray, bool addLineBreak) {
	if (message.Len() > 0 && addLineBreak) {
		message.Append("\r\n");
	}

	if (tcpSendThread == nullptr) {
		FString threadName = "FSendDataToServerThread" + FGuid::NewGuid().ToString();
		tcpSendThread = new FSendDataToServerThread();
		tcpSendThread->sendMessage(message, byteArray, USocketClientHandler::socketClientHandler->getSocketClientTargetByIP_AndPortInternal(clientConnectionID, 0), clientConnectionID);
		tcpSendThread->setThread(FRunnableThread::Create(tcpSendThread, *threadName, 0, EThreadPriority::TPri_Normal));
	}
	else {
		tcpSendThread->sendMessage(message, byteArray, USocketClientHandler::socketClientHandler->getSocketClientTargetByIP_AndPortInternal(clientConnectionID, 0), clientConnectionID);
	}



}

void USocketClientBPLibrary::socketClientSendTCPMessageTo(FString ip, int32 port, FString message, TArray<uint8> byteArray, bool addLineBreak) {
	if (message.Len() > 0 && addLineBreak) {
		message.Append("\r\n");
	}

	if (tcpSendThread == nullptr) {
		FString threadName = "FSendDataToServerThread" + FGuid::NewGuid().ToString();
		tcpSendThread = new FSendDataToServerThread();
		tcpSendThread->sendMessage(message, byteArray, USocketClientHandler::socketClientHandler->getSocketClientTargetByIP_AndPortInternal(ip, port), clientConnectionID);
		tcpSendThread->setThread(FRunnableThread::Create(tcpSendThread, *threadName, 0, EThreadPriority::TPri_Normal));
	}
	else {
		tcpSendThread->sendMessage(message, byteArray, USocketClientHandler::socketClientHandler->getSocketClientTargetByIP_AndPortInternal(ip, port), clientConnectionID);
	}

	//new FSendDataToServerThread(message, byteArray, USocketClientHandler::socketClientHandler->getSocketClientTargetByIP_AndPortInternal(ip, port), ip, port);
}



void USocketClientBPLibrary::connectSocketClient(FString domain, int32 port, EReceiveFilterClient receiveFilter, FString& clientConnectionIDP) {
	UDPreceiveFilter = receiveFilter;

	UPROPERTY()
		USocketClientBPLibrary* socketClientBPLibrary = NewObject<USocketClientBPLibrary>(USocketClientBPLibrary::StaticClass());
	socketClientBPLibrary->AddToRoot();

	//USocketClientBPLibrary* oldClient = USocketClientBPLibrary::getSocketClientTargetFromSocketCllientHandler(domain, port);

	USocketClientBPLibrary::addSocketClientTargetFromSocketCllientHandler(domain, port, socketClientBPLibrary);
	clientConnectionIDP = FGuid::NewGuid().ToString();
	socketClientBPLibrary->setClientConnectionID(clientConnectionIDP);
	USocketClientBPLibrary::addSocketClientTargetFromSocketCllientHandler(clientConnectionIDP, 0, socketClientBPLibrary);

	//connect to server
	new FServerConnectionThread(socketClientBPLibrary, clientConnectionIDP, receiveFilter, domain, port);
}

void USocketClientBPLibrary::socketClientSendUDPMessage(FString domain, int32 port, FString message, TArray<uint8> byteArray, bool addLineBreak, bool useReceiverSocket, FString uniqueID) {

	USocketClientBPLibrary* socketClientBPLibrary = USocketClientHandler::getSocketClientTarget();
	//socketClientBPLibrary->setServerIP(domain);
	//socketClientBPLibrary->setServerPort(port);


	if (message.Len() > 0 && addLineBreak) {
		message.Append("\r\n");
	}
	FString key = domain + "_" + FString::FromInt(port);
	if (uniqueID.IsEmpty() == false) {
		key += "_" + uniqueID;
	}

	if (udpThreads.Find(key) == nullptr) {
		FServerUDPConnectionThread* serverUDPConnectionThread = new FServerUDPConnectionThread(this, domain, port, (useReceiverSocket ? udpSocket : nullptr));
		FString threadName = "FServerUDPConnectionThread_" + FGuid::NewGuid().ToString();
		//set message
		serverUDPConnectionThread->setMessage(message, byteArray);
		//init thread. seted message is send after this line.
		serverUDPConnectionThread->setThread(FRunnableThread::Create(serverUDPConnectionThread, *threadName, 0, EThreadPriority::TPri_Normal));
		udpThreads.Add(key, serverUDPConnectionThread);
	}
	else {
		//reuse old thread to send message. is much faster than each time a new thread
		FServerUDPConnectionThread* serverUDPConnectionThread = *udpThreads.Find(key);
		serverUDPConnectionThread->sendMessage(message, byteArray);
	}

}

void USocketClientBPLibrary::socketClientInitUDPReceiver(FString domain, int32 port, EReceiveFilterClient receiveFilter) {
	USocketClientBPLibrary* socketClientBPLibrary = USocketClientHandler::getSocketClientTarget();

	socketClientBPLibrary->setReceiveFilter(receiveFilter);

	FString key = domain + "_" + FString::FromInt(port);


	if (udpThreads.Find(key) == nullptr) {
		FServerUDPConnectionThread* serverUDPConnectionThread = new FServerUDPConnectionThread(this, domain, port, nullptr);
		FString threadName = "FServerUDPConnectionThread_" + FGuid::NewGuid().ToString();
		serverUDPConnectionThread->setThread(FRunnableThread::Create(serverUDPConnectionThread, *threadName, 0, EThreadPriority::TPri_Normal));
		udpThreads.Add(key, serverUDPConnectionThread);
	}
}

void USocketClientBPLibrary::UDPReceiver(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt) {

	if (FSocketClientModule::isShuttingDown)
		return;

	TSharedPtr<FInternetAddr> peerAddr = EndPt.ToInternetAddr();
	FString ip = peerAddr->ToString(false);
	int32 port = peerAddr->GetPort();

	FString recvMessage;
	if (UDPreceiveFilter == EReceiveFilterClient::E_SAB || UDPreceiveFilter == EReceiveFilterClient::E_S) {
		char* Data = (char*)ArrayReaderPtr->GetData();
		Data[ArrayReaderPtr->Num()] = '\0';
		recvMessage = FString(UTF8_TO_TCHAR(Data));
	}

	TArray<uint8> byteArray;
	if (UDPreceiveFilter == EReceiveFilterClient::E_SAB || UDPreceiveFilter == EReceiveFilterClient::E_B) {
		byteArray.Append(ArrayReaderPtr->GetData(), ArrayReaderPtr->Num());
	}

	//switch to gamethread
	AsyncTask(ENamedThreads::GameThread, [recvMessage, byteArray, ip, port]() {
		if (FSocketClientModule::isShuttingDown)
			return;
		USocketClientHandler::socketClientHandler->getSocketClientTarget()->onreceiveUDPMessageEventDelegate.Broadcast(recvMessage, byteArray, ip, port);
		});

}


void USocketClientBPLibrary::closeSocketClientConnection() {
	//tcp
	run = false;
	//udp
	FUdpSocketReceiver* udpSR = getUDPSocketReceiver();
	if (udpSR != nullptr && udpSR != NULL) {
		udpSR->Stop();
		udpSR->Exit();
		udpSR = nullptr;
	}
	setUDPSocketReceiver(nullptr);
	setUDPSocket(nullptr);

	for (auto& elem : udpThreads) {
		FServerUDPConnectionThread* thread = elem.Value;
		if (thread != nullptr) {
			thread->stopThread();
		}
	}
	udpThreads.Empty();
	tcpSendThread = nullptr;
}


FString USocketClientBPLibrary::resolveDomain(FString serverDomainP) {

	FString* cachedDomainPointer = domainCache.Find(serverDomainP);
	if (cachedDomainPointer != nullptr) {
		return *cachedDomainPointer;
	}

	//is IP
	TArray<FString> ipNumbers;
	int32 lineCount = serverDomainP.ParseIntoArray(ipNumbers, TEXT("."), true);
	if (lineCount == 4 && serverDomainP.Len() <= 15 && serverDomainP.Len() >= 7) {
		domainCache.Add(serverDomainP, serverDomainP);
		return serverDomainP;
	}

	//resolve Domain
	ISocketSubsystem* sSS = USocketClientBPLibrary::getSocketSubSystemClientHandler();

	auto ResolveInfo = sSS->GetHostByName(TCHAR_TO_ANSI(*serverDomainP));
	while (!ResolveInfo->IsComplete());

	int32 errorCode = ResolveInfo->GetErrorCode();
	if (errorCode == 0) {
		const FInternetAddr* Addr = &ResolveInfo->GetResolvedAddress();
		uint32 OutIP = 0;
		FString adr = Addr->ToString(false);
		domainCache.Add(serverDomainP, adr);
		return adr;
	}
	else {
		if (dnsClient == nullptr)
			dnsClient = NewObject<UDNSClientSocketClient>(UDNSClientSocketClient::StaticClass());
		dnsClient->resolveDomain(sSS, serverDomainP);
		int32 timeout = 1000;
		while (dnsClient->isResloving() && timeout > 0) {
			timeout -= 10;
			FPlatformProcess::Sleep(0.01);
		}
		FString adr = dnsClient->getIP();
		domainCache.Add(serverDomainP, adr);
		return adr;
	}

	return serverDomainP;
}

void USocketClientBPLibrary::setClientConnectionID(FString clientConnectionIDP) {
	clientConnectionID = clientConnectionIDP;
}


//FString USocketClientBPLibrary::getServerIP() {
//	return serverIP;
//}
//
//int32 USocketClientBPLibrary::getServerPort() {
//	return serverPort;
//}

EReceiveFilterClient USocketClientBPLibrary::getReceiveFilter() {
	return UDPreceiveFilter;
}

//void USocketClientBPLibrary::setServerIP(FString ip) {
//	serverIP = ip;
//}
//
//void USocketClientBPLibrary::setServerPort(int32 port) {
//	serverPort = port;
//}

void USocketClientBPLibrary::setReceiveFilter(EReceiveFilterClient receiveFilter) {
	UDPreceiveFilter = receiveFilter;
}

USocketClientBPLibrary* USocketClientBPLibrary::getSocketClientTargetFromSocketCllientHandler(FString IP, int32 Port) {
	return USocketClientHandler::socketClientHandler->getSocketClientTargetByIP_AndPortInternal(IP, Port);
}

void USocketClientBPLibrary::removeSocketClientTargetFromSocketCllientHandler(FString IP, int32 Port) {
	USocketClientHandler::socketClientHandler->removeSocketClientTargetByIP_AndPortInternal(IP, Port);
}
void USocketClientBPLibrary::addSocketClientTargetFromSocketCllientHandler(FString IP, int32 Port, USocketClientBPLibrary* target) {
	USocketClientHandler::socketClientHandler->addTCPClientToMap(IP, Port, target);
}

ISocketSubsystem* USocketClientBPLibrary::getSocketSubSystemClientHandler() {
	return USocketClientHandler::socketClientHandler->getSocketSubSystem();
}

void USocketClientBPLibrary::getSystemType(ESocketClientSystem& system) {
#if PLATFORM_ANDROID
	system = ESocketClientSystem::Android;
	return;
#endif	
#if PLATFORM_IOS
	system = ESocketClientSystem::IOS;
	return;
#endif	
#if PLATFORM_WINDOWS
	system = ESocketClientSystem::Windows;
	return;
#endif	
#if PLATFORM_LINUX
	system = ESocketClientSystem::Linux;
	return;
#endif	
#if PLATFORM_MAC
	system = ESocketClientSystem::Mac;
	return;
#endif	
}

TArray<uint8> USocketClientBPLibrary::parseHexToBytes(FString hex) {
	TArray<uint8> bytes;

	if (hex.Contains(" ")) {
		hex = hex.Replace(TEXT(" "), TEXT(""));
	}

	if (hex.Len() % 2 != 0) {
		UE_LOG(LogTemp, Error, TEXT("This is not a valid hex string: %s"), *hex);
		return bytes;
	}


	TArray<TCHAR> charArray = hex.GetCharArray();
	for (int32 i = 0; i < (charArray.Num() - 1); i++) {
		if (CheckTCharIsHex(charArray[i]) == false) {
			UE_LOG(LogTemp, Error, TEXT("This is not a valid hex string: %s"), *hex);
			return bytes;
		}
	}


	bytes.AddZeroed(hex.Len() / 2);
	HexToBytes(hex, bytes.GetData());

	return bytes;
}

FString USocketClientBPLibrary::parseHexToString(FString hex) {
	TArray<uint8> bytes = parseHexToBytes(hex);
	char* Data = (char*)bytes.GetData();
	Data[bytes.Num()] = '\0';
	return FString(UTF8_TO_TCHAR(Data));
}

FString USocketClientBPLibrary::parseBytesToHex(TArray<uint8> bytes) {
	FString hex;
	hex = BytesToHex(bytes.GetData(), bytes.Num());
	return hex;
}

TArray<uint8> USocketClientBPLibrary::parseHexToBytesPure(FString hex) {
	return parseHexToBytes(hex);
}

FString USocketClientBPLibrary::parseHexToStringPure(FString hex) {
	return parseHexToString(hex);
}

FString USocketClientBPLibrary::parseBytesToHexPure(TArray<uint8> bytes) {
	return parseBytesToHex(bytes);
}


bool USocketClientBPLibrary::shouldRun() {
	return run;
}

void USocketClientBPLibrary::setRun(bool runP) {
	run = runP;
}

void USocketClientBPLibrary::setTCPSocket(FSocket* socketP) {
	socketTCP = socketP;
}

FSocket* USocketClientBPLibrary::getTCPSocket() {
	return socketTCP;
}

int32 USocketClientBPLibrary::getUniquePlayerID(APlayerController* playerController) {
	if (playerController == nullptr || playerController->GetLocalPlayer() == nullptr)
		return 0;
	return playerController->GetLocalPlayer()->GetUniqueID();
}

void USocketClientBPLibrary::setUDPSocketReceiver(FUdpSocketReceiver* udpSocketReceiverP) {
	udpSocketReceiver = udpSocketReceiverP;
}

void USocketClientBPLibrary::setUDPSocket(FSocket* socketP) {
	udpSocket = socketP;
}

//FSocket* USocketClientBPLibrary::getUDPSocket() {
//	return socketUDP;
//}

FUdpSocketReceiver* USocketClientBPLibrary::getUDPSocketReceiver() {
	return udpSocketReceiver;
}
//FString USocketClientBPLibrary::getUDPIP() {
//	return serverIPUDP;
//}
//
//int32 USocketClientBPLibrary::getUDPPort() {
//	return serverPortUDP;
//}