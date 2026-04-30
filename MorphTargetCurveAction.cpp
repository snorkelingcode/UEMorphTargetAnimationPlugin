#include "MorphTargetCurveAction.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

UMorphTargetCurveAction* UMorphTargetCurveAction::SetMorphTarget(
	UObject* WorldContextObject,
	USkeletalMeshComponent* Target,
	FName MorphTargetName,
	EMorphInterpCurve Curve,
	EMorphCurveDirection Direction,
	float Duration)
{
	UMorphTargetCurveAction* A = NewObject<UMorphTargetCurveAction>();
	A->TargetComp  = Target;
	A->MorphName   = MorphTargetName;
	A->CurveType   = Curve;
	A->Dir         = Direction;
	A->DurationSec = FMath::Max(Duration, KINDA_SMALL_NUMBER);
	A->WorldPtr    = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	return A;
}

void UMorphTargetCurveAction::Activate()
{
	if (!TargetComp.IsValid())
	{
		Finish(0.f);
		return;
	}

	ElapsedSec = 0.f;
	bActive = true;

	const float StartAlpha = (Dir == EMorphCurveDirection::Open) ? 0.f : 1.f;
	const float V = EvaluateCurve(CurveType, StartAlpha);
	TargetComp->SetMorphTarget(MorphName, V);
	OnUpdate.Broadcast(V);
}

void UMorphTargetCurveAction::Tick(float DeltaTime)
{
	if (!bActive) return;
	if (!TargetComp.IsValid()) { Finish(0.f); return; }

	ElapsedSec += DeltaTime;
	const float T = FMath::Clamp(ElapsedSec / DurationSec, 0.f, 1.f);
	const float Alpha = (Dir == EMorphCurveDirection::Open) ? T : (1.f - T);
	const float V = EvaluateCurve(CurveType, Alpha);

	TargetComp->SetMorphTarget(MorphName, V);
	OnUpdate.Broadcast(V);

	if (T >= 1.f) Finish(V);
}

void UMorphTargetCurveAction::Cancel()
{
	bActive = false;
	SetReadyToDestroy();
}

void UMorphTargetCurveAction::Finish(float FinalValue)
{
	bActive = false;
	OnFinished.Broadcast(FinalValue);
	SetReadyToDestroy();
}

TStatId UMorphTargetCurveAction::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMorphTargetCurveAction, STATGROUP_Tickables);
}

float UMorphTargetCurveAction::EvaluateCurve(EMorphInterpCurve Curve, float t)
{
	t = FMath::Clamp(t, 0.f, 1.f);

	switch (Curve)
	{
	case EMorphInterpCurve::Linear:
		return t;

	case EMorphInterpCurve::EaseIn:
		return t * t;

	case EMorphInterpCurve::EaseOut:
		return 1.f - (1.f - t) * (1.f - t);

	case EMorphInterpCurve::EaseInOut:
		return (t < 0.5f) ? 2.f * t * t
		                  : 1.f - 0.5f * FMath::Pow(-2.f * t + 2.f, 2.f);

	case EMorphInterpCurve::CubicInOut:
		return (t < 0.5f) ? 4.f * t * t * t
		                  : 1.f - 0.5f * FMath::Pow(-2.f * t + 2.f, 3.f);

	case EMorphInterpCurve::Sine:
		return -(FMath::Cos(PI * t) - 1.f) * 0.5f;

	case EMorphInterpCurve::Overshoot:
	{
		const float c1 = 1.70158f;
		const float c3 = c1 + 1.f;
		const float p  = t - 1.f;
		return 1.f + c3 * p * p * p + c1 * p * p;
	}

	case EMorphInterpCurve::Elastic:
	{
		if (t <= 0.f) return 0.f;
		if (t >= 1.f) return 1.f;
		const float c4 = (2.f * PI) / 3.f;
		return FMath::Pow(2.f, -10.f * t) * FMath::Sin((t * 10.f - 0.75f) * c4) + 1.f;
	}

	case EMorphInterpCurve::Bounce:
	{
		const float n1 = 7.5625f;
		const float d1 = 2.75f;
		if (t < 1.f / d1)        { return n1 * t * t; }
		else if (t < 2.f / d1)   { const float u = t - 1.5f   / d1; return n1 * u * u + 0.75f; }
		else if (t < 2.5f / d1)  { const float u = t - 2.25f  / d1; return n1 * u * u + 0.9375f; }
		else                     { const float u = t - 2.625f / d1; return n1 * u * u + 0.984375f; }
	}
	}

	return t;
}
