// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "IWebSocketServer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ServerWebSocketLiteSubsystem.generated.h"


UCLASS(config = ServerWebSocketLite)
class SERVERWEBSOCKETLITE_API UServerWebSocketLiteSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJsonRecieved, const FString&, Payload);

	UPROPERTY(BlueprintAssignable)
	FOnJsonRecieved OnJsonRecieved;

protected:

	void OnWebSocketClientConnected(INetworkingWebSocket* ClientWebSocket); // to the server.

	virtual void ReceivedRawPacket(void* Data, int32 Count);

	UPROPERTY(config)
	bool bUseSubsystem;

	UPROPERTY(config)
	uint32 WebSocketPort;

private:

	TUniquePtr<class IWebSocketServer> ServerWebSocket;

	/** Delegate for callbacks to GameThreadTick */
	FTSTicker::FDelegateHandle TickHandle;
};

