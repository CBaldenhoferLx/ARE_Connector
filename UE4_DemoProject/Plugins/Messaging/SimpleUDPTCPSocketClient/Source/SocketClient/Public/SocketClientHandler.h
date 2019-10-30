// Copyright 2017-2019 David Romanski (Socke). All Rights Reserved.
#pragma once

#include "SocketClient.h"
#include "DNSClientSocketClient.h"
#include "SocketClientHandler.generated.h"


class USocketClientBPLibrary;


USTRUCT(BlueprintType)
struct FClientSocketTCPStruct
{
	GENERATED_USTRUCT_BODY()

		FString ip;
	int32	port;
	//FString sessionID;
	USocketClientBPLibrary* socketClientBPLibrary;
};

UENUM(BlueprintType)
enum class ESocketPlatformClient : uint8
{
	E_SSC_SYSTEM		UMETA(DisplayName = "System"),
	E_SSC_DEFAULT 		UMETA(DisplayName = "Auto"),
	E_SSC_WINDOWS		UMETA(DisplayName = "WINDOWS"),
	E_SSC_MAC			UMETA(DisplayName = "MAC"),
	E_SSC_IOS			UMETA(DisplayName = "IOS"),
	E_SSC_UNIX			UMETA(DisplayName = "UNIX"),
	E_SSC_ANDROID		UMETA(DisplayName = "ANDROID"),
	E_SSC_PS4			UMETA(DisplayName = "PS4"),
	E_SSC_XBOXONE		UMETA(DisplayName = "XBOXONE"),
	E_SSC_HTML5			UMETA(DisplayName = "HTML5"),
	E_SSC_SWITCH		UMETA(DisplayName = "SWITCH")

};

UCLASS(Blueprintable, BlueprintType)
class USocketClientHandler : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	//UPROPERTY()
	//	FString InaccessibleUProperty;

	static USocketClientHandler* socketClientHandler;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SocketClient")
		static USocketClientBPLibrary* getSocketClientTarget();

	/**
	*Use this only for the multi TCP connections.
	*@param DomainOrIP IP or domain of an existing TCP connection
	*@param port Port of an existing TCP connection
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SocketClient")
		static void getSocketClientTargetByIP_AndPort(const FString DomainOrIP, const int32 Port, bool& found, USocketClientBPLibrary*& target);

	/**
	*Use this only for the multi TCP connections.
	*@param ClientConnectionID
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SocketClient")
		static void getSocketClientTargetByClientConnectionID(const FString ClientConnectionID, bool& found, USocketClientBPLibrary*& target);

	/**
	*UE4 uses different socket connections. When Steam is active, Steam Sockets are used for all connections. This leads to problems if you want to use Steam but not Steam Sockets. Therefore you can change the sockets to "System".
	*@param ESocketPlatformServer System = Windows on Windows, Mac = Mac on Mac ect.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketClient")
		static void changeSocketPlatform(ESocketPlatformClient platform);


	static ISocketSubsystem* getSocketSubSystem();

	static USocketClientBPLibrary* getSocketClientTargetByIP_AndPortInternal(FString IP, int32 Port);
	static void removeSocketClientTargetByIP_AndPortInternal(FString IP, int32 Port);


	static void addTCPClientToMap(FString IP, int32 Port, USocketClientBPLibrary* client);

private:
	ESocketPlatformClient systemSocketPlatform;

protected:
	UPROPERTY()
		TMap<FString, FClientSocketTCPStruct> TCP_ClientsMap;

};

