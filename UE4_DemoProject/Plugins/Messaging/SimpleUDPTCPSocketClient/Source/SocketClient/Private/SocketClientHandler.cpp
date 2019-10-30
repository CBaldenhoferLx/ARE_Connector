// Copyright 2017-2019 David Romanski (Socke). All Rights Reserved.

#include "SocketClientHandler.h"

USocketClientHandler* USocketClientHandler::socketClientHandler;

USocketClientHandler::USocketClientHandler(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	socketClientHandler = this;
}


USocketClientBPLibrary* USocketClientHandler::getSocketClientTarget() {
	//if it is empty than create defaut instance because backward compatibility 

	if (USocketClientHandler::socketClientHandler->TCP_ClientsMap.Num() == 0) {
		UPROPERTY()
			USocketClientBPLibrary* socketClientBPLibrary = NewObject<USocketClientBPLibrary>(USocketClientBPLibrary::StaticClass());
		socketClientBPLibrary->AddToRoot();
		USocketClientHandler::socketClientHandler->addTCPClientToMap("0.0.0.0", 0, socketClientBPLibrary);
		return socketClientBPLibrary;
	}

	//allways last one in the map
	TArray<FString> keys;
	USocketClientHandler::socketClientHandler->TCP_ClientsMap.GetKeys(keys);
	FClientSocketTCPStruct* sPointer = USocketClientHandler::socketClientHandler->TCP_ClientsMap.Find(keys[keys.Num() - 1]);
	if (sPointer != nullptr) {
		FClientSocketTCPStruct s = *sPointer;
		if (s.socketClientBPLibrary->IsValidLowLevel())
			return s.socketClientBPLibrary;
	}
	return nullptr;
}

void USocketClientHandler::getSocketClientTargetByIP_AndPort(const FString IP, const int32 Port, bool& found, USocketClientBPLibrary*& target) {
	found = false;
	if (IP.IsEmpty() || USocketClientHandler::socketClientHandler->TCP_ClientsMap.Num() == 0) {
		//UE_LOG(LogTemp, Warning, TEXT("getSocketClientTargetByIP_AndPort 1"));
		return;
	}

	FString key = IP + ":" + FString::FromInt(Port);
	FClientSocketTCPStruct* sPointer = USocketClientHandler::socketClientHandler->TCP_ClientsMap.Find(key);
	if (sPointer != nullptr) {

		FClientSocketTCPStruct s = *sPointer;
		if (s.socketClientBPLibrary->IsValidLowLevel()) {
			target = s.socketClientBPLibrary;
			found = true;
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("getSocketClientTargetByIP_AndPort 2"));
}

void USocketClientHandler::getSocketClientTargetByClientConnectionID(const FString ClientConnectionID, bool& found, USocketClientBPLibrary*& target) {
	getSocketClientTargetByIP_AndPort(ClientConnectionID, 0, found, target);
}

USocketClientBPLibrary* USocketClientHandler::getSocketClientTargetByIP_AndPortInternal(FString IP, int32 Port) {
	if (IP.IsEmpty()) {
		//UE_LOG(LogTemp, Warning, TEXT("getSocketClientTargetByIP_AndPortInternal 1"));
		return nullptr;
	}
	FString key = IP + ":" + FString::FromInt(Port);
	FClientSocketTCPStruct* sPointer = USocketClientHandler::socketClientHandler->TCP_ClientsMap.Find(key);
	if (sPointer != nullptr) {
		//UE_LOG(LogTemp, Warning, TEXT("getSocketClientTargetByIP_AndPortInternal 2"));
		FClientSocketTCPStruct s = *sPointer;
		if (s.socketClientBPLibrary->IsValidLowLevel())
			return s.socketClientBPLibrary;
	}
	//UE_LOG(LogTemp, Warning, TEXT("getSocketClientTargetByIP_AndPortInternal 3"));
	return nullptr;
}


void USocketClientHandler::removeSocketClientTargetByIP_AndPortInternal(FString IP, int32 Port) {
	if (IP.IsEmpty()) {
		return;
	}

	FString key = IP + ":" + FString::FromInt(Port);
	FClientSocketTCPStruct* sPointer = USocketClientHandler::socketClientHandler->TCP_ClientsMap.Find(key);
	if (sPointer != nullptr) {
		FClientSocketTCPStruct s = *sPointer;
		//s.socketClientBPLibrary->RemoveFromRoot();
		USocketClientHandler::socketClientHandler->TCP_ClientsMap.Remove(key);
	}
	return;
}

void USocketClientHandler::addTCPClientToMap(FString IP, int32 Port, USocketClientBPLibrary* client) {
	if (IP.IsEmpty()) {
		return;
	}
	FString key = IP + ":" + FString::FromInt(Port);
	FClientSocketTCPStruct s;
	s.ip = IP;
	s.port = Port;
	s.socketClientBPLibrary = client;

	USocketClientHandler::socketClientHandler->TCP_ClientsMap.Add(key, s);
}


void USocketClientHandler::changeSocketPlatform(ESocketPlatformClient platform) {
	USocketClientHandler::socketClientHandler->systemSocketPlatform = platform;
}

ISocketSubsystem* USocketClientHandler::getSocketSubSystem() {
	switch (USocketClientHandler::socketClientHandler->systemSocketPlatform)
	{
	case ESocketPlatformClient::E_SSC_SYSTEM:
		return ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	case ESocketPlatformClient::E_SSC_WINDOWS:
		return ISocketSubsystem::Get(FName(TEXT("WINDOWS")));
	case ESocketPlatformClient::E_SSC_MAC:
		return ISocketSubsystem::Get(FName(TEXT("MAC")));
	case ESocketPlatformClient::E_SSC_IOS:
		return ISocketSubsystem::Get(FName(TEXT("IOS")));
	case ESocketPlatformClient::E_SSC_UNIX:
		return ISocketSubsystem::Get(FName(TEXT("UNIX")));
	case ESocketPlatformClient::E_SSC_ANDROID:
		return ISocketSubsystem::Get(FName(TEXT("ANDROID")));
	case ESocketPlatformClient::E_SSC_PS4:
		return ISocketSubsystem::Get(FName(TEXT("PS4")));
	case ESocketPlatformClient::E_SSC_XBOXONE:
		return ISocketSubsystem::Get(FName(TEXT("XBOXONE")));
	case ESocketPlatformClient::E_SSC_HTML5:
		return ISocketSubsystem::Get(FName(TEXT("HTML5")));
	case ESocketPlatformClient::E_SSC_SWITCH:
		return ISocketSubsystem::Get(FName(TEXT("SWITCH")));
	case ESocketPlatformClient::E_SSC_DEFAULT:
		return ISocketSubsystem::Get();
	default:
		return ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	}
}