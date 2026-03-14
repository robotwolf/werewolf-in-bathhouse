#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "ButchDecorationMarkerComponent.generated.h"

class UBillboardComponent;

UENUM(BlueprintType)
enum class EButchDecorationMarkerType : uint8
{
    GenericProp,
    PipeLane,
    LeakCandidate,
    SteamVentCandidate,
    AudioPoint,
    WindowView
};

UCLASS(ClassGroup=(WerewolfBH), meta=(BlueprintSpawnableComponent))
class WEREWOLFNBH_API UButchDecorationMarkerComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UButchDecorationMarkerComponent();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Butch")
    TObjectPtr<UBillboardComponent> BillboardComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Butch")
    FName MarkerID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Butch")
    EButchDecorationMarkerType MarkerType = EButchDecorationMarkerType::GenericProp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Butch", meta=(ClampMin="0.0"))
    float Weight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Butch", meta=(ClampMin="0.0"))
    float Radius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Butch")
    bool bOptional = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Butch")
    bool bAllowSteamFx = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Butch")
    TArray<FName> SemanticTags;
};
