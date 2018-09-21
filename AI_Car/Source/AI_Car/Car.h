// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Containers/Array.h"
#include "NeuralNetwork.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Car.generated.h"

UCLASS()
class AI_CAR_API ACar : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACar();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* OurVisibleActor;

	UPROPERTY(EditAnywhere)
    USpringArmComponent* OurCameraSpringArm;

	UPROPERTY(EditAnywhere)
    UCameraComponent* OurCamera;

	UPROPERTY(EditAnywhere)
		UBoxComponent* BoxComponent;

	UPROPERTY(EditAnywhere)
		UBoxComponent* Component;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car",meta =(UIMin = "0.0", UIMax = "1.0"))
		int SmoothInput = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car")
		int StickNumber = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car")
		bool best = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car")
		float amplitude = 90.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car")
		float VelocityX = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car")
		float ActualVelocity = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car")
		float Aceleration = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car")
		float RotationVelocityPawn = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car")
		float MaxDistance = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "View")
		float camera_velocity = 0.30f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "View")
		float stick_velocity = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "View")
		float approach_velocity = 7.f;

	FCollisionQueryParams CollisionParams;

	UFUNCTION()
		void OnCompHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	NeuralNetwork nn;
	bool hit = false;

	bool isPossessed = false;

	void ResetMovement(FTransform transform,int initTarget);
	void Change();

	int lastTarget=0;

	float percentage;

	int laps=0;
	float last_time_lap = 0;

	float diversity = 0;
	float fitness = 0;

	void InitNet(TArray<int>topology);

	bool drawLine=false;

	void StartPossessing();
	void StopPossessing();
private:
	TArray<float> Input;
	TArray<float> result[2];

	float NNproportion;

	float deltatime;
	float time = 0;

	void UpdateStick();
	void UpdateLocation();
	void UpdateRotation();
	void UpdateCameraRotation();
	void UpdateStickRotation();
	void UpdateStickLength();

	FRotator CameraRotation;
	FRotator StickRotation;
	float RotationPlayer;

	FVector2D CameraInput;
	FVector2D StickInput;
	float StickDistance;

	void Input_Approach(float AxisValue);
	void Input_CameraPitch(float AxisValue);
	void Input_CameraYaw(float AxisValue);
	void Input_StickYaw(float AxisValue);
	void Input_StickPitch(float AxisValue);

	bool actual = false;
};

