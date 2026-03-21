#include "MagicHazardField.h"

#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AMagicHazardField::AMagicHazardField()
{
    PrimaryActorTick.bCanEverTick = false;

    DamageSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DamageSphere"));
    SetRootComponent(DamageSphere);
    DamageSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DamageSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    DamageSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLight"));
    PointLight->SetupAttachment(DamageSphere);
}

void AMagicHazardField::BeginPlay()
{
    Super::BeginPlay();

    GetWorldTimerManager().SetTimer(
        TickTimerHandle,
        this,
        &AMagicHazardField::ApplyHazardTick,
        FMath::Max(0.05f, Config.LingeringTickInterval),
        true);
}

void AMagicHazardField::InitializeFromConfig(const FMagicProjectileConfig& InConfig, AController* InInstigatorController)
{
    Config = InConfig;
    InstigatorController = InInstigatorController;

    const float Radius = Config.LingeringRadius > 0.0f ? Config.LingeringRadius : DefaultRadius;
    DamageSphere->SetSphereRadius(Radius);
    PointLight->SetLightColor(Config.SpellColor);
    PointLight->SetIntensity(Config.LightIntensity * 0.5f);
    PointLight->SetAttenuationRadius(FMath::Max(Radius * 1.25f, Config.LightRadius));
    SetLifeSpan(FMath::Max(0.1f, Config.LingeringDuration));
}

void AMagicHazardField::ApplyHazardTick()
{
    TArray<AActor*> OverlappingActors;
    DamageSphere->GetOverlappingActors(OverlappingActors, AActor::StaticClass());

    for (AActor* Actor : OverlappingActors)
    {
        if (!Actor || Actor == GetOwner())
        {
            continue;
        }

        if (Config.LingeringDamagePerTick > 0.0f)
        {
            UGameplayStatics::ApplyDamage(Actor, Config.LingeringDamagePerTick, InstigatorController.Get(), this, nullptr);
        }

        if (Config.LingeringForce > 0.0f)
        {
            const FVector Direction = (Actor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
            if (ACharacter* Character = Cast<ACharacter>(Actor))
            {
                Character->LaunchCharacter(Direction * Config.LingeringForce, true, true);
            }
        }
    }
}
