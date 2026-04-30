#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Tickable.h"
#include "MorphTargetCurveAction.generated.h"

class USkeletalMeshComponent;

UENUM(BlueprintType)
enum class EMorphInterpCurve : uint8
{
	Linear      UMETA(DisplayName = "Linear"),
	EaseIn      UMETA(DisplayName = "Ease In (Quadratic)"),
	EaseOut     UMETA(DisplayName = "Ease Out (Quadratic)"),
	EaseInOut   UMETA(DisplayName = "Ease In Out (Quadratic)"),
	CubicInOut  UMETA(DisplayName = "Cubic In Out"),
	Sine        UMETA(DisplayName = "Sine In Out"),
	Overshoot   UMETA(DisplayName = "Back Out (Overshoot)"),
	Elastic     UMETA(DisplayName = "Elastic Out (Spring)"),
	Bounce      UMETA(DisplayName = "Bounce Out")
};

UENUM(BlueprintType)
enum class EMorphCurveDirection : uint8
{
	Open  UMETA(DisplayName = "Open (0 -> 1)"),
	Close UMETA(DisplayName = "Close (1 -> 0)")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMorphCurveValue, float, Value);

UCLASS()
class AGILE_API UMorphTargetCurveAction
	: public UBlueprintAsyncActionBase
	, public FTickableGameObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable) FMorphCurveValue OnUpdate;
	UPROPERTY(BlueprintAssignable) FMorphCurveValue OnFinished;

	UFUNCTION(BlueprintCallable,
		meta = (BlueprintInternalUseOnly = "true",
				WorldContext = "WorldContextObject",
				DisplayName  = "Set Morph Target",
				ToolTip      = "Animate a skeletal mesh morph target with a built-in interpolation curve over Duration. Direction picks Open (0->1) or Close (1->0)."),
		Category = "Animation|Morph Target")
	static UMorphTargetCurveAction* SetMorphTarget(
		UObject* WorldContextObject,
		USkeletalMeshComponent* Target,
		FName MorphTargetName,
		EMorphInterpCurve Curve,
		EMorphCurveDirection Direction = EMorphCurveDirection::Open,
		float Duration = 0.5f);

	virtual void Activate() override;

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override { return bActive; }
	virtual UWorld* GetTickableGameObjectWorld() const override { return WorldPtr.Get(); }
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Conditional; }

	UFUNCTION(BlueprintCallable, Category = "Animation|Morph Target")
	void Cancel();

	static float EvaluateCurve(EMorphInterpCurve Curve, float Alpha);

private:
	void Finish(float FinalValue);

	UPROPERTY() TWeakObjectPtr<USkeletalMeshComponent> TargetComp;
	TWeakObjectPtr<UWorld> WorldPtr;
	FName MorphName;
	float DurationSec = 0.5f;
	float ElapsedSec  = 0.f;
	EMorphInterpCurve CurveType = EMorphInterpCurve::EaseInOut;
	EMorphCurveDirection Dir    = EMorphCurveDirection::Open;
	bool bActive = false;
};
