// Copyright[2025][Scuola Superiore Sant'Anna]
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


#include "SimPRIVEGameInstance.h"

USimPRIVEGameInstance::USimPRIVEGameInstance()
{
    FString ServerIP;
    int32 ServerPort;

    if (!ReadIPConfig(ServerIP, ServerPort))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to read json IP configuration."));
        //return;
    }

	UE_LOG(LogTemp, Warning, TEXT("INITIALIZING ROSBRIDGE"));
	this->ROSBridgeServerProtocol = "tcp";
	this->bConnectToROS = true;
	this->bCheckHealth = true;
	this->bReconnect = true;
	this->ROSBridgeServerHosts = { ServerIP };
	this->ROSBridgeServerPorts = { ServerPort };
	this->ROSVersion = 2;
}

void USimPRIVEGameInstance::Init()
{
	UE_LOG(LogTemp, Warning, TEXT("calling Init"));
	Super::Init();
}


bool USimPRIVEGameInstance::ReadIPConfig(FString& OutIP, int32& OutPort)
{

    FString PluginName = TEXT("SimPRIVE"); // Replace with your actual plugin name
    FString PluginBaseDir = IPluginManager::Get().FindPlugin(PluginName)->GetBaseDir();
    FString FilePath = FPaths::Combine(PluginBaseDir, TEXT("Content/Config/IPConfig.json"));
    UE_LOG(LogTemp, Warning, TEXT("Reading IP configuration from %s"), *FilePath);

    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load IP config file."));
        return false;
    }

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        OutIP = JsonObject->GetStringField(TEXT("IPAddress"));
        OutPort = JsonObject->GetIntegerField(TEXT("Port"));
        return true;
    }

    UE_LOG(LogTemp, Error, TEXT("Failed to parse IP config JSON."));
    return false;
}
