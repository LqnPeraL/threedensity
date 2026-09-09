// Copyright 2026 HyperlinksSpace. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class ACombatPlayerController;

class SThreeDensityHUDWidget : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SThreeDensityHUDWidget) {}
		SLATE_ARGUMENT(TWeakObjectPtr<ACombatPlayerController>, Owner)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:

	TWeakObjectPtr<ACombatPlayerController> Owner;

	TSharedRef<SWidget> BuildPauseMenu();
	TSharedRef<SWidget> BuildControlsPanel();
	TSharedRef<SWidget> BuildSettingsPanel();
	TSharedRef<SWidget> BuildHealthBar();

	FReply OnResume();
	FReply OnQuit();
	FReply OnTabControls();
	FReply OnTabSettings();
	FReply OnPreset(int32 PresetIndex);
	FReply OnAutoDetect();
	FReply OnBenchmark();
	FReply OnToggleLumen();
	FReply OnToggleVSync();
	FReply OnFpsCap(int32 Cap);
	FReply OnDisplayMode(int32 Mode);
	FReply OnDismissTip();

	EVisibility GetPauseVisibility() const;
	EVisibility GetTipVisibility() const;
	EVisibility GetHintVisibility() const;
	EVisibility GetHealthBarVisibility() const;
	EVisibility GetControlsVisibility() const;
	EVisibility GetSettingsVisibility() const;
	FText GetTipText() const;
	FText GetHealthLabel() const;
	FText GetHardwareText() const;
	FText GetBenchmarkText() const;
	FText GetPresetLabel() const;
	FText GetLumenLabel() const;
	FText GetVSyncLabel() const;
	FText GetFpsLabel() const;
	FText GetResolutionLabel() const;
	TOptional<float> GetHealthPercent() const;

	FSlateColor NavColor(int32 Tab) const;
	FSlateColor PresetColor(int32 PresetIndex) const;
};
