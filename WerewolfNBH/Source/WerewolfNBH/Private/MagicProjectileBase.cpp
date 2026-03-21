#include "MagicProjectileBase.h"

#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MagicHazardField.h"
#include "UObject/ConstructorHelpers.h"

AMagicProjectileBase::AMagicProjectileBase()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    SetRootComponent(CollisionSphere);
    CollisionSphere->InitSphereRadius(16.0f);
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionSphere->SetCollisionProfileName(TEXT("Projectile"));
    CollisionSphere->SetNotifyRigidBodyCollision(true);

    DisplayMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisplayMesh"));
    DisplayMesh->SetupAttachment(CollisionSphere);
    DisplayMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLight"));
    PointLight->SetupAttachment(CollisionSphere);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->ProjectileGravityScale = 0.0f;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMeshFinder.Succeeded())
    {
        DisplayMesh->SetStaticMesh(SphereMeshFinder.Object);
    }
}

void AMagicProjectileBase::BeginPlay()
{
    Super::BeginPlay();

    CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AMagicProjectileBase::OnProjectileOverlap);
    CollisionSphere->OnComponentHit.AddDynamic(this, &AMagicProjectileBase::OnProjectileHit);
}

void AMagicProjectileBase::InitializeFromConfig(const FMagicProjectileConfig& InConfig, AController* InInstigatorController)
{
    Config = InConfig;
    InstigatorController = InInstigatorController;

    ProjectileMovement->InitialSpeed = Config.ProjectileSpeed;
    ProjectileMovement->MaxSpeed = Config.ProjectileSpeed;

    CollisionSphere->SetSphereRadius(FMath::Max(8.0f, Config.ProjectileScale * 50.0f));
    DisplayMesh->SetRelativeScale3D(FVector(FMath::Max(0.05f, Config.ProjectileScale)));
    PointLight->SetLightColor(Config.SpellColor);
    PointLight->SetIntensity(Config.LightIntensity);
    PointLight->SetAttenuationRadius(FMath::Max(80.0f, Config.LightRadius));
    DisplayMesh->SetVectorParameterValueOnMaterials(TEXT("Color"), FVector(Config.SpellColor));
    DisplayMesh->SetVectorParameterValueOnMaterials(TEXT("Tint"), FVector(Config.SpellColor));
    SetLifeSpan(FMath::Max(0.1f, Config.ProjectileLifeSeconds));
}

void AMagicProjectileBase::OnProjectileOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != GetOwner())
    {
        HandleImpact(SweepResult, OtherActor);
    }
}

void AMagicProjectileBase::OnProjectileHit(
    UPrimitiveComponent* HitComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    FVector NormalImpulse,
    const FHitResult& Hit)
{
    if (OtherActor && OtherActor != GetOwner())
    {
        HandleImpact(Hit, OtherActor);
    }
}

void AMagicProjectileBase::HandleImpact(const FHitResult& HitResult, AActor* OtherActor)
{
    if (bHasImpacted)
    {
        return;
    }

    bHasImpacted = true;
    const FVector ImpactLocation = HitResult.ImpactPoint.IsNearlyZero() ? GetActorLocation() : FVector(HitResult.ImpactPoint);

    if (Config.ImpactRadius > KINDA_SMALL_NUMBER)
    {
        if (Config.DirectDamage > 0.0f)
        {
            UGameplayStatics::ApplyRadialDamage(
                this,
                Config.DirectDamage,
                ImpactLocation,
                Config.ImpactRadius,
                nullptr,
                TArray<AActor*>(),
                this,
                InstigatorController.Get(),
                true);
        }

        ApplyRadialImpulse(ImpactLocation);
        ApplyCharacterForce(ImpactLocation);
    }
    else if (OtherActor && Config.DirectDamage > 0.0f)
    {
        UGameplayStatics::ApplyDamage(OtherActor, Config.DirectDamage, InstigatorController.Get(), this, nullptr);
    }

    if (Config.LingeringDuration > 0.0f && Config.LingeringRadius > 0.0f)
    {
        TSubclassOf<AMagicHazardField> FieldClass = Config.LingeringFieldClass;
        if (!*FieldClass)
        {
            FieldClass = AMagicHazardField::StaticClass();
        }

        if (GetWorld() && *FieldClass)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = GetOwner();
            SpawnParams.Instigator = GetInstigator();
            if (AMagicHazardField* Field = GetWorld()->SpawnActor<AMagicHazardField>(FieldClass, ImpactLocation, FRotator::ZeroRotator, SpawnParams))
            {
                Field->InitializeFromConfig(Config, InstigatorController.Get());
            }
        }
    }

    Destroy();
}

void AMagicProjectileBase::ApplyRadialImpulse(const FVector& Origin) const
{
    if (!GetWorld() || Config.ImpactForce <= 0.0f || Config.ImpactRadius <= 0.0f)
    {
        return;
    }

    TArray<FOverlapResult> Overlaps;
    FCollisionShape Shape = FCollisionShape::MakeSphere(Config.ImpactRadius);
    FCollisionObjectQueryParams QueryParams;
    QueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
    QueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
    QueryParams.AddObjectTypesToQuery(ECC_Pawn);

    if (GetWorld()->OverlapMultiByObjectType(Overlaps, Origin, FQuat::Identity, QueryParams, Shape))
    {
        for (const FOverlapResult& Overlap : Overlaps)
        {
            if (UPrimitiveComponent* Primitive = Overlap.GetComponent())
            {
                if (Primitive->IsSimulatingPhysics())
                {
                    Primitive->AddRadialImpulse(Origin, Config.ImpactRadius, Config.ImpactForce, ERadialImpulseFalloff::RIF_Linear, true);
                }
            }
        }
    }
}

void AMagicProjectileBase::ApplyCharacterForce(const FVector& Origin) const
{
    if (!GetWorld() || Config.ImpactForce <= 0.0f || Config.ImpactRadius <= 0.0f)
    {
        return;
    }

    TArray<FOverlapResult> Overlaps;
    FCollisionShape Shape = FCollisionShape::MakeSphere(Config.ImpactRadius);
    FCollisionObjectQueryParams QueryParams;
    QueryParams.AddObjectTypesToQuery(ECC_Pawn);

    if (GetWorld()->OverlapMultiByObjectType(Overlaps, Origin, FQuat::Identity, QueryParams, Shape))
    {
        for (const FOverlapResult& Overlap : Overlaps)
        {
            if (ACharacter* Character = Cast<ACharacter>(Overlap.GetActor()))
            {
                const FVector Direction = (Character->GetActorLocation() - Origin).GetSafeNormal();
                Character->LaunchCharacter(Direction * Config.ImpactForce, true, true);
            }
        }
    }
}
