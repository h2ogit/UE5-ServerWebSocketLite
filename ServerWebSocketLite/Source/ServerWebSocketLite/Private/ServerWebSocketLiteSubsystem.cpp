 //Fill out your copyright notice in the Description page of Project Settings.

#include "ServerWebSocketLiteSubsystem.h"

#include "INetworkingWebSocket.h"
#include "IWebSocketNetworkingModule.h"
#include "WebSocketNetworkingDelegates.h"


bool UServerWebSocketLiteSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return bUseSubsystem;
}

void UServerWebSocketLiteSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	ServerWebSocket = FModuleManager::Get().LoadModuleChecked<IWebSocketNetworkingModule>(TEXT("WebSocketNetworking")).CreateServer();

	FWebSocketClientConnectedCallBack CallBack;
	CallBack.BindUObject(this, &UServerWebSocketLiteSubsystem::OnWebSocketClientConnected);

	if (!ServerWebSocket->Init(WebSocketPort, CallBack))
	{
		UE_LOG(LogClass, Warning, TEXT("ServerWebSocket Init FAIL"));
		ServerWebSocket.Reset();
		CallBack.Unbind();
		return;
	}

	TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([=](float Time)		
	{
		if (ServerWebSocket)
		{
			ServerWebSocket->Tick();
			return true;
		}
		else
		{
			return false;
		}
	}));
}

void UServerWebSocketLiteSubsystem::Deinitialize()
{
	ServerWebSocket = nullptr;

	if (TickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
		TickHandle.Reset();
	}
}

void UServerWebSocketLiteSubsystem::OnWebSocketClientConnected(INetworkingWebSocket* ClientWebSocket)
{
	FWebSocketPacketReceivedCallBack CallBack;
	CallBack.BindUObject(this, &UServerWebSocketLiteSubsystem::ReceivedRawPacket);

	ClientWebSocket->SetReceiveCallBack(CallBack);
}

void UServerWebSocketLiteSubsystem::ReceivedRawPacket(void* Data, int32 Count)
{
	if (Count == 0)   // nothing to process
	{
		return;
	}

	uint8* DataRef = reinterpret_cast<uint8*>(Data);

	FString JsonData = UTF8_TO_TCHAR(DataRef);

	if (JsonData.StartsWith(TEXT("{")))
	{
		int32 index = INDEX_NONE;
		JsonData.FindLastChar(*TEXT("}"), index);

		if (index != INDEX_NONE)
		{
			if (index < JsonData.Len() - 1)
			{
				JsonData.RemoveAt(index + 1, JsonData.Len() - index);
			}
			
			OnJsonRecieved.Broadcast(JsonData);
		}
	}
}
