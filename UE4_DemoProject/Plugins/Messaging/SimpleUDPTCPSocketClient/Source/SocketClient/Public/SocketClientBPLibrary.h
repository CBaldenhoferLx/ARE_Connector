// Copyright 2017-2019 David Romanski (Socke). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SocketClient.h"
#include "Runtime/Sockets/Public/Sockets.h"
#include "Runtime/Networking/Public/Networking.h"
#include "Runtime/Sockets/Public/SocketSubsystem.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
#include "Runtime/Engine/Classes/Engine/LocalPlayer.h"
#include "SocketClientBPLibrary.generated.h"


UENUM(BlueprintType)
enum class EReceiveFilterClient : uint8
{
	E_SAB 	UMETA(DisplayName = "Message And Bytes"),
	E_S		UMETA(DisplayName = "Message"),
	E_B		UMETA(DisplayName = "Bytes")

};

UENUM(BlueprintType)
enum class ESocketClientSystem : uint8
{
	Android,
	IOS,
	Windows,
	Linux,
	Mac
};

class USocketClientHandler;
class FServerUDPConnectionThread;

UCLASS()
class SOCKETCLIENT_API USocketClientBPLibrary : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	~USocketClientBPLibrary();

	//Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FsocketClientConnectionEventDelegate, bool, success, FString, message, FString, clientConnectionID);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FreceiveTCPMessageEventDelegate, FString, message, const TArray<uint8>&, byteArray, FString, clientConnectionID);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FsocketClientUDPConnectionEventDelegate, bool, success, FString, message);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FreceiveUDPMessageEventDelegate, FString, message, const TArray<uint8>&, byteArray, FString, IP, int32, port);

	UFUNCTION()
		void socketClientConnectionEventDelegate(const bool success, const FString message, const FString clientConnectionID);
	UPROPERTY(BlueprintAssignable, Category = "SocketClient|TCP|Events|ConnectionInfo")
		FsocketClientConnectionEventDelegate onsocketClientConnectionEventDelegate;
	UFUNCTION()
		void receiveTCPMessageEventDelegate(const FString message, const TArray<uint8>& byteArray, const FString clientConnectionID);
	UPROPERTY(BlueprintAssignable, Category = "SocketClient|TCP|Events|ReceiveMessage")
		FreceiveTCPMessageEventDelegate onreceiveTCPMessageEventDelegate;
	UFUNCTION()
		void socketClientUDPConnectionEventDelegate(const bool success, const FString message);
	UPROPERTY(BlueprintAssignable, Category = "SocketClient|UDP|Events|ConnectionInfo")
		FsocketClientUDPConnectionEventDelegate onsocketClientUDPConnectionEventDelegate;
	UFUNCTION()
		void receiveUDPMessageEventDelegate(const FString message, const TArray<uint8>& byteArray, const  FString IP, const int32 Port);
	UPROPERTY(BlueprintAssignable, Category = "SocketClient|UDP|Events|ReceiveMessage")
		FreceiveUDPMessageEventDelegate onreceiveUDPMessageEventDelegate;

	/**
	*Trying to determine the local IP. It uses a function in the engine that does not work on all devices. On Windows and Linux it seems to work very well. Very bad on Android. 0.0.0.0 will be returned if it doesn't work. 
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketClient")
		static FString getLocalIP();



	//TCP
	/**
	*Single TCP Connection only! Send a string. TCP connection must exist.
	*@param message String to send
	*@param addLineBreak add a line break at the end
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketClient|TCP", meta = (AutoCreateRefTerm = "byteArray"))
		void socketClientSendTCPMessage(FString message, TArray<uint8> byteArray, bool addLineBreak = true);
	/**
	*Multi TCP Connection only! Send a string to a specific connection. TCP connection must exist.
	*@param DomainOrIP target IP or Domain
	*@param port target port
	*@param message String to send
	*@param addLineBreak add a line break at the end
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketClient|TCP", meta = (AutoCreateRefTerm = "byteArray"))
		void socketClientSendTCPMessageTo(FString DomainOrIP, int32 port, FString message, TArray<uint8> byteArray, bool addLineBreak = true);
	/**
	*TCP and UDP connections are closed.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketClient")
		void closeSocketClientConnection();
	/**
	*Connect to a TCP Server
	*@param domainOrIP IP or Domain to listen
	*@param port port to listen
	*@param receiveFilter This allows you to decide which data type you want to receive. If you receive files it makes no sense to convert them into a string.
	*@param ClientConnectionID Unique ID for this connection. The ID can be used to reuse exactly this connection.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketClient|TCP")
		void connectSocketClient(FString domainOrIP, int32 port, EReceiveFilterClient receiveFilter, FString& ClientConnectionID);

	bool shouldRun();
	void setRun(bool runP);
	void setTCPSocket(FSocket* socket);
	FSocket* getTCPSocket();
	FSocket* socketTCP;

	//UDP
	/**
	*UDP is Host to Host. Sends a Message to specific target and opens a connection on random port to listen. If you want to create multiple connections to the same server (ip and port) then the parameter "useReceiverSocket" must be false and a uniqueID must be set.
	*@param DomainOrIP target IP or Domain
	*@param port target port
	*@param message String to send
	*@param addLineBreak add a line break at the end
	*@param useReceiverSocket with this parameter the same socket is used that was created with "initUDPReceiver". There is only one socket to send and receive to increase compatibility.
	*@param uniqueID is optional and required when multiple connections to the same server (same ip and port) shall be established. You can use getUniquePlayerID
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketClient|UDP", meta = (AutoCreateRefTerm = "byteArray"))
		void socketClientSendUDPMessage(FString domainOrIP, int32 port, FString message, TArray<uint8> byteArray, bool addLineBreak = true, bool useReceiverSocket = true, FString uniqueID = "");
	/**
	*UDP is Host to Host. Opens a connection on specific ip and port and listen on it.
	*@param DomainOrIP IP or Domain to listen
	*@param port port to listen
	*@param receiveFilter This allows you to decide which data type you want to receive. If you receive files it makes no sense to convert them into a string.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketClient|UDP")
		void socketClientInitUDPReceiver(FString domainOrIP, int32 port, EReceiveFilterClient receiveFilter);
	void UDPReceiver(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt);


	UFUNCTION(BlueprintCallable, Category = "SocketClient", Meta = (ExpandEnumAsExecs = "system"))
		static void getSystemType(ESocketClientSystem& system);

	UFUNCTION(BlueprintCallable, Category = "SocketClient|Hex")
		static TArray<uint8> parseHexToBytes(FString hex);

	UFUNCTION(BlueprintCallable, Category = "SocketClient|Hex")
		static FString parseHexToString(FString hex);

	UFUNCTION(BlueprintCallable, Category = "SocketClient|Hex")
		static FString parseBytesToHex(TArray<uint8> bytes);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SocketClient|Hex")
		static TArray<uint8> parseHexToBytesPure(FString hex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SocketClient|Hex")
		static FString parseHexToStringPure(FString hex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SocketClient|Hex")
		static FString parseBytesToHexPure(TArray<uint8> bytes);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SocketClient")
		static int32 getUniquePlayerID(APlayerController* playerController = nullptr);

	void setUDPSocketReceiver(FUdpSocketReceiver* udpSocketReceiver);
	void setUDPSocket(FSocket* socket);

	FUdpSocketReceiver* getUDPSocketReceiver();
	FUdpSocketReceiver* udpSocketReceiver;
	EReceiveFilterClient getReceiveFilter();
	FSocket* udpSocket = nullptr;


	void setReceiveFilter(EReceiveFilterClient receiveFilter);

	/*Sorry Spaghetti code because backward compatibility*/
	static USocketClientBPLibrary* getSocketClientTargetFromSocketCllientHandler(FString IP, int32 Port);
	static void removeSocketClientTargetFromSocketCllientHandler(FString IP, int32 Port);
	static void addSocketClientTargetFromSocketCllientHandler(FString IP, int32 Port, USocketClientBPLibrary* target);

	static ISocketSubsystem* getSocketSubSystemClientHandler();


	FString resolveDomain(FString domain);
	//ue4 domain resolve does not work with steam. this is my own dns client
	class UDNSClientSocketClient* dnsClient = nullptr;
	TMap<FString, FString> domainCache;

	void setClientConnectionID(FString clientConnectionID);

private:

	FString serverIP = FString(TEXT("0.0.0.0"));
	FString serverDomain = FString(TEXT("0.0.0.0"));
	int32	serverPort = 0;
	bool    run;
	FString clientConnectionID;

	class FSendDataToServerThread* tcpSendThread = nullptr;

	TMap<FString, FServerUDPConnectionThread*> udpThreads;// serverUDPConnectionThread = nullptr;
	EReceiveFilterClient			UDPreceiveFilter;
};

/*********************************************** TCP ***********************************************/
/* asynchronous Thread*/
class FServerConnectionThread : public FRunnable {

public:

	FServerConnectionThread(USocketClientBPLibrary* socketClientP, FString clientConnectionIDP, EReceiveFilterClient receiveFilterP, FString ipP, int32 portP) :
		socketClient(socketClientP),
		clientConnectionID(clientConnectionIDP),
		receiveFilter(receiveFilterP),
		ipGlobal(ipP),
		portGlobal(portP) {
		FString threadName = "FServerConnectionThread" + FGuid::NewGuid().ToString();
		FRunnableThread::Create(this, *threadName, 0, EThreadPriority::TPri_Normal);
	}

	virtual uint32 Run() override {
		//UE_LOG(LogTemp, Display, TEXT("DoWork:%s"),*(FDateTime::Now()).ToString());
		FString ipOrDomain = ipGlobal;
		FString ip = socketClient->resolveDomain(ipOrDomain);
		int32 port = portGlobal;
		FString clientConnectionIDGlobal = clientConnectionID;
		//UE_LOG(LogTemp, Warning, TEXT("Tread:%s:%i"),*ip, port);
		ISocketSubsystem* sSS = USocketClientBPLibrary::getSocketSubSystemClientHandler();
		TSharedRef<FInternetAddr> addr = sSS->CreateInternetAddr();
		bool bIsValid;
		addr->SetIp(*ip, bIsValid);
		addr->SetPort(port);

		//is already connected? 
		//USocketClientBPLibrary* oldClient = USocketClientBPLibrary::getSocketClientTargetFromSocketCllientHandler(ip, port);

		//if (oldClient != nullptr) {
		//	oldClient->closeSocketClientConnection();
		//	FPlatformProcess::Sleep(0.2);
		//	//wait for clean disconnect
		//	int32 maxWaitTime = 200;//max x seconds
		//	while (maxWaitTime > 0 && oldClient->shouldRun() && USocketClientBPLibrary::getSocketClientTargetFromSocketCllientHandler(ipOrDomain, port) != nullptr) {
		//		FPlatformProcess::Sleep(0.1);
		//		maxWaitTime--;
		//		//UE_LOG(LogTemp, Warning, TEXT("Wait for clean disconnect:%i"), maxWaitTime);
		//	}
		//	USocketClientBPLibrary::removeSocketClientTargetFromSocketCllientHandler(ipOrDomain, port);
		//	USocketClientBPLibrary::addSocketClientTargetFromSocketCllientHandler(ipOrDomain, port, socketClient);
		//}

		//USocketClientBPLibrary::addSocketClientTargetFromSocketCllientHandler(ip, port, socketClient);
		//USocketClientBPLibrary::addSocketClientTargetFromSocketCllientHandler(originalIP, port, socketClient);





		if (bIsValid) {
			// create the socket
			FSocket* socket = sSS->CreateSocket(NAME_Stream, TEXT("socketClient"));
			socketClient->setTCPSocket(socket);

			// try to connect to the server
			if (socket == nullptr || socket->Connect(*addr) == false) {
				// on failure, shut it all down
				sSS->DestroySocket(socket);
				socket = NULL;
				AsyncTask(ENamedThreads::GameThread, [ipOrDomain, ip, port, clientConnectionIDGlobal]() {
					USocketClientBPLibrary* socketClientTmp = USocketClientBPLibrary::getSocketClientTargetFromSocketCllientHandler(clientConnectionIDGlobal, 0);
					if (socketClientTmp != nullptr)
						socketClientTmp->onsocketClientConnectionEventDelegate.Broadcast(false, "Connection failed:" + ipOrDomain + "(" + ip + "):" + FString::FromInt(port) + "|" + clientConnectionIDGlobal, clientConnectionIDGlobal);

					socketClientTmp = USocketClientBPLibrary::getSocketClientTargetFromSocketCllientHandler("0.0.0.0", 0);
					if (socketClientTmp != nullptr && !socketClientTmp->shouldRun()) {
						socketClientTmp->onsocketClientConnectionEventDelegate.Broadcast(false, "Connection failed:" + ipOrDomain + "(" + ip + "):" + FString::FromInt(port) + "|" + clientConnectionIDGlobal, clientConnectionIDGlobal);
					}

					});
				return 0;
				//UE_LOG(LogNetworkPlatformFile, Error, TEXT("Failed to connect to file server at %s."), *Addr->ToString(true));
			}
			else {
				AsyncTask(ENamedThreads::GameThread, [ipOrDomain, port, clientConnectionIDGlobal]() {
					USocketClientBPLibrary* socketClientTmp = USocketClientBPLibrary::getSocketClientTargetFromSocketCllientHandler(clientConnectionIDGlobal, 0);
					if (socketClientTmp != nullptr)
						socketClientTmp->onsocketClientConnectionEventDelegate.Broadcast(true, "Connection successful:" + ipOrDomain + ":" + FString::FromInt(port) + "|" + clientConnectionIDGlobal, clientConnectionIDGlobal);

					socketClientTmp = USocketClientBPLibrary::getSocketClientTargetFromSocketCllientHandler("0.0.0.0", 0);
					if (socketClientTmp != nullptr && !socketClientTmp->shouldRun()) {
						socketClientTmp->onsocketClientConnectionEventDelegate.Broadcast(true, "Connection successful:" + ipOrDomain + ":" + FString::FromInt(port), clientConnectionIDGlobal);
					}
					});
				socketClient->setRun(true);

				int64 ticks1;
				int64 ticks2;

				uint32 DataSize;
				while (socket != nullptr && socketClient->shouldRun()) {

					//ESocketConnectionState::SCS_Connected does not work https://issues.unrealengine.com/issue/UE-27542
					//Compare ticks is a workaround to get a disconnect. clientSocket->Wait() stop working after disconnect. (Another bug?)
					//If it doesn't wait any longer, ticks1 and ticks2 should be the same == disconnect.
					ticks1 = FDateTime::Now().GetTicks();
					socket->Wait(ESocketWaitConditions::WaitForReadOrWrite, FTimespan::FromSeconds(0.1));
					ticks2 = FDateTime::Now().GetTicks();

					bool hasData = socket->HasPendingData(DataSize);
					if (!hasData && ticks1 == ticks2) {
						UE_LOG(LogTemp, Display, TEXT("TCP connection broken. End Loop"));
						break;
					}

					if (hasData) {
						FArrayReaderPtr Datagram = MakeShareable(new FArrayReader(true));
						Datagram->SetNumUninitialized(DataSize);
						int32 BytesRead = 0;
						if (socket->Recv(Datagram->GetData(), Datagram->Num(), BytesRead)) {

							FString recvMessage;
							if (receiveFilter == EReceiveFilterClient::E_SAB || receiveFilter == EReceiveFilterClient::E_S) {
								char* Data = (char*)Datagram->GetData();
								Data[BytesRead] = '\0';
								recvMessage = FString(UTF8_TO_TCHAR(Data));
							}

							TArray<uint8> byteArray;
							if (receiveFilter == EReceiveFilterClient::E_SAB || receiveFilter == EReceiveFilterClient::E_B) {
								byteArray.Append(Datagram->GetData(), Datagram->Num());
							}

							//switch to gamethread
							AsyncTask(ENamedThreads::GameThread, [recvMessage, byteArray, ipOrDomain, port, clientConnectionIDGlobal]() {
								USocketClientBPLibrary* socketClientTmp = USocketClientBPLibrary::getSocketClientTargetFromSocketCllientHandler(clientConnectionIDGlobal, 0);
								if (socketClientTmp != nullptr)
									socketClientTmp->onreceiveTCPMessageEventDelegate.Broadcast(recvMessage, byteArray, clientConnectionIDGlobal);

								socketClientTmp = USocketClientBPLibrary::getSocketClientTargetFromSocketCllientHandler("0.0.0.0", 0);
								if (socketClientTmp != nullptr && !socketClientTmp->shouldRun()) {
									socketClientTmp->onreceiveTCPMessageEventDelegate.Broadcast(recvMessage, byteArray, clientConnectionIDGlobal);
								}
								});
						}
						Datagram->Empty();
					}
				}

				//UE_LOG(LogTemp, Warning, TEXT("Close Socket 1"));
				AsyncTask(ENamedThreads::GameThread, [ipOrDomain, port, clientConnectionIDGlobal]() {
					//UE_LOG(LogTemp, Warning, TEXT("Close Socket 2"));
					USocketClientBPLibrary* socketClientTmp = USocketClientBPLibrary::getSocketClientTargetFromSocketCllientHandler(clientConnectionIDGlobal, 0);
					if (socketClientTmp != nullptr) {
						socketClientTmp->onsocketClientConnectionEventDelegate.Broadcast(false, "Connection close:" + ipOrDomain + ":" + FString::FromInt(port) + "|" + clientConnectionIDGlobal, clientConnectionIDGlobal);
						socketClientTmp->setRun(false);
					}

					socketClientTmp = USocketClientBPLibrary::getSocketClientTargetFromSocketCllientHandler("0.0.0.0", 0);
					if (socketClientTmp != nullptr && !socketClientTmp->shouldRun()) {
						socketClientTmp->onsocketClientConnectionEventDelegate.Broadcast(false, "Connection close:" + ipOrDomain + ":" + FString::FromInt(port) + "|" + clientConnectionIDGlobal, clientConnectionIDGlobal);
					}

					//UE_LOG(LogTemp, Warning, TEXT("Close Socket 3"));
					USocketClientBPLibrary::removeSocketClientTargetFromSocketCllientHandler(ipOrDomain, port);
					USocketClientBPLibrary::removeSocketClientTargetFromSocketCllientHandler(clientConnectionIDGlobal, 0);


					});
				//UE_LOG(LogTemp, Warning, TEXT("Close Socket 4"));
				socketClient->setRun(false);
				sSS->DestroySocket(socket);
				socket = NULL;
				UE_LOG(LogTemp, Display, TEXT("Close Socket"));
			}
		}

		return 0;
	}


protected:
	USocketClientBPLibrary* socketClient;
	//USocketClientBPLibrary*		oldClient;
	FString						clientConnectionID;
	FString						originalIP;
	EReceiveFilterClient		receiveFilter;
	FString ipGlobal;
	int32 portGlobal;
};




/* asynchronous Thread*/
class FSendDataToServerThread : public FRunnable {

public:

	FSendDataToServerThread() {
		/*FString threadName = "FSendDataToServerThread" + FGuid::NewGuid().ToString();
		FRunnableThread::Create(this, *threadName, 0, EThreadPriority::TPri_Normal);*/
	}

	virtual uint32 Run() override {

		if (socketClient == nullptr) {
			UE_LOG(LogTemp, Error, TEXT("Class is not initialized."));
			return 0;
		}

		while (socketClient->shouldRun()) {

			FString clientConnectionIDGlobal = clientConnectionID;

			// get the socket
			FSocket* socket = socketClient->getTCPSocket();

			// try to connect to the server

			if (socket == NULL || socket == nullptr) {
				UE_LOG(LogTemp, Error, TEXT("Connection not exist."));
				AsyncTask(ENamedThreads::GameThread, [clientConnectionIDGlobal]() {
					USocketClientBPLibrary* socketClientTmp = USocketClientBPLibrary::getSocketClientTargetFromSocketCllientHandler(clientConnectionIDGlobal, 0);
					if (socketClientTmp != nullptr) {
						socketClientTmp->onsocketClientConnectionEventDelegate.Broadcast(false, "Connection not exist:" + clientConnectionIDGlobal, clientConnectionIDGlobal);
						socketClientTmp->closeSocketClientConnection();
					}

					socketClientTmp = USocketClientBPLibrary::getSocketClientTargetFromSocketCllientHandler("0.0.0.0", 0);
					if (socketClientTmp != nullptr && !socketClientTmp->shouldRun()) {
						socketClientTmp->onsocketClientConnectionEventDelegate.Broadcast(false, "Connection not exist:" + clientConnectionIDGlobal, clientConnectionIDGlobal);
					}
					});
				return 0;
			}


			if (socket != nullptr && socket->GetConnectionState() == ESocketConnectionState::SCS_Connected) {
				while (messageQueue.IsEmpty() == false) {
					FString m;
					messageQueue.Dequeue(m);
					FTCHARToUTF8 Convert(*m);
					int32 sent = 0;
					socket->Send((uint8*)((ANSICHAR*)Convert.Get()), Convert.Length(), sent);
				}

				while (byteArrayQueue.IsEmpty() == false) {
					TArray<uint8> ba;
					byteArrayQueue.Dequeue(ba);
					int32 sent = 0;
					socket->Send(ba.GetData(), ba.Num(), sent);
					ba.Empty();
				}

			}
			else {
				UE_LOG(LogTemp, Error, TEXT("Connection Lost"));
				AsyncTask(ENamedThreads::GameThread, [clientConnectionIDGlobal]() {
					USocketClientBPLibrary* socketClientTmp = USocketClientBPLibrary::getSocketClientTargetFromSocketCllientHandler(clientConnectionIDGlobal, 0);
					if (socketClientTmp != nullptr)
						socketClientTmp->onsocketClientConnectionEventDelegate.Broadcast(false, "Connection lost:" + clientConnectionIDGlobal, clientConnectionIDGlobal);

					socketClientTmp = USocketClientBPLibrary::getSocketClientTargetFromSocketCllientHandler("0.0.0.0", 0);
					if (socketClientTmp != nullptr && !socketClientTmp->shouldRun()) {
						socketClientTmp->onsocketClientConnectionEventDelegate.Broadcast(false, "Connection lost:" + clientConnectionIDGlobal, clientConnectionIDGlobal);
					}
					});
			}
			if (socketClient->shouldRun()) {
				pauseThread(true);
				//workaround. suspend do not work on all platforms. lets sleep
				while (paused && socketClient->shouldRun()) {
					FPlatformProcess::Sleep(0.01);
				}
			}
		}

		return 0;
	}

	FRunnableThread* getThread() {
		return thread;
	}

	void setThread(FRunnableThread* threadP) {
		thread = threadP;
	}

	void sendMessage(FString messageP, TArray<uint8> byteArrayP, USocketClientBPLibrary* socketClientP, FString clientConnectionIDP) {
		if (messageP.Len() > 0)
			messageQueue.Enqueue(messageP);
		if (byteArrayP.Num() > 0)
			byteArrayQueue.Enqueue(byteArrayP);
		socketClient = socketClientP;
		clientConnectionID = clientConnectionIDP;
		pauseThread(false);
	}


	void pauseThread(bool pause) {
		paused = pause;
		if (thread != nullptr)
			thread->Suspend(pause);
	}


protected:
	TQueue<FString> messageQueue;
	TQueue<TArray<uint8>> byteArrayQueue;
	USocketClientBPLibrary* socketClient;
	FString clientConnectionID;
	FRunnableThread* thread = nullptr;
	bool					run;
	bool					paused;
};


/*********************************************** UDP ***********************************************/
/* asynchronous Thread*/
class FServerUDPConnectionThread : public FRunnable {

public:

	FServerUDPConnectionThread(USocketClientBPLibrary* socketClientP, FString ipP, int32 portP, FSocket* receiverSocketP) :
		socketClient(socketClientP),
		ipGlobal(ipP),
		portGlobal(portP),
		receiverSocket(receiverSocketP) {
		/*FString threadName = "FServerUDPConnectionThread_" + FGuid::NewGuid().ToString();
		thread = FRunnableThread::Create(this, *threadName, 0, EThreadPriority::TPri_Normal);*/
		run = true;
	}

	virtual uint32 Run() override {

		while (run) {

			if (socketClient == nullptr || thread == nullptr) {
				FPlatformProcess::Sleep(0.1);
				continue;
			}


			FString ip = socketClient->resolveDomain(ipGlobal);
			int32 port = portGlobal;

			if (socket == nullptr && receiverSocket != nullptr) {
				socket = receiverSocket;
			}


			if (socket == nullptr || socket == NULL) {

				FString endpointAdress = ip + ":" + FString::FromInt(port);
				FIPv4Endpoint Endpoint;

				// create the socket
				FString socketName;
				ISocketSubsystem* socketSubsystem = USocketClientBPLibrary::getSocketSubSystemClientHandler();
				TSharedPtr<class FInternetAddr> addr = socketSubsystem->CreateInternetAddr();

				socket = socketSubsystem->CreateSocket(NAME_DGram, *socketName, true);
				if (socket == nullptr || socket == NULL) {
					run = false;
					const TCHAR* SocketErr = socketSubsystem->GetSocketError(SE_GET_LAST_ERROR_CODE);
					UE_LOG(LogTemp, Error, TEXT("UE4 could not init a UDP socket. You can only create listening connections on local IPs. An external IP must be redirected to a local IP in your router. %s:%i. Error: %s"), *ip, port, SocketErr);
					break;
				}

				if (!socket->SetRecvErr()) {
					UE_LOG(LogTemp, Error, TEXT("SocketClient UDP. Can't set recverr"));
				}

				bool validIP = true;
				//if you send a message without init udp receiver first than a random port is set
				if (messageQueue.IsEmpty() == false || byteArrayQueue.IsEmpty() == false) {
					addr->SetIp(TEXT("0.0.0.0"), validIP);
					addr->SetPort(socket->GetPortNo());
				}
				else {
					addr->SetPort(port);
					addr->SetIp(*ip, validIP);
				}

				if (!validIP) {
					UE_LOG(LogTemp, Error, TEXT("SocketClient UDP. Can't set ip"));
					USocketClientBPLibrary* socketClientTMP = socketClient;
					AsyncTask(ENamedThreads::GameThread, [socketClientTMP, addr]() {
						if (socketClientTMP != nullptr)
							socketClientTMP->onsocketClientUDPConnectionEventDelegate.Broadcast(false, "SocketClient UDP. Can't set ip");
						});
					break;
				}

				if (socket == nullptr || socket == NULL || !validIP) {
					run = false;
					const TCHAR* SocketErr = socketSubsystem->GetSocketError(SE_GET_LAST_ERROR_CODE);
					UE_LOG(LogTemp, Error, TEXT("UE4 could not init a UDP socket. You can only create listening connections on local IPs. An external IP must be redirected to a local IP in your router. %s:%i. Error: %s"), *ip, port, SocketErr);
					USocketClientBPLibrary* socketClientTMP = socketClient;
					AsyncTask(ENamedThreads::GameThread, [socketClientTMP, addr, SocketErr]() {
						if (socketClientTMP != nullptr)
							socketClientTMP->onsocketClientUDPConnectionEventDelegate.Broadcast(false, "(Error 1) UE4 could not init a UDP socket. You can only create listening connections on local IPs. An external IP must be redirected to a local IP in your router." + addr->ToString(true) + " Error:" + SocketErr);
						});
					break;
				}
				socket->SetReuseAddr(true);
				socket->SetNonBlocking(true);
				socket->SetBroadcast(true);

				if (!socket->Bind(*addr)) {
					run = false;
					const TCHAR* SocketErr = socketSubsystem->GetSocketError(SE_GET_LAST_ERROR_CODE);
					UE_LOG(LogTemp, Error, TEXT("UE4 could not init a UDP socket. You can only create listening connections on local IPs. An external IP must be redirected to a local IP in your router. %s:%i. Error: %s"), *ip, port, SocketErr);

					USocketClientBPLibrary* socketClientTMP = socketClient;
					AsyncTask(ENamedThreads::GameThread, [socketClientTMP, addr, SocketErr]() {
						if (socketClientTMP != nullptr)
							socketClientTMP->onsocketClientUDPConnectionEventDelegate.Broadcast(false, "(Error 2) UE4 could not init a UDP socket. You can only create listening connections on local IPs. An external IP must be redirected to a local IP in your router." + addr->ToString(true) + " Error:" + SocketErr);
						});
					break;
				}


				FUdpSocketReceiver* udpSocketReceiver = socketClient->getUDPSocketReceiver();
				if (udpSocketReceiver == nullptr) {

					FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(100);
					udpSocketReceiver = new FUdpSocketReceiver(socket, ThreadWaitTime, TEXT("SocketClientBPLibUDPReceiverThread"));
					udpSocketReceiver->OnDataReceived().BindUObject(socketClient, &USocketClientBPLibrary::UDPReceiver);
					udpSocketReceiver->Start();
					socketClient->setUDPSocketReceiver(udpSocketReceiver);
					socketClient->setUDPSocket(socket);

					USocketClientBPLibrary* socketClientTMP = socketClient;
					AsyncTask(ENamedThreads::GameThread, [socketClientTMP, addr]() {
						if (socketClientTMP != nullptr)
							socketClientTMP->onsocketClientUDPConnectionEventDelegate.Broadcast(true, "Init UDP Connection OK. " + addr->ToString(true));
						});
				}

			}



			if (run && (messageQueue.IsEmpty() == false || byteArrayQueue.IsEmpty() == false)) {

				TSharedRef<FInternetAddr> addr = USocketClientBPLibrary::getSocketSubSystemClientHandler()->CreateInternetAddr();
				bool bIsValid;
				addr->SetIp(*ip, bIsValid);
				addr->SetPort(port);
				if (bIsValid) {
					while (messageQueue.IsEmpty() == false) {
						FString m;
						messageQueue.Dequeue(m);
						FTCHARToUTF8 Convert(*m);
						int32 sent = 0;
						socket->SendTo((uint8*)((ANSICHAR*)Convert.Get()), Convert.Length(), sent, *addr);
					}

					while (byteArrayQueue.IsEmpty() == false) {
						TArray<uint8> ba;
						byteArrayQueue.Dequeue(ba);
						int32 sent = 0;
						socket->SendTo(ba.GetData(), ba.Num(), sent, *addr);
						ba.Empty();
					}

				}
				else {
					UE_LOG(LogTemp, Error, TEXT("UDP Connection fail."));
				}

			}

			if (run) {
				pauseThread(true);
				//workaround. suspend do not work on all platforms. lets sleep
				while (paused && run) {
					FPlatformProcess::Sleep(0.01);
				}
			}
		}


		if (socket != nullptr && socket != NULL) {
			USocketClientBPLibrary* socketClientTMP = socketClient;
			AsyncTask(ENamedThreads::GameThread, [socketClientTMP]() {
				if (socketClientTMP != nullptr)
					socketClientTMP->onsocketClientUDPConnectionEventDelegate.Broadcast(false, "UDP Connection has been closed.");
				});
			FPlatformProcess::Sleep(1);
			socket->Close();
			socket = NULL;
		}


		return 0;
	}

	FRunnableThread* getThread() {
		return thread;
	}

	void setThread(FRunnableThread* threadP) {
		thread = threadP;
	}

	void stopThread() {
		run = false;
		if (thread != nullptr) {
			pauseThread(false);
		}
	}

	bool isRun() {
		return run;
	}


	void setMessage(FString messageP, TArray<uint8> byteArrayP) {
		if (messageP.Len() > 0)
			messageQueue.Enqueue(messageP);
		if (byteArrayP.Num() > 0)
			byteArrayQueue.Enqueue(byteArrayP);
	}

	void sendMessage(FString messageP, TArray<uint8> byteArrayP) {
		if (messageP.Len() > 0)
			messageQueue.Enqueue(messageP);
		if (byteArrayP.Num() > 0)
			byteArrayQueue.Enqueue(byteArrayP);
		pauseThread(false);

	}

	void pauseThread(bool pause) {
		paused = pause;
		if (thread != nullptr)
			thread->Suspend(pause);
	}

protected:
	USocketClientBPLibrary* socketClient = nullptr;
	FRunnableThread* thread = nullptr;
	bool					run;
	bool					paused;
	FString					ipGlobal;
	int32					portGlobal;
	FSocket* socket = nullptr;
	FSocket* receiverSocket = nullptr;
	TQueue<FString> messageQueue;
	TQueue<TArray<uint8>> byteArrayQueue;

};