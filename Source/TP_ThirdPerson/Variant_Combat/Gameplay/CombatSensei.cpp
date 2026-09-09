// Copyright Epic Games, Inc. All Rights Reserved.

#include "CombatSensei.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ACombatSensei::ACombatSensei()
{
	PrimaryActorTick.bCanEverTick = false;

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->InitCapsuleSize(42.0f, 96.0f);
	Capsule->SetCollisionProfileName(TEXT("Pawn"));
	Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	Capsule->SetCanEverAffectNavigation(false);
	RootComponent = Capsule;

	Robe = CreatePart(TEXT("Robe"), Capsule);
	Sash = CreatePart(TEXT("Sash"), Capsule);
	Head = CreatePart(TEXT("Head"), Capsule);
	Topknot = CreatePart(TEXT("Topknot"), Capsule);
	Beard = CreatePart(TEXT("Beard"), Capsule);
	Mustache = CreatePart(TEXT("Mustache"), Capsule);
	LeftSleeve = CreatePart(TEXT("LeftSleeve"), Capsule);
	RightSleeve = CreatePart(TEXT("RightSleeve"), Capsule);
	Staff = CreatePart(TEXT("Staff"), Capsule);

	NameLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("NameLabel"));
	NameLabel->SetupAttachment(Capsule);
	NameLabel->SetHorizontalAlignment(EHTA_Center);
	NameLabel->SetVerticalAlignment(EVRTA_TextBottom);
	NameLabel->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
	NameLabel->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	NameLabel->WorldSize = 28.0f;
	NameLabel->SetTextRenderColor(FColor(255, 230, 180));
	NameLabel->SetText(DisplayName);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMesh(TEXT("/Engine/BasicShapes/Cone.Cone"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));

	if (CylinderMesh.Succeeded())
	{
		Robe->SetStaticMesh(CylinderMesh.Object);
		Sash->SetStaticMesh(CylinderMesh.Object);
		LeftSleeve->SetStaticMesh(CylinderMesh.Object);
		RightSleeve->SetStaticMesh(CylinderMesh.Object);
		Staff->SetStaticMesh(CylinderMesh.Object);
	}
	if (SphereMesh.Succeeded())
	{
		Head->SetStaticMesh(SphereMesh.Object);
		Topknot->SetStaticMesh(SphereMesh.Object);
	}
	if (ConeMesh.Succeeded())
	{
		Beard->SetStaticMesh(ConeMesh.Object);
	}
	if (CubeMesh.Succeeded())
	{
		Mustache->SetStaticMesh(CubeMesh.Object);
	}

	// Proportions for an elder master: long robe, bald head + topknot, long beard, staff
	Robe->SetRelativeLocation(FVector(0.0f, 0.0f, -10.0f));
	Robe->SetRelativeScale3D(FVector(0.55f, 0.55f, 0.95f));

	Sash->SetRelativeLocation(FVector(0.0f, 0.0f, -5.0f));
	Sash->SetRelativeScale3D(FVector(0.58f, 0.58f, 0.08f));

	Head->SetRelativeLocation(FVector(0.0f, 0.0f, 70.0f));
	Head->SetRelativeScale3D(FVector(0.38f, 0.38f, 0.38f));

	Topknot->SetRelativeLocation(FVector(0.0f, 0.0f, 92.0f));
	Topknot->SetRelativeScale3D(FVector(0.14f, 0.14f, 0.18f));

	Beard->SetRelativeLocation(FVector(12.0f, 0.0f, 48.0f));
	Beard->SetRelativeRotation(FRotator(180.0f, 0.0f, 0.0f));
	Beard->SetRelativeScale3D(FVector(0.22f, 0.18f, 0.55f));

	Mustache->SetRelativeLocation(FVector(16.0f, 0.0f, 62.0f));
	Mustache->SetRelativeScale3D(FVector(0.18f, 0.06f, 0.04f));

	LeftSleeve->SetRelativeLocation(FVector(8.0f, -28.0f, 20.0f));
	LeftSleeve->SetRelativeRotation(FRotator(0.0f, 0.0f, 55.0f));
	LeftSleeve->SetRelativeScale3D(FVector(0.16f, 0.16f, 0.45f));

	RightSleeve->SetRelativeLocation(FVector(8.0f, 28.0f, 20.0f));
	RightSleeve->SetRelativeRotation(FRotator(0.0f, 0.0f, -55.0f));
	RightSleeve->SetRelativeScale3D(FVector(0.16f, 0.16f, 0.45f));

	Staff->SetRelativeLocation(FVector(10.0f, 38.0f, 10.0f));
	Staff->SetRelativeRotation(FRotator(8.0f, 0.0f, 12.0f));
	Staff->SetRelativeScale3D(FVector(0.05f, 0.05f, 1.35f));

	Tags.Add(FName("Sensei"));
	Tags.Add(FName("NPC"));
}

void ACombatSensei::BeginPlay()
{
	Super::BeginPlay();

	NameLabel->SetText(DisplayName);

	UMaterialInterface* SourceMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	ColorPart(Robe, FLinearColor(0.42f, 0.07f, 0.05f), SourceMat);       // deep crimson robe
	ColorPart(Sash, FLinearColor(0.72f, 0.55f, 0.12f), SourceMat);       // gold sash
	ColorPart(Head, FLinearColor(0.82f, 0.65f, 0.52f), SourceMat);       // warm skin
	ColorPart(Topknot, FLinearColor(0.08f, 0.07f, 0.06f), SourceMat);    // dark hair knot
	ColorPart(Beard, FLinearColor(0.85f, 0.84f, 0.80f), SourceMat);      // white beard
	ColorPart(Mustache, FLinearColor(0.78f, 0.76f, 0.72f), SourceMat);
	ColorPart(LeftSleeve, FLinearColor(0.38f, 0.06f, 0.04f), SourceMat);
	ColorPart(RightSleeve, FLinearColor(0.38f, 0.06f, 0.04f), SourceMat);
	ColorPart(Staff, FLinearColor(0.28f, 0.16f, 0.08f), SourceMat);      // wood staff
}

UStaticMeshComponent* ACombatSensei::CreatePart(FName Name, USceneComponent* Parent)
{
	UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(Name);
	Part->SetupAttachment(Parent);
	Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Part->SetGenerateOverlapEvents(false);
	Part->SetCastShadow(true);
	return Part;
}

void ACombatSensei::ColorPart(UStaticMeshComponent* Part, const FLinearColor& Color, UMaterialInterface* SourceMaterial)
{
	if (!Part)
	{
		return;
	}

	if (SourceMaterial)
	{
		if (UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(SourceMaterial, this))
		{
			MID->SetVectorParameterValue(TEXT("Color"), Color);
			Part->SetMaterial(0, MID);
			return;
		}
	}

	Part->SetVectorParameterValueOnMaterials(TEXT("Color"), FVector(Color));
}
