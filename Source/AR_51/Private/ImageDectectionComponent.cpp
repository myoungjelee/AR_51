// Fill out your copyright notice in the Description page of Project Settings.


#include "ImageDectectionComponent.h"
#include "AR_Player.h"
#include "ARBlueprintLibrary.h"
#include "MainWidget.h"
#include "Components/TextBlock.h"
#include "ARTrackable.h"
#include "Containers/Map.h"

UImageDectectionComponent::UImageDectectionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UImageDectectionComponent::BeginPlay()
{
	Super::BeginPlay();

	player = Cast<AAR_Player>(GetOwner());
	/*FOnTrackableRemovedDelegate dele;
	dele.BindUObject(this, &UImageDectectionComponent::TestFunc);
	UARBlueprintLibrary::AddOnTrackableRemovedDelegate_Handle(dele);*/
}


void UImageDectectionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// �̹��� ���ؼ� �߿��� ����ǵ��� ó��
	if (player != nullptr)
	{
		if (player->detectionType != EDetectionType::Image)
		{
			return;
		}
	}

	DetectImages();

}

void UImageDectectionComponent::DetectImages()
{
	TArray<UARTrackedImage*> trackedImages = UARBlueprintLibrary::GetAllGeometriesByClass<UARTrackedImage>();

	player->mainUI->logText1->SetText(FText::AsNumber(trackedImages.Num()));

	// ����, ������ �̹����� �ִٸ�...
	if (trackedImages.Num() > 0)
	{
		FString fName;

		// �ش� �̹����� Friendly Name�� �α׷� ����غ���.
		for (UARTrackedImage* image : trackedImages)
		{
			EARTrackingState curState = image->GetTrackingState();
			FString trackingState = UEnum::GetValueAsString(curState);

			//player->mainUI->logText3->SetText(FText::FromString(TEXT("I've tracking state...")));
			player->mainUI->logText3->SetText(FText::FromString(trackingState));
			UARCandidateImage* myImage = image->GetDetectedImage();

			if (myImage == nullptr)
			{
				player->mainUI->logText2->SetText(FText::FromString("Image NULL"));

			}
			else
			{
				// �̸��� �α׷� ����غ���.
				fName.Append(myImage->GetFriendlyName());
				fName.Append(TEXT(", "));
				player->mainUI->logText2->SetText(FText::FromString(fName));

				// �� �̹����� �ش��ϴ� �𵨸��� �����Ѵ�.
				if (trackingTable.Contains(myImage))
				{
					FString friendlyName = myImage->GetFriendlyName();
					FTransform trans = image->GetLocalToWorldTransform();

					if (spawnedData.Contains(friendlyName) == false && curState == EARTrackingState::Tracking)
					{
						TSubclassOf<AActor> model = trackingTable[myImage];
						AActor* spawnedModel = GetWorld()->SpawnActor<AActor>(model, trans);

						// �̸��� ������ ���� spawnedData ������ �ִ´�.
						spawnedData.Add(friendlyName, spawnedModel);
					}
					else if(spawnedData.Contains(friendlyName) && curState == EARTrackingState::Tracking)
					{
						// �� �̸��� �ش��ϴ� �𵨸��� Ʈ�������� �����Ѵ�.
						spawnedData[friendlyName]->SetActorTransform(trans);
					}
					// ���� �����Ǿ� �ְ�, Ʈ��ŷ ���´� �ƴҶ�
					else if (spawnedData.Contains(friendlyName) && curState == EARTrackingState::NotTracking)
					{
						// TMap���� �����ϰ�, �𵨸� ���͵� �����Ѵ�.
						AActor* removeActor = spawnedData[friendlyName];
						spawnedData.Remove(friendlyName);
						removeActor->Destroy();
					}
				}
			}
		}
	}
	// ������ ���ƴٸ�...
	else
	{
		player->mainUI->logText3->SetText(FText::FromString(TEXT("miss tracking")));

		for (TPair<FString, AActor*> actor : spawnedData)
		{
			AActor* spawnedActor = actor.Value;

			//player->mainUI->logText2->SetText(FText::FromString(FString::Printf(TEXT("%s"), *spawnedActor->GetName())));

			spawnedActor->Destroy();
		}
		spawnedData.Empty();
	}
}

void UImageDectectionComponent::TestFunc(UARTrackedGeometry* geo)
{
	UARTrackedImage* tImage = Cast<UARTrackedImage>(geo);
	FString name = tImage->GetDetectedImage()->GetFriendlyName();

	player->mainUI->logText2->SetText(FText::FromString(FString::Printf(TEXT("%s"), *name)));
	spawnedData[name]->Destroy();
}

