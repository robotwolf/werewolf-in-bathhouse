#include "MagicCasterCharacter.h"

#include "Animation/AnimSequenceBase.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "MagicHoverComponent.h"
#include "MagicSpellWeaponBase.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AMagicCasterCharacter::AMagicCasterCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(GetRootComponent());
    CameraBoom->TargetArmLength = 420.0f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->SocketOffset = FVector(0.0f, 40.0f, 70.0f);

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    HoverComponent = CreateDefaultSubobject<UMagicHoverComponent>(TEXT("HoverComponent"));

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 380.0f;

    static ConstructorHelpers::FObjectFinder<USkeletalMesh> MannyMeshFinder(
        TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
    if (MannyMeshFinder.Succeeded())
    {
        GetMesh()->SetSkeletalMesh(MannyMeshFinder.Object);
        GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
        GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    }

    static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> LevitationFinder(
        TEXT("/Game/CombatMagicAnims/Animations/AS_Levitating.AS_Levitating"));
    if (LevitationFinder.Succeeded())
    {
        IdleLevitationAnimation = LevitationFinder.Object;
    }

    DefaultInputMapping = TSoftObjectPtr<UInputMappingContext>(FSoftObjectPath(TEXT("/Game/Input/IMC_Default.IMC_Default")));
    MouseLookInputMapping = TSoftObjectPtr<UInputMappingContext>(FSoftObjectPath(TEXT("/Game/Input/IMC_MouseLook.IMC_MouseLook")));
    WeaponInputMapping = TSoftObjectPtr<UInputMappingContext>(FSoftObjectPath(TEXT("/Game/Variant_Shooter/Input/IMC_Weapons.IMC_Weapons")));
    MoveAction = TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/Actions/IA_Move.IA_Move")));
    LookAction = TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/Actions/IA_Look.IA_Look")));
    JumpActionAsset = TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/Actions/IA_Jump.IA_Jump")));
    ShootAction = TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Variant_Shooter/Input/Actions/IA_Shoot.IA_Shoot")));
}

void AMagicCasterCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (GetMesh())
    {
        GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
        RestoreIdleAnimation();
    }

    AddMappingIfValid(DefaultInputMapping, 0);
    AddMappingIfValid(MouseLookInputMapping, 1);
    AddMappingIfValid(WeaponInputMapping, 2);

    if (!CurrentSpell && *StartingSpellClass)
    {
        EquipSpell(StartingSpellClass);
    }
}

void AMagicCasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (UInputAction* MoveInputAction = MoveAction.LoadSynchronous())
        {
            EnhancedInputComponent->BindAction(MoveInputAction, ETriggerEvent::Triggered, this, &AMagicCasterCharacter::Move);
        }
        if (UInputAction* LookInputAction = LookAction.LoadSynchronous())
        {
            EnhancedInputComponent->BindAction(LookInputAction, ETriggerEvent::Triggered, this, &AMagicCasterCharacter::Look);
        }
        if (UInputAction* JumpInputAction = JumpActionAsset.LoadSynchronous())
        {
            EnhancedInputComponent->BindAction(JumpInputAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
            EnhancedInputComponent->BindAction(JumpInputAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        }
        if (UInputAction* ShootInputAction = ShootAction.LoadSynchronous())
        {
            EnhancedInputComponent->BindAction(ShootInputAction, ETriggerEvent::Started, this, &AMagicCasterCharacter::StartCast);
        }
    }
}

bool AMagicCasterCharacter::EquipSpell(TSubclassOf<AMagicSpellWeaponBase> NewSpellClass)
{
    if (!*NewSpellClass || !GetWorld())
    {
        return false;
    }

    if (CurrentSpell)
    {
        CurrentSpell->Destroy();
        CurrentSpell = nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    CurrentSpell = GetWorld()->SpawnActor<AMagicSpellWeaponBase>(NewSpellClass, GetActorLocation(), GetActorRotation(), SpawnParams);
    if (CurrentSpell)
    {
        CurrentSpell->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
        return true;
    }

    return false;
}

FText AMagicCasterCharacter::GetCurrentSpellName() const
{
    return CurrentSpell ? CurrentSpell->GetSpellDisplayName() : FText::FromString(TEXT("No Spell"));
}

float AMagicCasterCharacter::GetCurrentSpellCooldownRemaining() const
{
    return CurrentSpell ? CurrentSpell->GetCooldownRemaining(GetWorld()) : 0.0f;
}

float AMagicCasterCharacter::GetCurrentSpellCooldownAlpha() const
{
    return CurrentSpell ? CurrentSpell->GetCooldownAlpha(GetWorld()) : 1.0f;
}

bool AMagicCasterCharacter::TryCastCurrentSpell()
{
    return CurrentSpell ? CurrentSpell->TryCast(this) : false;
}

bool AMagicCasterCharacter::HasSpellEquipped() const
{
    return CurrentSpell != nullptr;
}

bool AMagicCasterCharacter::PlayCastSequence(UAnimSequenceBase* Sequence, float PlayRate)
{
    if (!GetMesh() || !Sequence)
    {
        return false;
    }

    GetMesh()->PlayAnimation(Sequence, false);

    const float Duration = Sequence->GetPlayLength() / FMath::Max(0.05f, PlayRate);
    GetWorldTimerManager().ClearTimer(IdleRestoreTimerHandle);
    GetWorldTimerManager().SetTimer(IdleRestoreTimerHandle, this, &AMagicCasterCharacter::RestoreIdleAnimation, Duration, false);
    return true;
}

FTransform AMagicCasterCharacter::GetSpellSpawnTransform() const
{
    FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 100.0f + FVector(0.0f, 0.0f, 80.0f);
    if (GetMesh() && GetMesh()->DoesSocketExist(SpellSocketName))
    {
        SpawnLocation = GetMesh()->GetSocketLocation(SpellSocketName);
    }

    SpawnLocation += GetControlRotation().RotateVector(SpellSpawnOffset);
    return FTransform(GetControlRotation(), SpawnLocation, FVector::OneVector);
}

void AMagicCasterCharacter::Move(const FInputActionValue& Value)
{
    const FVector2D MovementVector = Value.Get<FVector2D>();
    if (!Controller)
    {
        return;
    }

    const FRotator Rotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
    const FVector ForwardDirection = FRotationMatrix(Rotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);

    AddMovementInput(ForwardDirection, MovementVector.Y);
    AddMovementInput(RightDirection, MovementVector.X);
}

void AMagicCasterCharacter::Look(const FInputActionValue& Value)
{
    const FVector2D LookAxisVector = Value.Get<FVector2D>();
    AddControllerYawInput(LookAxisVector.X);
    AddControllerPitchInput(LookAxisVector.Y);
}

void AMagicCasterCharacter::StartCast()
{
    TryCastCurrentSpell();
}

void AMagicCasterCharacter::RestoreIdleAnimation()
{
    if (GetMesh() && IdleLevitationAnimation)
    {
        GetMesh()->PlayAnimation(IdleLevitationAnimation, true);
    }
}

void AMagicCasterCharacter::AddMappingIfValid(const TSoftObjectPtr<UInputMappingContext>& Mapping, int32 Priority) const
{
    const APlayerController* PC = Cast<APlayerController>(Controller);
    if (!PC)
    {
        return;
    }

    if (const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            if (UInputMappingContext* LoadedMapping = Mapping.LoadSynchronous())
            {
                InputSubsystem->AddMappingContext(LoadedMapping, Priority);
            }
        }
    }
}
