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
	Capsule->InitCapsuleSize(44.0f, 96.0f);
	Capsule->SetCollisionProfileName(TEXT("Pawn"));
	Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	Capsule->SetCanEverAffectNavigation(false);
	RootComponent = Capsule;

	Robe = CreatePart(TEXT("Robe"), Capsule);
	Collar = CreatePart(TEXT("Collar"), Capsule);
	Sash = CreatePart(TEXT("Sash"), Capsule);
	Head = CreatePart(TEXT("Head"), Capsule);
	Topknot = CreatePart(TEXT("Topknot"), Capsule);
	TopknotBase = CreatePart(TEXT("TopknotBase"), Capsule);
	Beard = CreatePart(TEXT("Beard"), Capsule);
	BeardLeft = CreatePart(TEXT("BeardLeft"), Capsule);
	BeardRight = CreatePart(TEXT("BeardRight"), Capsule);
	Mustache = CreatePart(TEXT("Mustache"), Capsule);
	BrowLeft = CreatePart(TEXT("BrowLeft"), Capsule);
	BrowRight = CreatePart(TEXT("BrowRight"), Capsule);
	LeftSleeve = CreatePart(TEXT("LeftSleeve"), Capsule);
	RightSleeve = CreatePart(TEXT("RightSleeve"), Capsule);
	Staff = CreatePart(TEXT("Staff"), Capsule);

	NameLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("NameLabel"));
	NameLabel->SetupAttachment(Capsule);
	NameLabel->SetHorizontalAlignment(EHTA_Center);
	NameLabel->SetVerticalAlignment(EVRTA_TextBottom);
	NameLabel->SetRelativeLocation(FVector(0.0f, 0.0f, 125.0f));
	NameLabel->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	NameLabel->WorldSize = 26.0f;
	NameLabel->SetTextRenderColor(FColor(255, 230, 180));
	NameLabel->SetText(DisplayName);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMesh(TEXT("/Engine/BasicShapes/Cone.Cone"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));

	if (CylinderMesh.Succeeded())
	{
		Robe->SetStaticMesh(CylinderMesh.Object);
		Collar->SetStaticMesh(CylinderMesh.Object);
		Sash->SetStaticMesh(CylinderMesh.Object);
		TopknotBase->SetStaticMesh(CylinderMesh.Object);
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
		BeardLeft->SetStaticMesh(ConeMesh.Object);
		BeardRight->SetStaticMesh(ConeMesh.Object);
	}
	if (CubeMesh.Succeeded())
	{
		Mustache->SetStaticMesh(CubeMesh.Object);
		BrowLeft->SetStaticMesh(CubeMesh.Object);
		BrowRight->SetStaticMesh(CubeMesh.Object);
	}

	// Elder Chinese martial-arts master silhouette
	Robe->SetRelativeLocation(FVector(0.0f, 0.0f, -8.0f));
	Robe->SetRelativeScale3D(FVector(0.58f, 0.58f, 0.98f));

	Collar->SetRelativeLocation(FVector(0.0f, 0.0f, 48.0f));
	Collar->SetRelativeScale3D(FVector(0.50f, 0.50f, 0.12f));

	Sash->SetRelativeLocation(FVector(0.0f, 0.0f, -2.0f));
	Sash->SetRelativeScale3D(FVector(0.62f, 0.62f, 0.09f));

	Head->SetRelativeLocation(FVector(0.0f, 0.0f, 72.0f));
	Head->SetRelativeScale3D(FVector(0.40f, 0.38f, 0.40f));

	TopknotBase->SetRelativeLocation(FVector(0.0f, 0.0f, 90.0f));
	TopknotBase->SetRelativeScale3D(FVector(0.10f, 0.10f, 0.08f));

	Topknot->SetRelativeLocation(FVector(0.0f, 0.0f, 98.0f));
	Topknot->SetRelativeScale3D(FVector(0.16f, 0.16f, 0.18f));

	// Long white beard + side locks
	Beard->SetRelativeLocation(FVector(14.0f, 0.0f, 46.0f));
	Beard->SetRelativeRotation(FRotator(185.0f, 0.0f, 0.0f));
	Beard->SetRelativeScale3D(FVector(0.26f, 0.20f, 0.72f));

	BeardLeft->SetRelativeLocation(FVector(10.0f, -10.0f, 52.0f));
	BeardLeft->SetRelativeRotation(FRotator(190.0f, -25.0f, 15.0f));
	BeardLeft->SetRelativeScale3D(FVector(0.12f, 0.10f, 0.40f));

	BeardRight->SetRelativeLocation(FVector(10.0f, 10.0f, 52.0f));
	BeardRight->SetRelativeRotation(FRotator(190.0f, 25.0f, -15.0f));
	BeardRight->SetRelativeScale3D(FVector(0.12f, 0.10f, 0.40f));

	Mustache->SetRelativeLocation(FVector(17.0f, 0.0f, 64.0f));
	Mustache->SetRelativeScale3D(FVector(0.20f, 0.07f, 0.035f));

	BrowLeft->SetRelativeLocation(FVector(16.0f, -8.0f, 78.0f));
	BrowLeft->SetRelativeRotation(FRotator(0.0f, 0.0f, 18.0f));
	BrowLeft->SetRelativeScale3D(FVector(0.08f, 0.035f, 0.025f));

	BrowRight->SetRelativeLocation(FVector(16.0f, 8.0f, 78.0f));
	BrowRight->SetRelativeRotation(FRotator(0.0f, 0.0f, -18.0f));
	BrowRight->SetRelativeScale3D(FVector(0.08f, 0.035f, 0.025f));

	// Wide sleeves in a calm ready stance
	LeftSleeve->SetRelativeLocation(FVector(6.0f, -32.0f, 22.0f));
	LeftSleeve->SetRelativeRotation(FRotator(5.0f, 0.0f, 62.0f));
	LeftSleeve->SetRelativeScale3D(FVector(0.20f, 0.20f, 0.50f));

	RightSleeve->SetRelativeLocation(FVector(6.0f, 32.0f, 22.0f));
	RightSleeve->SetRelativeRotation(FRotator(5.0f, 0.0f, -62.0f));
	RightSleeve->SetRelativeScale3D(FVector(0.20f, 0.20f, 0.50f));

	Staff->SetRelativeLocation(FVector(8.0f, 42.0f, 8.0f));
	Staff->SetRelativeRotation(FRotator(6.0f, 0.0f, 10.0f));
	Staff->SetRelativeScale3D(FVector(0.045f, 0.045f, 1.40f));

	Tags.Add(FName("Sensei"));
	Tags.Add(FName("NPC"));
}

void ACombatSensei::BeginPlay()
{
	Super::BeginPlay();

	NameLabel->SetText(DisplayName);

	UMaterialInterface* SourceMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	const FLinearColor RobeColor(0.40f, 0.06f, 0.04f);
	const FLinearColor BeardColor(0.90f, 0.89f, 0.85f);
	const FLinearColor HairColor(0.07f, 0.06f, 0.05f);

	ColorPart(Robe, RobeColor, SourceMat);
	ColorPart(Collar, FLinearColor(0.55f, 0.10f, 0.06f), SourceMat);
	ColorPart(Sash, FLinearColor(0.75f, 0.58f, 0.14f), SourceMat);
	ColorPart(Head, FLinearColor(0.84f, 0.68f, 0.54f), SourceMat);
	ColorPart(Topknot, HairColor, SourceMat);
	ColorPart(TopknotBase, HairColor, SourceMat);
	ColorPart(Beard, BeardColor, SourceMat);
	ColorPart(BeardLeft, BeardColor, SourceMat);
	ColorPart(BeardRight, BeardColor, SourceMat);
	ColorPart(Mustache, BeardColor, SourceMat);
	ColorPart(BrowLeft, FLinearColor(0.55f, 0.52f, 0.48f), SourceMat);
	ColorPart(BrowRight, FLinearColor(0.55f, 0.52f, 0.48f), SourceMat);
	ColorPart(LeftSleeve, RobeColor, SourceMat);
	ColorPart(RightSleeve, RobeColor, SourceMat);
	ColorPart(Staff, FLinearColor(0.30f, 0.17f, 0.08f), SourceMat);
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
