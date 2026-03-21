#include "MagicSpellWeaponBase.h"

#include "Animation/AnimSequenceBase.h"
#include "MagicCasterCharacter.h"
#include "MagicProjectileBase.h"
#include "TimerManager.h"

AMagicSpellWeaponBase::AMagicSpellWeaponBase()
{
    PrimaryActorTick.bCanEverTick = false;
    SetActorHiddenInGame(true);
}

bool AMagicSpellWeaponBase::TryCast(AMagicCasterCharacter* Caster)
{
    if (!IsValid(Caster))
    {
        return false;
    }

    const UWorld* World = GetWorld();
    if (!World || GetCooldownRemaining(World) > 0.0f)
    {
        return false;
    }

    PendingCaster = Caster;
    LastCastWorldTime = World->GetTimeSeconds();

    if (CastAnimation)
    {
        Caster->PlayCastSequence(CastAnimation, CastPlayRate);
    }

    GetWorldTimerManager().SetTimer(
        CastTimerHandle,
        this,
        &AMagicSpellWeaponBase::FinishCast,
        FMath::Max(0.0f, CastLeadTime),
        false);

    return true;
}

float AMagicSpellWeaponBase::GetCooldownRemaining(const UWorld* World) const
{
    if (!World)
    {
        return 0.0f;
    }

    return FMath::Max(0.0f, (LastCastWorldTime + CooldownSeconds) - World->GetTimeSeconds());
}

float AMagicSpellWeaponBase::GetCooldownAlpha(const UWorld* World) const
{
    if (CooldownSeconds <= KINDA_SMALL_NUMBER)
    {
        return 1.0f;
    }

    return 1.0f - (GetCooldownRemaining(World) / CooldownSeconds);
}

void AMagicSpellWeaponBase::FinishCast()
{
    AMagicCasterCharacter* Caster = PendingCaster.Get();
    if (!IsValid(Caster) || !Caster->GetWorld())
    {
        return;
    }

    TSubclassOf<AMagicProjectileBase> ClassToSpawn = ProjectileClass;
    if (!*ClassToSpawn)
    {
        ClassToSpawn = AMagicProjectileBase::StaticClass();
    }
    if (!*ClassToSpawn)
    {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Caster;
    SpawnParams.Instigator = Caster;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    if (AMagicProjectileBase* Projectile =
            Caster->GetWorld()->SpawnActor<AMagicProjectileBase>(ClassToSpawn, Caster->GetSpellSpawnTransform(), SpawnParams))
    {
        Projectile->InitializeFromConfig(ProjectileConfig, Caster->GetController());
    }
}
