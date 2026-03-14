#include "ButchDecorationMarkerComponent.h"

#include "Components/BillboardComponent.h"

UButchDecorationMarkerComponent::UButchDecorationMarkerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    BillboardComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
    BillboardComponent->SetupAttachment(this);
    BillboardComponent->SetRelativeScale3D(FVector(0.35f));
}
