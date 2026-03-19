#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WerewolfGameplayTagLibrary.generated.h"

UCLASS()
class WEREWOLFNBH_API UWerewolfGameplayTagLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category="WerewolfNBH|GameplayTags")
    static FGameplayTag MakeGameplayTagFromName(FName TagName, bool bErrorIfNotFound = false);

    UFUNCTION(BlueprintPure, Category="WerewolfNBH|GameplayTags")
    static FGameplayTagContainer MakeGameplayTagContainerFromNames(const TArray<FName>& TagNames, bool bErrorIfNotFound = false);
};
