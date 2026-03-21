#include "MagicSpellPickup.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "MagicCasterCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMagicSpellPickup::AMagicSpellPickup()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
    SphereCollision->SetupAttachment(SceneRoot);
    SphereCollision->InitSphereRadius(120.0f);
    SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SphereCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
    SphereCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    BasePlate = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BasePlate"));
    BasePlate->SetupAttachment(SceneRoot);
    BasePlate->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BasePlate->SetRelativeScale3D(FVector(1.2f, 1.2f, 0.2f));

    DisplayMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisplayMesh"));
    DisplayMesh->SetupAttachment(SceneRoot);
    DisplayMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    DisplayMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 80.0f));
    DisplayMesh->SetRelativeScale3D(FVector(0.75f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMeshFinder.Succeeded())
    {
        DisplayMesh->SetStaticMesh(SphereMeshFinder.Object);
        BasePlate->SetStaticMesh(SphereMeshFinder.Object);
    }
}

void AMagicSpellPickup::BeginPlay()
{
    Super::BeginPlay();

    DisplayStartLocation = DisplayMesh->GetRelativeLocation();
    SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AMagicSpellPickup::HandleOverlap);
}

void AMagicSpellPickup::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    RunningTime += DeltaTime;

    if (DisplayMesh)
    {
        DisplayMesh->AddLocalRotation(FRotator(0.0f, RotationSpeed * DeltaTime, 0.0f));
        FVector HoverLocation = DisplayStartLocation;
        HoverLocation.Z += FMath::Sin(RunningTime * BobFrequency * 2.0f * PI) * BobAmplitude;
        DisplayMesh->SetRelativeLocation(HoverLocation);
    }
}

void AMagicSpellPickup::HandleOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!*SpellWeaponClass)
    {
        return;
    }

    if (AMagicCasterCharacter* Caster = Cast<AMagicCasterCharacter>(OtherActor))
    {
        TryGiveToCaster(Caster);
    }
}

bool AMagicSpellPickup::TryGiveToCaster(AMagicCasterCharacter* Caster)
{
    if (!Caster || !*SpellWeaponClass)
    {
        return false;
    }

    if (Caster->EquipSpell(SpellWeaponClass))
    {
        Destroy();
        return true;
    }

    return false;
}
