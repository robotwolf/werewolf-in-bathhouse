#pragma once

#include "Commandlets/Commandlet.h"
#include "CoreMinimal.h"
#include "CreateMagicStateTreeCommandlet.generated.h"

UCLASS()
class WEREWOLFNBHEDITOR_API UCreateMagicStateTreeCommandlet : public UCommandlet
{
    GENERATED_BODY()

public:
    UCreateMagicStateTreeCommandlet();

    virtual int32 Main(const FString& Params) override;

private:
    class UStateTree* LoadOrCreateStateTreeAsset(const FString& AssetPath) const;
    bool BuildMagicStateTree(class UStateTree& StateTree) const;
    bool SaveAsset(class UObject* Asset) const;
    void CompileBlueprintAsset(const TCHAR* AssetPath) const;
};
