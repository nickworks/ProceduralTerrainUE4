// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectCPPCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

AProjectCPPCharacter::AProjectCPPCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);


	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh(),FName("headSocket"));
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

}

void AProjectCPPCharacter::Tick(float dt)
{
	GameMode->UpdateSimulationLocation(GetActorLocation());
}

void AProjectCPPCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AProjectCPPCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AProjectCPPCharacter::MoveRight);

	// mouse look:
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	// controller look:
	PlayerInputComponent->BindAxis("TurnRate", this, &AProjectCPPCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AProjectCPPCharacter::LookUpAtRate);
	
	PlayerInputComponent->BindAction("Reload Level", EInputEvent::IE_Pressed, this, &AProjectCPPCharacter::ReloadLevel);
	PlayerInputComponent->BindAction("Throw Flare", EInputEvent::IE_Pressed, this, &AProjectCPPCharacter::ThrowFlare);
	PlayerInputComponent->BindAction("Toggle View", EInputEvent::IE_Pressed, this, &AProjectCPPCharacter::ChangeView);
	
	auto &ref = PlayerInputComponent->BindAction("Pause", IE_Pressed, this, &AProjectCPPCharacter::HandlePause);
	//ref.bExecuteWhenPaused = true;
}

void AProjectCPPCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (PauseWidgetClass) {
		PauseWidget = CreateWidget<UPauseWidget>(GetWorld()->GetFirstPlayerController(), PauseWidgetClass);
		PauseWidget->OnCloseWithApply.AddDynamic(this, &AProjectCPPCharacter::HandlePauseApply);
		PauseWidget->OnCloseWithCancel.AddDynamic(this, &AProjectCPPCharacter::HandlePauseCancel);
		
	}
	GameMode = Cast<AProjectCPPGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	OnDestroyed.AddDynamic(this, &AProjectCPPCharacter::OnDestroy);
}

void AProjectCPPCharacter::OnDestroy(AActor* actor)
{
	
}

void AProjectCPPCharacter::ReloadLevel()
{
	UWorld* world = GetWorld();
	UGameplayStatics::OpenLevel(world, FName(UGameplayStatics::GetCurrentLevelName(world)));
}

void AProjectCPPCharacter::ThrowFlare()
{
	if (!flareToSpawn) return;

	FRotator rot = FollowCamera->GetComponentRotation();
	
	FVector pos = GetActorLocation() + FRotator(0, rot.Yaw, 0).Vector() * 100;
	
	AActor* flare = GetWorld()->SpawnActor<AActor>(flareToSpawn, pos, FRotator::MakeFromEuler(FMath::VRand() * 360));

	if (UPrimitiveComponent* root = Cast<UPrimitiveComponent>(flare->GetRootComponent())) {
		rot.Pitch += 45;
		if (rot.Pitch > 90) rot.Pitch = 90;

		root->SetAllPhysicsLinearVelocity(rot.Vector() * 1000);
		root->SetAllPhysicsAngularVelocityInRadians(FMath::VRand());
	}


}

void AProjectCPPCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AProjectCPPCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AProjectCPPCharacter::ChangeView()
{
	static bool isFirstPerson = true;
	isFirstPerson = !isFirstPerson;
	
	if (isFirstPerson) {
		CameraBoom->TargetArmLength = 0;
		bUseControllerRotationYaw = true;
	}
	else {
		CameraBoom->TargetArmLength = 300;
		bUseControllerRotationYaw = false;
	}
}

void AProjectCPPCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AProjectCPPCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AProjectCPPCharacter::HandlePause()
{
	UGameplayStatics::SetGamePaused(GetWorld(), true);
	if (PauseWidget) {
		PauseWidget->AddToPlayerScreen();

		APlayerController *pc = GetWorld()->GetFirstPlayerController();

		FInputModeUIOnly mode;
		mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		
		pc->SetInputMode(mode);
		pc->SetShowMouseCursor(true);
	}
}
void AProjectCPPCharacter::HandlePauseApply(TArray<FSignalField> data) {

	HandlePauseCancel();
	GameMode->SetFields(data);
}
void AProjectCPPCharacter::HandlePauseCancel() {

	UGameplayStatics::SetGamePaused(GetWorld(), false);
	
	if (PauseWidget) PauseWidget->RemoveFromParent();

	APlayerController* pc = GetWorld()->GetFirstPlayerController();

	FInputModeGameOnly mode;
	mode.SetConsumeCaptureMouseDown(true);

	pc->SetInputMode(mode);
	pc->SetShowMouseCursor(false);

}