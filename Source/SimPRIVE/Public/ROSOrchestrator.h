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

#pragma once

#include "CoreMinimal.h"											//Cosas básicas de Unreal.
#include "GameFramework/Actor.h"									//Para crear un Actor
#include "Camera/CameraActor.h"										//Para poder tomar como referencia actor tipo Camera

#include "ROSIntegration/Classes/RI/Topic.h"						//Trae UTopic ? la clase que representa un topic ROS
#include "ROSIntegration/Classes/ROSIntegrationGameInstance.h"		//Puente con ROS
#include "ROSIntegration/Public/sensor_msgs/PointCloud2.h"
#include "ROSIntegration/Public/sensor_msgs/LaserScan.h"
#include "ROSIntegration/Public/sensor_msgs/Image.h"
#include "ROSIntegration/Public/nav_msgs/Odometry.h"
#include "ROSIntegration/Public/std_msgs/Bool.h"
#include "ROSIntegration/Public/std_msgs/Float32.h"
#include "ROSIntegration/Public/geometry_msgs/Quaternion.h"
#include "ROSIntegration/Public/geometry_msgs/Vector3.h"


#include "Rover.h"
#include "SpawningManager.h"


#include "ROSOrchestrator.generated.h"

UCLASS()															//Unreal reflection system macro.
class SIMPRIVE_API AROSOrchestrator : public AActor					//Class definition, name, from whom inherits
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AROSOrchestrator();												//Función del constructor

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;								//Llama al blueprint una vez al comienzo de la simu


	// Internal
	bool bProcessOdomMessages = false;								//Controla si se procesan los mensajes de odometría.
	bool JustResumed = false;										//Indica si la simulación se reanudó

	FTransform LastRealPose;
	FTransform CurrentOffset;										//offset entre los sistemas de coordenadas de ROS y Unreal.

	void InitEnvironment();											//Inicializo escena/spawning.

	void SubscribeToTopics();										//Escucho ROS topics.
	void AdvertiseTopics();											//Creo publishers

	//Métodos para tomar datos de UE y publicarlos en ROS
	void PublishImage(const TArray<FColor>& BitMap);
	void PublishPointCloud(const TArray<FVector>& PointCloudData);
	void PublishLaserScan(const TArray<float>& Ranges);
	void PublishCollision();
	//void PublishAdditionalInfo();

	void PauseGame();												//Pause y resume via mensajes de ROS 
	void ResumeGame();


	bool CheckSetup() const;										//Chequea si Rover y SpawnManager están asignados

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	//Editable Unreal Properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rover")
	ARover* Rover;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning Manager")
	ASpawningManager* SpawnManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene Camera")
	ACameraActor* Camera;

	// Subscribers and callbacks. 
	// Cada tópico tiene un topic name (FString), un puntero UTopic que envía data afuera de UE, y un callback
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Topic Names")
	FString OdometryTopicName;

	UPROPERTY()
	UTopic* OdometryTopic;
	void OdometryCallback(TSharedPtr<FROSBaseMsg> Msg);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Topic Names")
	FString ResetTopicName;

	UPROPERTY()
	UTopic* ResetTopic;
	void ResetCallback(TSharedPtr<FROSBaseMsg> Msg);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Topic Names")
	FString PauseSimTopicName;

	UPROPERTY()
	UTopic* PauseSimTopic;
	void PauseSimCallback(TSharedPtr<FROSBaseMsg> Msg);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Topic Names")
	FString ResumeSimTopicName;

	UPROPERTY()
	UTopic* ResumeSimTopic;
	void ResumeSimCallback(TSharedPtr<FROSBaseMsg> Msg);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Topic Names")
	FString CameraRotationTopicName;

	UPROPERTY()
	UTopic* CameraRotationTopic;
	void CameraRotationCallback(TSharedPtr<FROSBaseMsg> Msg);



	// Publishers
	UPROPERTY()
	UTopic* ImagePublisher;
	//TSharedPtr<UTopic> ImagePublisher;
	UPROPERTY()
	UTopic* LaserScanPublisher;

	UPROPERTY()
	UTopic* PointCloudPublisher;

	UPROPERTY()
	UTopic* CollisionPublisher;
	//TSharedPtr<UTopic> CollisionPublisher;
	UPROPERTY()
	UTopic* AdditionalInfoPublisher;

	UPROPERTY()
	UTopic* CameraRotationPublisher;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning Manager")
	int NumSpawnObjects = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning Manager")
	int NumSpawnPedestrians = 0;

};
