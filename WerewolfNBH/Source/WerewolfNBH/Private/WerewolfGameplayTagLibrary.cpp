#include "WerewolfGameplayTagLibrary.h"

#include "GameplayTagsManager.h"

FGameplayTag UWerewolfGameplayTagLibrary::MakeGameplayTagFromName(FName TagName, bool bErrorIfNotFound)
{
    if (TagName.IsNone())
    {
        return FGameplayTag();
    }

    return UGameplayTagsManager::Get().RequestGameplayTag(TagName, bErrorIfNotFound);
}

FGameplayTagContainer UWerewolfGameplayTagLibrary::MakeGameplayTagContainerFromNames(const TArray<FName>& TagNames, bool bErrorIfNotFound)
{
    FGameplayTagContainer Container;
    for (const FName& TagName : TagNames)
    {
        const FGameplayTag Tag = MakeGameplayTagFromName(TagName, bErrorIfNotFound);
        if (Tag.IsValid())
        {
            Container.AddTag(Tag);
        }
    }

    return Container;
}
