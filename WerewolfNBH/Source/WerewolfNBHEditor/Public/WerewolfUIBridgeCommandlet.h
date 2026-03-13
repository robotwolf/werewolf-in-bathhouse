#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "WerewolfUIBridgeCommandlet.generated.h"

class FJsonObject;

UCLASS()
class WEREWOLFNBHEDITOR_API UWerewolfUIBridgeCommandlet : public UCommandlet
{
    GENERATED_BODY()

public:
    UWerewolfUIBridgeCommandlet();
    virtual int32 Main(const FString& Params) override;

private:
    struct FBridgeResult
    {
        TArray<FString> Completed;
        TArray<FString> Warnings;
        TArray<FString> RequiresInteraction;
    };

    bool ProcessCommands(const FString& CommandsPath, FBridgeResult& OutResult) const;
    bool ProcessAction(const TSharedPtr<FJsonObject>& ActionObject, FBridgeResult& OutResult) const;
    bool ScaffoldHudRoot(const FString& AssetPath, FBridgeResult& OutResult) const;
    bool ScaffoldMeterTotemBase(const FString& AssetPath, FBridgeResult& OutResult) const;
    bool ScaffoldHudSupportWidgets(FBridgeResult& OutResult) const;
    bool ScaffoldWidgetPack(FBridgeResult& OutResult) const;
    bool ScaffoldScreenWipeFramework(const FString& AssetPath, FBridgeResult& OutResult) const;
    bool AuditWidget(const FString& AssetPath, FBridgeResult& OutResult) const;
    class UWidgetBlueprint* LoadOrCreateWidgetBlueprint(const FString& AssetPath) const;
    bool SaveAsset(UObject* Asset) const;
    void WriteResultFile(const FString& ResultPath, const FBridgeResult& Result, bool bSucceeded) const;
};
