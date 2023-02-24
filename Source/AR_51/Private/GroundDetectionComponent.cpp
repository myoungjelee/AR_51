// Fill out your copyright notice in the Description page of Project Settings.


#include "GroundDetectionComponent.h"
#include "ARBlueprintLibrary.h"
#include "AR_Player.h"
#include "MainWidget.h"
#include "Components/TextBlock.h"


UGroundDetectionComponent::UGroundDetectionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UGroundDetectionComponent::BeginPlay()
{
	Super::BeginPlay();
	
	player = Cast<AAR_Player>(GetOwner());
}


void UGroundDetectionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ���� Ÿ���� Gound�� ������ ����
	if (player->detectionType != EDetectionType::Ground)
	{
		return;
	}

	// ������ ��� ��� ������ �����´�.
	planes = UARBlueprintLibrary::GetAllGeometries();
	DetectGround();
}


void UGroundDetectionComponent::DebugGroundDetection()
{
	// �� ������ �ϰ� �ִ��� ������� �غ���.
	

	for (UARTrackedGeometry* plane : planes)
	{
		UARBlueprintLibrary::DebugDrawTrackedGeometry(plane, GetWorld(), FLinearColor(1.0f, 1.0f, 0, 1.0f));
	}
}

void UGroundDetectionComponent::CarControl()
{
	if (IsValid(spawnedCar))
	{
		bool isFirstTouch, isSecondTouch;
		FVector2D first, second;

		// �� �հ��� ��ġ�� üũ�Ѵ�.
		player->playerCon->GetInputTouchState(ETouchIndex::Touch1, first.X, first.Y, isFirstTouch);
		player->playerCon->GetInputTouchState(ETouchIndex::Touch2, second.X, second.Y, isSecondTouch);

		FString firstTouchResult = isFirstTouch ? FString("True") : FString("False");
		FString secondTouchResult = isSecondTouch ? FString("True") : FString("False");

		player->mainUI->logText2->SetText(FText::FromString(FString::Printf(TEXT("%s, %s"), *firstTouchResult, *secondTouchResult)));

		// �� �հ��� ������ �Ÿ��� ���� �ڵ��� �𵨸��� ũ�⸦ �ø��ų� ���̰� �ʹ�!
		float currentDist = FVector2D::Distance(first, second);

		if (isFirstTouch && isSecondTouch)
		{
			// ù ��° ����ġ��� �� �Ÿ� ���� �����صд�.
			if (prev_dist < 0)
			{
				prev_dist = currentDist;
			}
			else
			{
				// �հ��� �Ÿ��� ��ȭ����ŭ �ڵ��� �𵨸��� ũ�⸦ �����Ѵ�.
				float delta = currentDist - prev_dist;
				FVector newScale = spawnedCar->GetActorScale3D() + FVector(delta * 0.001f);

				// ũ�� ��ȭ�� 0.5�� ~ 2�� ���̷� �����Ѵ�.
				newScale.X = FMath::Clamp(newScale.X, 0.5f, 2.0f);
				newScale.Y = FMath::Clamp(newScale.Y, 0.5f, 2.0f);
				newScale.Z = FMath::Clamp(newScale.Z, 0.5f, 2.0f);

				spawnedCar->SetActorScale3D(newScale);
			}
		}
		else
		{
			// �հ����� ���� ���� prev_dist ���� -1�� �ٽ� �ʱ�ȭ�Ѵ�.
			prev_dist = -1;
		}
	}
}

void UGroundDetectionComponent::DetectGround()
{
	// ��ũ�� �߾�(�ػ� ����)���� AR ���� ȭ�鿡 ���� Ʈ���̽��� �߻��Ѵ�.
	float touch_x, touch_y;
	bool bIsTouchFirst = false;

	GetWorld()->GetFirstPlayerController()->GetInputTouchState(ETouchIndex::Touch1, touch_x, touch_y, bIsTouchFirst);


	// ���� Ʈ���̽��� ���� ������ �����ߴ� ���̶��...
	TArray<FARTraceResult> hitInfos = UARBlueprintLibrary::LineTraceTrackedObjects(FVector2D(vSize.X / 2, vSize.Y / 2));

	if (hitInfos.Num() > 0)
	{
		if (spawnedIndicator == nullptr)
		{
			// �� ��ġ�� �ε������� ���͸� �����Ѵ�.
			FActorSpawnParameters param;
			param.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			spawnedIndicator = GetWorld()->SpawnActor<AActor>(indicator, hitInfos[0].GetLocalToWorldTransform(), param);

		}
		else
		{
			spawnedIndicator->SetActorTransform(hitInfos[0].GetLocalToWorldTransform());
		}

		// �ε������Ͱ� �ִ� ���¿��� ȭ���� ��ġ���� �� �ε������� ��ġ�� �ڵ��� ���͸� �����Ѵ�.
		if (bIsTouchFirst)
		{
			FActorSpawnParameters param;
			param.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			// �ڵ��� ���� ���忡 ���� ������ �𵨸� ���͸� �����Ѵ�.
			if (spawnedCar == nullptr)
			{
				spawnedCar = GetWorld()->SpawnActor<AActor>(carModel, spawnedIndicator->GetTransform(), param);
			}
		}

		CarControl();
	}
	else
	{
		// �ε��������� ����� ������ �ʵ��� ó���Ѵ�.
		if (spawnedIndicator != nullptr)
		{
			spawnedIndicator->Destroy();
			spawnedIndicator = nullptr;
		}
	}
}