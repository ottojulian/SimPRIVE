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


#include "ROSOrchestrator.h"

// Sets default values
AROSOrchestrator::AROSOrchestrator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.TickInterval = 0.1f;

}

// Called when the game starts or when spawned
void AROSOrchestrator::BeginPlay()
{
	Super::BeginPlay();

	if (CheckSetup())
	{
		bProcessOdomMessages = true;

		SubscribeToTopics();
		AdvertiseTopics();
	}
	
	
}

// Called every frame
void AROSOrchestrator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AROSOrchestrator::InitEnvironment()
{
	// TODO Spawner stuff
	SpawnManager->ResetSpawnProbabilities();


	FVector StartLocation = FVector::ZeroVector;
	FRotator StartRotation = FRotator::ZeroRotator;


	// Rover init
	Rover->ResetCollision();
	SpawnManager->GetRoverSpawnPoint(StartLocation, StartRotation);
	Rover->TryMove(StartLocation, StartRotation);

	// Compute the inverse of the initial pose
	FTransform PCurrent(StartRotation, StartLocation, FVector(1.0f, 1.0f, 1.0f));
	FTransform PInverse = LastRealPose.Inverse();

	// Compute the offset transform
	CurrentOffset = PInverse * PCurrent;

	SpawnManager->DeleteSpawnedObjects();
	for (int i = 0; i < NumSpawnPedestrians; i++)
	{
		SpawnManager->SpawnPedestrian();
	}
	for (int i = 0; i < NumSpawnObjects; i++)
	{
		SpawnManager->SpawnObject();

	}

	SpawnManager->SpawnQRCodeObject();
}

void AROSOrchestrator::SubscribeToTopics()
{
	UROSIntegrationGameInstance* rosinst = Cast<UROSIntegrationGameInstance>(GetGameInstance());
	if (rosinst)
	{
		if (!OdometryTopicName.IsEmpty())
		{
			OdometryTopic = NewObject<UTopic>(UTopic::StaticClass());
			OdometryTopic->Init(rosinst->ROSIntegrationCore, OdometryTopicName, TEXT("nav_msgs/Odometry"));
			OdometryTopic->Subscribe(std::bind(&AROSOrchestrator::OdometryCallback, this, std::placeholders::_1));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Empty Odometry topic name! Skipping callback init."));
		}

		if (!ResetTopicName.IsEmpty())
		{
			ResetTopic = NewObject<UTopic>(UTopic::StaticClass());
			ResetTopic->Init(rosinst->ROSIntegrationCore, ResetTopicName, TEXT("std_msgs/Bool"));
			ResetTopic->Subscribe(std::bind(&AROSOrchestrator::ResetCallback, this, std::placeholders::_1));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Empty Reset topic name! Skipping callback init."));
		}

		if (!PauseSimTopicName.IsEmpty())
		{
			PauseSimTopic = NewObject<UTopic>(UTopic::StaticClass());
			PauseSimTopic->Init(rosinst->ROSIntegrationCore, PauseSimTopicName, TEXT("std_msgs/Bool"));
			PauseSimTopic->Subscribe(std::bind(&AROSOrchestrator::PauseSimCallback, this, std::placeholders::_1));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Empty Pause topic name! Skipping callback init."));
		}

		if (!ResumeSimTopicName.IsEmpty())
		{
			ResumeSimTopic = NewObject<UTopic>(UTopic::StaticClass());
			ResumeSimTopic->Init(rosinst->ROSIntegrationCore, ResumeSimTopicName, TEXT("std_msgs/Bool"));
			ResumeSimTopic->Subscribe(std::bind(&AROSOrchestrator::ResumeSimCallback, this, std::placeholders::_1));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Empty Resume topic name! Skipping callback init."));
		}
		//Me suscribo al tópico de CameraRotation
		if (!CameraRotationTopicName.IsEmpty()) {
			CameraRotationTopic = NewObject<UTopic>(UTopic::StaticClass());
			CameraRotationTopic->Init(rosinst->ROSIntegrationCore, CameraRotationTopicName, TEXT("geometry_msgs/Vector3"));
			CameraRotationTopic->Subscribe(std::bind(&AROSOrchestrator::CameraRotationCallback, this, std::placeholders::_1));
		}
		else {
			UE_LOG(LogTemp, Error, TEXT("Empty Camera Rotation topic name! Skipping callback init."));
		}
	}
}



void AROSOrchestrator::AdvertiseTopics()
{
	UROSIntegrationGameInstance* rosinst = Cast<UROSIntegrationGameInstance>(GetGameInstance());
	if (rosinst)
	{
		ImagePublisher = NewObject<UTopic>(UTopic::StaticClass());
		//ImagePublisher = MakeShareable(NewObject<UTopic>(UTopic::StaticClass()));
		ImagePublisher->Init(rosinst->ROSIntegrationCore, TEXT("/rover/camera/image"), TEXT("sensor_msgs/Image"));
		ImagePublisher->Advertise();


		PointCloudPublisher = NewObject<UTopic>(UTopic::StaticClass());
		PointCloudPublisher->Init(rosinst->ROSIntegrationCore, TEXT("/rover/lidar/point_cloud"), TEXT("sensor_msgs/PointCloud2"));
		PointCloudPublisher->Advertise();

		//LaserScanPublisher = NewObject<UTopic>(UTopic::StaticClass());
		//LaserScanPublisher->Init(rosinst->ROSIntegrationCore, TEXT("/rover/lidar/laser_scan"), TEXT("sensor_msgs/LaserScan"));
		//LaserScanPublisher->Advertise();

		CollisionPublisher = NewObject<UTopic>(UTopic::StaticClass());
		//CollisionPublisher = MakeShareable(NewObject<UTopic>(UTopic::StaticClass()));
		CollisionPublisher->Init(rosinst->ROSIntegrationCore, TEXT("/rover/collision"), TEXT("std_msgs/Bool"));
		CollisionPublisher->Advertise();

		AdditionalInfoPublisher = NewObject<UTopic>(UTopic::StaticClass());
		//AdditionalInfoPublisher = MakeShareable(NewObject<UTopic>(UTopic::StaticClass()));
		AdditionalInfoPublisher->Init(rosinst->ROSIntegrationCore, TEXT("/rover/additional_info"), TEXT("std_msgs/Float32"));
		AdditionalInfoPublisher->Advertise();

		CameraRotationPublisher = NewObject<UTopic>(UTopic::StaticClass());
		CameraRotationPublisher->Init(rosinst->ROSIntegrationCore, TEXT("/rover/cam_rotation"), TEXT("geometry_msgs/Vector3"));
		CameraRotationPublisher->Advertise();
	}
}


void AROSOrchestrator::PauseGame()
{
}

void AROSOrchestrator::ResumeGame()
{
}

bool AROSOrchestrator::CheckSetup() const
{
	if (!this->Rover || !this->SpawnManager)
	{
		UE_LOG(LogTemp, Error, TEXT("SETUP ERROR! Please assign Rover and SpawnManager."));
		return false;

	}
	return true;
}




/*
									PUBLISHERS
*/




void AROSOrchestrator::PublishImage(const TArray<FColor>& BitMap)
{
	if (!ImagePublisher)
	{
		return;
	}

	// Convert Bitmap to ROS2 Image message
	TSharedPtr<ROSMessages::sensor_msgs::Image> ImageMsg = MakeShareable(new ROSMessages::sensor_msgs::Image());
	//sensor_msgs::Image ImageMsg;
	ImageMsg->width = Rover->Camera->GetImageWidth();
	ImageMsg->height = Rover->Camera->GetImageHeight();
	ImageMsg->encoding = "bgra8";
	ImageMsg->step = 4 * Rover->Camera->GetImageWidth();;
	
	// Create a new array for the image data
	TArray<uint8> ImageData;
	ImageData.SetNumUninitialized(BitMap.Num() * 4);

	// Populate the new array
	for (int32 i = 0; i < BitMap.Num(); ++i)
	{
		ImageData[i * 4 + 0] = BitMap[i].B;
		ImageData[i * 4 + 1] = BitMap[i].G;
		ImageData[i * 4 + 2] = BitMap[i].R;
		ImageData[i * 4 + 3] = BitMap[i].A;
	}

	// Assign the new array to the data field
	ImageMsg->data = ImageData.GetData();
	//try
	{
		if (ImagePublisher)
		{
			ImagePublisher->Publish(ImageMsg);
		}
	}

}

void AROSOrchestrator::PublishPointCloud(const TArray<FVector>& PointCloudData)
{
	// noise addition
	//std::normal_distribution<float> distribution(0.0f, 0.03f); // Mean 0, Standard deviation 1

	UWorld* World = GetWorld();
	TSharedPtr<ROSMessages::sensor_msgs::PointCloud2> PointCloudMsg = MakeShareable(new ROSMessages::sensor_msgs::PointCloud2());
	PointCloudMsg->header.time = FROSTime::Now();
	PointCloudMsg->header.frame_id = "lidar";

	// Set the point cloud data (example with dummy data)
	PointCloudMsg->height = 1;
	PointCloudMsg->width = PointCloudData.Num();
	PointCloudMsg->is_dense = false;
	PointCloudMsg->is_bigendian = false;
	PointCloudMsg->point_step = 12; // 3 fields * 4 bytes per float
	PointCloudMsg->row_step = PointCloudMsg->point_step * PointCloudMsg->width;

	// Define the fields (x, y, z)
	PointCloudMsg->fields.SetNum(3);
	PointCloudMsg->fields[0].name = "x";
	PointCloudMsg->fields[0].offset = 0;
	PointCloudMsg->fields[0].datatype = ROSMessages::sensor_msgs::PointCloud2::PointField::FLOAT32;
	PointCloudMsg->fields[0].count = 1;

	PointCloudMsg->fields[1].name = "y";
	PointCloudMsg->fields[1].offset = 4;
	PointCloudMsg->fields[1].datatype = ROSMessages::sensor_msgs::PointCloud2::PointField::FLOAT32;
	PointCloudMsg->fields[1].count = 1;

	PointCloudMsg->fields[2].name = "z";
	PointCloudMsg->fields[2].offset = 8;
	PointCloudMsg->fields[2].datatype = ROSMessages::sensor_msgs::PointCloud2::PointField::FLOAT32;
	PointCloudMsg->fields[2].count = 1;

	// Set the data (example with dummy data)
	int PointCloudSize = PointCloudData.Num();
	static uint8 Data[129600]; // 10800 points * 12 bytes per point
	float* DataPtr = reinterpret_cast<float*>(Data);
	int point_count = 0;
	FTransform LiDARTransform = Rover->Lidar->GetActorTransform();

	FVector Location = LiDARTransform.GetLocation();
	FRotator Rotation = LiDARTransform.GetRotation().Rotator();
	FVector Scale = LiDARTransform.GetScale3D();

	


	for (int i = 0; i < PointCloudSize; ++i)
	{
		DataPtr[i * 3 + 0] = PointCloudData[i].X / 100.; // +distribution(generator); // x
		DataPtr[i * 3 + 1] = -PointCloudData[i].Y / 100.; // +distribution(generator); // y
		DataPtr[i * 3 + 2] = PointCloudData[i].Z / 100.; // +distribution(generator); // z
		point_count++;
		
		//FVector DebugPt = LiDARTransform.GetRotation().RotateVector(PointCloudData[i]) + LiDARTransform.GetLocation();
		//DrawDebugPoint(GetWorld(), DebugPt, 10.f, FColor::Red, false, 1.0f);

	}
	PointCloudMsg->data_ptr = Data;

	// Publish the message
	PointCloudPublisher->Publish(PointCloudMsg);

}



void AROSOrchestrator::PublishLaserScan(const TArray<float>& Ranges)
{
	//UWorld* World = GetWorld();
	TSharedPtr<ROSMessages::sensor_msgs::LaserScan> LaserScanMsg = MakeShareable(new ROSMessages::sensor_msgs::LaserScan());
	LaserScanMsg->header.time = FROSTime::Now();
	LaserScanMsg->header.frame_id = "laser_frame";

	LaserScanMsg->angle_min = FMath::DegreesToRadians(-179.0f);
	LaserScanMsg->angle_max = FMath::DegreesToRadians(180.0f);
	LaserScanMsg->angle_increment = FMath::DegreesToRadians(1.0f);
	LaserScanMsg->time_increment = 0.1f;
	LaserScanMsg->scan_time = 0.1f;
	LaserScanMsg->range_min = 0.2f;
	LaserScanMsg->range_max = 25.0f;

	LaserScanMsg->ranges.SetNum(Ranges.Num());
	for (int i = 0; i < Ranges.Num(); ++i)
	{
		int correctIdx = Ranges.Num() - 1 - i;
		LaserScanMsg->ranges[i] = Ranges[correctIdx] / 100.0f;
	}

	// Publish the message
	LaserScanPublisher->Publish(LaserScanMsg);
}


void AROSOrchestrator::PublishCollision()
{
	TSharedPtr<ROSMessages::std_msgs::Bool> BoolMsg = MakeShareable(new ROSMessages::std_msgs::Bool());
	BoolMsg->_Data = Rover->GetCollision();
	//try
	{
		if (CollisionPublisher)
		{
			CollisionPublisher->Publish(BoolMsg);
		}
	}
	//catch (const std::exception& e)
	//{
	//	// Handle the exception, log the error message
	//	UE_LOG(LogTemp, Error, TEXT("Exception occurred: %s"), e.what());
	//}


}






/*
									CALLBACKS
*/
void AROSOrchestrator::OdometryCallback(TSharedPtr<FROSBaseMsg> Msg)
{
	if (!bProcessOdomMessages)
	{
		UE_LOG(LogTemp, Warning, TEXT("RECURSIVE CALL AVOIDED IN ODOMETRY MESSAGE! SKIPPING"));

		return;
	}

	if (Msg == nullptr || !Msg.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("INVALID ODOMETRY MESSAGE! SKIPPING"));
		return;

	}

	// Convert message
	TSharedPtr<ROSMessages::nav_msgs::Odometry> OdometryMsg_ = StaticCastSharedPtr<ROSMessages::nav_msgs::Odometry>(Msg);
	ROSMessages::nav_msgs::Odometry OdometryMsg = *OdometryMsg_;

	// Disable processing of incoming messages
	bProcessOdomMessages = false;

	// Callbacks are async. Must start a Game Thread

	

	AsyncTask(ENamedThreads::GameThread, [this, OdometryMsg]
		{
			FVector Pos(OdometryMsg.pose.pose.position.x * 100.f, -OdometryMsg.pose.pose.position.y * 100.f, Rover->Height); // OdometryMsg->pose.pose.position.z * 100.f);
			FQuat OrientQuat(OdometryMsg.pose.pose.orientation.x, OdometryMsg.pose.pose.orientation.y, OdometryMsg.pose.pose.orientation.z, OdometryMsg.pose.pose.orientation.w);
			FRotator Rot = OrientQuat.Rotator();
			Rot.Yaw = -Rot.Yaw;
			//double Yaw = OrientRot.Yaw;

			LastRealPose.SetLocation(Pos);
			LastRealPose.SetRotation(FQuat(Rot));

			if (JustResumed)
			{
				//UE_LOG(LogTemp, Warning, TEXT("Updated OFFSET!!!!!!!"));
				FTransform PCurrent(Rover->GetActorRotation(), Rover->GetActorLocation(), FVector(1.0f, 1.0f, 1.0f));
				FTransform PInverse = LastRealPose.Inverse();

				// Compute the offset transform
				CurrentOffset = PInverse * PCurrent;
				JustResumed = false;
			}

			// Apply transform to location
			FVector TransformedLocation = CurrentOffset.TransformPosition(Pos);

			// Apply transform to rotation
			FQuat OriginalQuat = FQuat(Rot);
			FQuat TransformedQuat = CurrentOffset.TransformRotation(OriginalQuat);
			FRotator TransformedRotation = TransformedQuat.Rotator();

			Rover->TryMove(TransformedLocation, TransformedRotation);

			TArray<FColor> BitMap;
			TArray<FVector> PointCloudData;
			//TArray<float> LaserScanData;
			//ENQUEUE_RENDER_COMMAND(MyCommand)(
				//[this, &BitMap](FRHICommandListImmediate& RHICmdList)
				//{
					// Safe render thread operations
			//if (BitMap.Num() > 0)
			//{
				Rover->CaptureImage(BitMap);
			//}
				//});
			Rover->CapturePointCloud(PointCloudData);
			//Rover->CaptureLaserScan(LaserScanData);
			
			//PublishAdditionalInfo();
			PublishImage(BitMap);
			PublishPointCloud(PointCloudData);
			//PublishLaserScan(LaserScanData);
			PublishCollision();
			
			// reenable odom msgs processing
			this->bProcessOdomMessages = true;
		}

	);

}

void AROSOrchestrator::ResetCallback(TSharedPtr<FROSBaseMsg> Msg)
{
	// Receive bool 
	if (!Msg.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("INVALID RESET MESSAGE! SKIPPING"));
		return;

	}
	TSharedPtr<ROSMessages::std_msgs::Bool> BoolMsg = StaticCastSharedPtr<ROSMessages::std_msgs::Bool>(Msg);

	bool doReset = false;
	if (BoolMsg.IsValid())
	{
		doReset = BoolMsg->_Data;
		UE_LOG(LogTemp, Warning, TEXT("***RECEIVED RESET TRIGGER!***"));

	}

	if (doReset)
	{
		bProcessOdomMessages = false;


		AsyncTask(ENamedThreads::GameThread, [this]
			{
				InitEnvironment();
				UE_LOG(LogTemp, Warning, TEXT("***Environment initialized***"));

				TArray<FColor> BitMap;
				TArray<FVector> PointCloudData;
				//TArray<float> LaserScanData;
				//ENQUEUE_RENDER_COMMAND(MyCommand)(
					//[this, &BitMap](FRHICommandListImmediate& RHICmdList)
					//{
						// Safe render thread operations
				if (BitMap.Num() > 0)
				{
					Rover->CaptureImage(BitMap);
				}
					//});
				Rover->CapturePointCloud(PointCloudData);
				//Rover->CaptureLaserScan(LaserScanData);

				//PublishAdditionalInfo();
				PublishImage(BitMap);
				PublishPointCloud(PointCloudData);
				//PublishLaserScan(LaserScanData);
				PublishCollision();

				//SubscribeToTopics();
			});


		bProcessOdomMessages = true;
	}
}

void AROSOrchestrator::PauseSimCallback(TSharedPtr<FROSBaseMsg> Msg)
{
	// Receive bool for pause
	if (!Msg.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("INVALID RESET MESSAGE! SKIPPING"));
		return;

	}
	TSharedPtr<ROSMessages::std_msgs::Bool> BoolMsg = StaticCastSharedPtr<ROSMessages::std_msgs::Bool>(Msg);

	bool doPause = false;
	if (BoolMsg.IsValid())
	{
		doPause = BoolMsg->_Data;
		UE_LOG(LogTemp, Warning, TEXT("***RECEIVED PAUSE TRIGGER!***"));

	}

	// Pause simulation
	if (doPause)
	{
		bProcessOdomMessages = false;
		UE_LOG(LogTemp, Warning, TEXT("***Stopped odom messages***"));
		// Call the Unreal Engine functions on the game thread
		AsyncTask(ENamedThreads::GameThread, [this]
			{
				PauseGame();
			});
	}
}

void AROSOrchestrator::ResumeSimCallback(TSharedPtr<FROSBaseMsg> Msg)
{

	// Receive bool for pause
	if (!Msg.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("INVALID RESET MESSAGE! SKIPPING"));
		return;

	}
	TSharedPtr<ROSMessages::std_msgs::Bool> BoolMsg = StaticCastSharedPtr<ROSMessages::std_msgs::Bool>(Msg);

	bool doResume = false;
	if (BoolMsg.IsValid())
	{
		doResume = BoolMsg->_Data;
		UE_LOG(LogTemp, Warning, TEXT("***RECEIVED RESUME TRIGGER!***"));

	}

	// Pause simulation
	if (doResume)
	{
		UE_LOG(LogTemp, Warning, TEXT("***RESUMING ODOM MESSAGES!***"));


		AsyncTask(ENamedThreads::GameThread, [this]
			{
				ResumeGame();
			});

		JustResumed = true;
		bProcessOdomMessages = true;
		// Call the Unreal Engine functions on the game thread

	}
}

void AROSOrchestrator::CameraRotationCallback(TSharedPtr<FROSBaseMsg> Msg) {
	if (!Msg.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid CameraRotation message"));
		return;
	}

	// Cast al tipo correcto
	TSharedPtr<ROSMessages::geometry_msgs::Vector3> VecMsg =
		StaticCastSharedPtr<ROSMessages::geometry_msgs::Vector3>(Msg);

	if (!VecMsg.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraRotation cast failed"));
		return;
	}

	float Roll = VecMsg->x;
	float Pitch = VecMsg->y;
	float Yaw = VecMsg->z;

	// IMPORTANTE: tocar actores solo en el GameThread
	AsyncTask(ENamedThreads::GameThread, [this, Roll, Pitch, Yaw]()
		{
			if (!Camera)
			{
				UE_LOG(LogTemp, Error, TEXT("Camera not assigned in ROSOrchestrator"));
				return;
			}

			FRotator NewRot(Pitch, Yaw, Roll);
			Camera->SetActorRotation(NewRot);
		});
}



