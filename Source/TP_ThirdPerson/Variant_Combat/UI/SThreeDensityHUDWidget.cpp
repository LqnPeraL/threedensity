// Copyright 2026 HyperlinksSpace. All Rights Reserved.

#include "SThreeDensityHUDWidget.h"
#include "Variant_Combat/CombatPlayerController.h"
#include "Variant_Combat/CombatCharacter.h"
#include "ThreeDensityGameUserSettings.h"
#include "ThreeDensityGameInstance.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Images/SImage.h"
#include "ThreeDensityLogo.h"
#include "Engine/Engine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformMisc.h"
#include "Misc/ConfigCacheIni.h"

namespace
{
	const FLinearColor PanelBg(0.02f, 0.025f, 0.03f, 0.94f);
	const FLinearColor Ember(0.82f, 0.42f, 0.16f, 1.f);
	const FLinearColor Steel(0.72f, 0.78f, 0.82f, 1.f);
	const FLinearColor Muted(0.55f, 0.58f, 0.6f, 1.f);
	const FLinearColor IdleBtn(0.08f, 0.09f, 0.1f, 1.f);

	TSharedRef<STextBlock> Label(const FString& Text, int32 Size = 16, FLinearColor Color = Steel)
	{
		return SNew(STextBlock)
			.Text(FText::FromString(Text))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", Size))
			.ColorAndOpacity(Color);
	}

	TSharedRef<SWidget> ControlRow(const FString& Action, const FString& Keyboard, const FString& Gamepad)
	{
		return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.2f).Padding(0, 6)
		[
			SNew(STextBlock).Text(FText::FromString(Action)).Font(FCoreStyle::GetDefaultFontStyle("Regular", 14)).ColorAndOpacity(Steel)
		]
		+ SHorizontalBox::Slot().FillWidth(1.f).Padding(8, 6)
		[
			SNew(STextBlock).Text(FText::FromString(Keyboard)).Font(FCoreStyle::GetDefaultFontStyle("Bold", 14)).ColorAndOpacity(Ember)
		]
		+ SHorizontalBox::Slot().FillWidth(1.f).Padding(8, 6)
		[
			SNew(STextBlock).Text(FText::FromString(Gamepad)).Font(FCoreStyle::GetDefaultFontStyle("Regular", 14)).ColorAndOpacity(Muted)
		];
	}
}

void SThreeDensityHUDWidget::Construct(const FArguments& InArgs)
{
	Owner = InArgs._Owner;

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot().VAlign(VAlign_Bottom).HAlign(HAlign_Center).Padding(0, 0, 0, 48)
		[
			SNew(SBox)
			.Visibility(this, &SThreeDensityHUDWidget::GetTipVisibility)
			.WidthOverride(720)
			[
				SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(0.04f, 0.05f, 0.06f, 0.88f))
				.Padding(FMargin(22, 16))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(this, &SThreeDensityHUDWidget::GetTipText)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
						.ColorAndOpacity(Steel)
						.AutoWrapText(true)
						.Justification(ETextJustify::Center)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
					[
						SNew(SButton)
						.ButtonColorAndOpacity(FLinearColor::Transparent)
						.OnClicked(this, &SThreeDensityHUDWidget::OnDismissTip)
						.HAlign(HAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("click to dismiss  ·  ESC opens the full cookbook")))
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
							.ColorAndOpacity(Muted)
						]
					]
				]
			]
		]
		+ SOverlay::Slot().VAlign(VAlign_Top).HAlign(HAlign_Left).Padding(24)
		[
			SNew(SBox)
			.WidthOverride(48)
			.HeightOverride(48)
			.Visibility(this, &SThreeDensityHUDWidget::GetHintVisibility)
			[
				SNew(SImage)
				.Image_Lambda([]() -> const FSlateBrush*
				{
					TSharedPtr<FSlateBrush> Brush = ThreeDensityLogo::GetMarkBrush();
					return Brush.IsValid() ? Brush.Get() : FCoreStyle::Get().GetDefaultBrush();
				})
				.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.85f))
			]
		]
		+ SOverlay::Slot().VAlign(VAlign_Top).HAlign(HAlign_Right).Padding(24)
		[
			SNew(STextBlock)
			.Visibility(this, &SThreeDensityHUDWidget::GetHintVisibility)
			.Text(FText::FromString(TEXT("ESC  MENU")))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
			.ColorAndOpacity(FLinearColor(0.85f, 0.85f, 0.85f, 0.55f))
		]
		+ SOverlay::Slot().VAlign(VAlign_Bottom).HAlign(HAlign_Left).Padding(28, 0, 0, 28)
		[
			SNew(SBox)
			.Visibility(this, &SThreeDensityHUDWidget::GetHealthBarVisibility)
			[
				BuildHealthBar()
			]
		]
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.Visibility(this, &SThreeDensityHUDWidget::GetPauseVisibility)
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.62f))
			.Padding(0)
			[
				BuildPauseMenu()
			]
		]
	];
}

TSharedRef<SWidget> SThreeDensityHUDWidget::BuildHealthBar()
{
	const float TrackWidth = 232.0f;

	return SNew(SBox)
		.WidthOverride(260)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.02f, 0.025f, 0.03f, 0.82f))
			.Padding(FMargin(14, 10))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 6)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("HP")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
						.ColorAndOpacity(Ember)
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Right).VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(this, &SThreeDensityHUDWidget::GetHealthLabel)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 13))
						.ColorAndOpacity(Steel)
					]
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SBox)
					.HeightOverride(14)
					.WidthOverride(TrackWidth)
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SNew(SImage)
							.Image(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.ColorAndOpacity(FLinearColor(0.08f, 0.09f, 0.1f, 1.f))
						]
						+ SOverlay::Slot().HAlign(HAlign_Left)
						[
							SNew(SBox)
							.WidthOverride_Lambda([this, TrackWidth]() -> FOptionalSize
							{
								const float Percent = GetHealthPercent().Get(0.0f);
								return TrackWidth * FMath::Clamp(Percent, 0.0f, 1.0f);
							})
							[
								SNew(SImage)
								.Image(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.ColorAndOpacity(Ember)
							]
						]
					]
				]
			]
		];
}

void SThreeDensityHUDWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

TSharedRef<SWidget> SThreeDensityHUDWidget::BuildPauseMenu()
{
	return SNew(SBox)
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	[
		SNew(SBorder)
		.BorderBackgroundColor(PanelBg)
		.Padding(FMargin(36, 28))
		[
			SNew(SBox).WidthOverride(860).HeightOverride(560)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 28, 0)
				[
					SNew(SBox).WidthOverride(210)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
						[
							SNew(SBox)
							.WidthOverride(200)
							.HeightOverride(64)
							.HAlign(HAlign_Left)
							[
								SNew(SImage)
								.Image_Lambda([]() -> const FSlateBrush*
								{
									TSharedPtr<FSlateBrush> Brush = ThreeDensityLogo::GetLockupBrush();
									return Brush.IsValid() ? Brush.Get() : FCoreStyle::Get().GetDefaultBrush();
								})
							]
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 22)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("PAUSED")))
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
							.ColorAndOpacity(Muted)
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)
						[
							SNew(SButton).ButtonColorAndOpacity(IdleBtn).OnClicked(this, &SThreeDensityHUDWidget::OnResume)
							[
								Label(TEXT("RESUME"), 14)
							]
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)
						[
							SNew(SButton).ButtonColorAndOpacity(this, &SThreeDensityHUDWidget::NavColor, 0)
							.OnClicked(this, &SThreeDensityHUDWidget::OnTabControls)
							[
								Label(TEXT("CONTROLS"), 14)
							]
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)
						[
							SNew(SButton).ButtonColorAndOpacity(this, &SThreeDensityHUDWidget::NavColor, 1)
							.OnClicked(this, &SThreeDensityHUDWidget::OnTabSettings)
							[
								Label(TEXT("SETTINGS"), 14)
							]
						]
						+ SVerticalBox::Slot().FillHeight(1.f)
						[
							SNullWidget::NullWidget
						]
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(SButton).ButtonColorAndOpacity(IdleBtn).OnClicked(this, &SThreeDensityHUDWidget::OnQuit)
							[
								Label(TEXT("QUIT"), 14, FLinearColor(0.75f, 0.3f, 0.22f))
							]
						]
					]
				]
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SBox).Visibility(this, &SThreeDensityHUDWidget::GetControlsVisibility)
						[
							BuildControlsPanel()
						]
					]
					+ SOverlay::Slot()
					[
						SNew(SBox).Visibility(this, &SThreeDensityHUDWidget::GetSettingsVisibility)
						[
							BuildSettingsPanel()
						]
					]
				]
			]
		]
	];
}

TSharedRef<SWidget> SThreeDensityHUDWidget::BuildControlsPanel()
{
	return SNew(SVerticalBox)
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 16)
	[
		Label(TEXT("CONTROL COOKBOOK"), 16, Ember)
	]
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 10)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.2f) [ Label(TEXT("Action"), 12, Muted) ]
		+ SHorizontalBox::Slot().FillWidth(1.f) [ Label(TEXT("Keyboard"), 12, Muted) ]
		+ SHorizontalBox::Slot().FillWidth(1.f) [ Label(TEXT("Gamepad"), 12, Muted) ]
	]
	+ SVerticalBox::Slot().FillHeight(1.f)
	[
		SNew(SScrollBox)
		+ SScrollBox::Slot() [ ControlRow(TEXT("Move"), TEXT("W A S D"), TEXT("Left stick")) ]
		+ SScrollBox::Slot() [ ControlRow(TEXT("Look"), TEXT("Mouse"), TEXT("Right stick")) ]
		+ SScrollBox::Slot() [ ControlRow(TEXT("Jump"), TEXT("Space"), TEXT("A / Cross")) ]
		+ SScrollBox::Slot() [ ControlRow(TEXT("Combo attack"), TEXT("Left mouse"), TEXT("RT / R2")) ]
		+ SScrollBox::Slot() [ ControlRow(TEXT("Charged strike"), TEXT("Hold right mouse"), TEXT("Hold RB / R1")) ]
		+ SScrollBox::Slot() [ ControlRow(TEXT("Camera shoulder"), TEXT("Q"), TEXT("L1 / LB")) ]
		+ SScrollBox::Slot() [ ControlRow(TEXT("Zoom in / out"), TEXT("Mouse wheel  ·  - / ="), TEXT("—")) ]
		+ SScrollBox::Slot() [ ControlRow(TEXT("Pause / this menu"), TEXT("Esc"), TEXT("Start")) ]
		+ SScrollBox::Slot() [ ControlRow(TEXT("Toggle fullscreen"), TEXT("Alt + Enter"), TEXT("—")) ]
	]
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 16, 0, 0)
	[
		SNew(STextBlock)
		.Text(FText::FromString(TEXT("Chain left-clicks during a combo window to string hits. Hold right-click to charge, then release for a heavy blow. Checkpoints restore you if you fall.")))
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 13))
		.ColorAndOpacity(Muted)
		.AutoWrapText(true)
	];
}

TSharedRef<SWidget> SThreeDensityHUDWidget::BuildSettingsPanel()
{
	return SNew(SVerticalBox)
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
	[
		Label(TEXT("GRAPHICS"), 16, Ember)
	]
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
	[
		SNew(STextBlock)
		.Text(this, &SThreeDensityHUDWidget::GetHardwareText)
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
		.ColorAndOpacity(Muted)
		.AutoWrapText(true)
	]
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
	[
		SNew(STextBlock)
		.Text(this, &SThreeDensityHUDWidget::GetBenchmarkText)
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
		.ColorAndOpacity(Muted)
		.AutoWrapText(true)
	]
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
	[
		SNew(STextBlock)
		.Text(this, &SThreeDensityHUDWidget::GetPresetLabel)
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
		.ColorAndOpacity(Steel)
	]
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 16)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 6, 0)
		[
			SNew(SButton).OnClicked(this, &SThreeDensityHUDWidget::OnAutoDetect).ButtonColorAndOpacity(IdleBtn)
			[ Label(TEXT("AUTO"), 12, Ember) ]
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 6, 0)
		[
			SNew(SButton).OnClicked(this, &SThreeDensityHUDWidget::OnBenchmark).ButtonColorAndOpacity(IdleBtn)
			[ Label(TEXT("BENCHMARK"), 12, Ember) ]
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 6, 0)
		[
			SNew(SButton).OnClicked(this, &SThreeDensityHUDWidget::OnPreset, 0).ButtonColorAndOpacity(this, &SThreeDensityHUDWidget::PresetColor, 0)
			[ Label(TEXT("LOW"), 12) ]
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 6, 0)
		[
			SNew(SButton).OnClicked(this, &SThreeDensityHUDWidget::OnPreset, 1).ButtonColorAndOpacity(this, &SThreeDensityHUDWidget::PresetColor, 1)
			[ Label(TEXT("MEDIUM"), 12) ]
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 6, 0)
		[
			SNew(SButton).OnClicked(this, &SThreeDensityHUDWidget::OnPreset, 2).ButtonColorAndOpacity(this, &SThreeDensityHUDWidget::PresetColor, 2)
			[ Label(TEXT("HIGH"), 12) ]
		]
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SNew(SButton).OnClicked(this, &SThreeDensityHUDWidget::OnPreset, 3).ButtonColorAndOpacity(this, &SThreeDensityHUDWidget::PresetColor, 3)
			[ Label(TEXT("EPIC"), 12) ]
		]
	]
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
	[
		SNew(STextBlock)
		.Text(this, &SThreeDensityHUDWidget::GetResolutionLabel)
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 13))
		.ColorAndOpacity(Steel)
	]
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 18)
	[
		SNew(SSlider)
		.Value_Lambda([]()
		{
			if (UThreeDensityGameUserSettings* S = UThreeDensityGameUserSettings::GetThreeDensitySettings())
			{
				return S->GetResolutionScaleNormalized();
			}
			return 1.f;
		})
		.OnValueChanged_Lambda([](float V)
		{
			if (UThreeDensityGameUserSettings* S = UThreeDensityGameUserSettings::GetThreeDensitySettings())
			{
				S->SetResolutionScaleNormalized(FMath::Clamp(V, 0.5f, 1.f));
				S->ApplySettings(false);
				S->SaveSettings();
			}
		})
	]
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
		[
			SNew(SButton).OnClicked(this, &SThreeDensityHUDWidget::OnToggleLumen).ButtonColorAndOpacity(IdleBtn)
			[ SNew(STextBlock).Text(this, &SThreeDensityHUDWidget::GetLumenLabel).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)).ColorAndOpacity(Steel) ]
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
		[
			SNew(SButton).OnClicked(this, &SThreeDensityHUDWidget::OnToggleVSync).ButtonColorAndOpacity(IdleBtn)
			[ SNew(STextBlock).Text(this, &SThreeDensityHUDWidget::GetVSyncLabel).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)).ColorAndOpacity(Steel) ]
		]
	]
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 12, 0, 6)
	[
		SNew(STextBlock).Text(this, &SThreeDensityHUDWidget::GetFpsLabel).Font(FCoreStyle::GetDefaultFontStyle("Regular", 13)).ColorAndOpacity(Steel)
	]
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 16)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 6, 0)
		[ SNew(SButton).OnClicked(this, &SThreeDensityHUDWidget::OnFpsCap, 30).ButtonColorAndOpacity(IdleBtn)[Label(TEXT("30"), 12)] ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 6, 0)
		[ SNew(SButton).OnClicked(this, &SThreeDensityHUDWidget::OnFpsCap, 60).ButtonColorAndOpacity(IdleBtn)[Label(TEXT("60"), 12)] ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 6, 0)
		[ SNew(SButton).OnClicked(this, &SThreeDensityHUDWidget::OnFpsCap, 120).ButtonColorAndOpacity(IdleBtn)[Label(TEXT("120"), 12)] ]
		+ SHorizontalBox::Slot().AutoWidth()
		[ SNew(SButton).OnClicked(this, &SThreeDensityHUDWidget::OnFpsCap, 0).ButtonColorAndOpacity(IdleBtn)[Label(TEXT("UNLIMITED"), 12)] ]
	]
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
	[
		Label(TEXT("DISPLAY"), 13, Muted)
	]
	+ SVerticalBox::Slot().AutoHeight()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 6, 0)
		[ SNew(SButton).OnClicked(this, &SThreeDensityHUDWidget::OnDisplayMode, 0).ButtonColorAndOpacity(IdleBtn)[Label(TEXT("FULLSCREEN"), 12)] ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 6, 0)
		[ SNew(SButton).OnClicked(this, &SThreeDensityHUDWidget::OnDisplayMode, 1).ButtonColorAndOpacity(IdleBtn)[Label(TEXT("BORDERLESS"), 12)] ]
		+ SHorizontalBox::Slot().AutoWidth()
		[ SNew(SButton).OnClicked(this, &SThreeDensityHUDWidget::OnDisplayMode, 2).ButtonColorAndOpacity(IdleBtn)[Label(TEXT("WINDOWED"), 12)] ]
	]
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 20, 0, 0)
	[
		Label(TEXT("BUILD"), 13, Muted)
	]
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 0)
	[
		SNew(STextBlock)
		.Text(this, &SThreeDensityHUDWidget::GetBuildVersionText)
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 13))
		.ColorAndOpacity(Ember)
	];
}

FReply SThreeDensityHUDWidget::OnResume()
{
	if (ACombatPlayerController* PC = Owner.Get())
	{
		PC->TogglePauseMenu();
	}
	return FReply::Handled();
}

FReply SThreeDensityHUDWidget::OnQuit()
{
	if (ACombatPlayerController* PC = Owner.Get())
	{
		PC->ConsoleCommand(TEXT("quit"));
	}
	return FReply::Handled();
}

FReply SThreeDensityHUDWidget::OnTabControls()
{
	if (ACombatPlayerController* PC = Owner.Get())
	{
		PC->SetMenuTab(0);
	}
	return FReply::Handled();
}

FReply SThreeDensityHUDWidget::OnTabSettings()
{
	if (ACombatPlayerController* PC = Owner.Get())
	{
		PC->SetMenuTab(1);
	}
	return FReply::Handled();
}

FReply SThreeDensityHUDWidget::OnPreset(int32 PresetIndex)
{
	if (UThreeDensityGameUserSettings* S = UThreeDensityGameUserSettings::GetThreeDensitySettings())
	{
		S->ApplyGraphicsPreset(static_cast<EThreeDensityGraphicsPreset>(PresetIndex), false);
	}
	return FReply::Handled();
}

FReply SThreeDensityHUDWidget::OnAutoDetect()
{
	if (UThreeDensityGameUserSettings* S = UThreeDensityGameUserSettings::GetThreeDensitySettings())
	{
		S->ApplyGraphicsPreset(S->RecommendPreset(), true);
	}
	return FReply::Handled();
}

FReply SThreeDensityHUDWidget::OnBenchmark()
{
	if (GEngine)
	{
		if (UWorld* World = GEngine->GetCurrentPlayWorld())
		{
			if (UThreeDensityGameInstance* GI = Cast<UThreeDensityGameInstance>(World->GetGameInstance()))
			{
				GI->StartPerformanceBenchmark(true);
			}
		}
	}
	return FReply::Handled();
}

FReply SThreeDensityHUDWidget::OnToggleLumen()
{
	if (UThreeDensityGameUserSettings* S = UThreeDensityGameUserSettings::GetThreeDensitySettings())
	{
		S->SetLumenEnabled(!S->IsLumenEnabled());
	}
	return FReply::Handled();
}

FReply SThreeDensityHUDWidget::OnToggleVSync()
{
	if (UThreeDensityGameUserSettings* S = UThreeDensityGameUserSettings::GetThreeDensitySettings())
	{
		S->SetVSyncEnabled(!S->IsVSyncEnabled());
		S->ApplySettings(false);
		S->SaveSettings();
	}
	return FReply::Handled();
}

FReply SThreeDensityHUDWidget::OnFpsCap(int32 Cap)
{
	if (UThreeDensityGameUserSettings* S = UThreeDensityGameUserSettings::GetThreeDensitySettings())
	{
		S->SetFrameCap(Cap);
	}
	return FReply::Handled();
}

FReply SThreeDensityHUDWidget::OnDisplayMode(int32 Mode)
{
	if (UThreeDensityGameUserSettings* S = UThreeDensityGameUserSettings::GetThreeDensitySettings())
	{
		if (Mode == 0)
		{
			S->SetFullscreenMode(EWindowMode::Fullscreen);
		}
		else if (Mode == 1)
		{
			S->SetFullscreenMode(EWindowMode::WindowedFullscreen);
		}
		else
		{
			S->SetFullscreenMode(EWindowMode::Windowed);
		}
		S->ApplySettings(false);
		S->SaveSettings();
	}
	return FReply::Handled();
}

FReply SThreeDensityHUDWidget::OnDismissTip()
{
	if (ACombatPlayerController* PC = Owner.Get())
	{
		PC->AdvanceOrDismissTip();
	}
	return FReply::Handled();
}

EVisibility SThreeDensityHUDWidget::GetPauseVisibility() const
{
	const ACombatPlayerController* PC = Owner.Get();
	return (PC && PC->IsPauseMenuOpen()) ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SThreeDensityHUDWidget::GetTipVisibility() const
{
	const ACombatPlayerController* PC = Owner.Get();
	if (!PC || PC->IsPauseMenuOpen() || !PC->ShouldShowTip())
	{
		return EVisibility::Collapsed;
	}
	return EVisibility::Visible;
}

EVisibility SThreeDensityHUDWidget::GetHintVisibility() const
{
	const ACombatPlayerController* PC = Owner.Get();
	if (!PC || PC->IsPauseMenuOpen() || PC->HasOpenedMenuOnce())
	{
		return EVisibility::Collapsed;
	}
	return EVisibility::HitTestInvisible;
}

EVisibility SThreeDensityHUDWidget::GetHealthBarVisibility() const
{
	const ACombatPlayerController* PC = Owner.Get();
	if (!PC || PC->IsPauseMenuOpen())
	{
		return EVisibility::Collapsed;
	}

	const ACombatCharacter* Character = Cast<ACombatCharacter>(PC->GetPawn());
	return Character ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
}

EVisibility SThreeDensityHUDWidget::GetControlsVisibility() const
{
	const ACombatPlayerController* PC = Owner.Get();
	return (PC && PC->GetMenuTab() == 0) ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SThreeDensityHUDWidget::GetSettingsVisibility() const
{
	const ACombatPlayerController* PC = Owner.Get();
	return (PC && PC->GetMenuTab() == 1) ? EVisibility::Visible : EVisibility::Collapsed;
}

FText SThreeDensityHUDWidget::GetTipText() const
{
	const ACombatPlayerController* PC = Owner.Get();
	return PC ? PC->GetCurrentTipText() : FText::GetEmpty();
}

FText SThreeDensityHUDWidget::GetHealthLabel() const
{
	const ACombatPlayerController* PC = Owner.Get();
	const ACombatCharacter* Character = PC ? Cast<ACombatCharacter>(PC->GetPawn()) : nullptr;
	if (!Character)
	{
		return FText::GetEmpty();
	}

	const int32 Current = FMath::Max(0, FMath::CeilToInt(Character->GetCurrentHP()));
	const int32 Max = FMath::Max(1, FMath::CeilToInt(Character->GetMaxHP()));
	return FText::FromString(FString::Printf(TEXT("%d / %d"), Current, Max));
}

TOptional<float> SThreeDensityHUDWidget::GetHealthPercent() const
{
	const ACombatPlayerController* PC = Owner.Get();
	const ACombatCharacter* Character = PC ? Cast<ACombatCharacter>(PC->GetPawn()) : nullptr;
	return Character ? Character->GetHPPercent() : 0.0f;
}

FText SThreeDensityHUDWidget::GetHardwareText() const
{
	if (UThreeDensityGameUserSettings* S = UThreeDensityGameUserSettings::GetThreeDensitySettings())
	{
		return FText::FromString(S->GetDetectedHardwareSummary());
	}
	return FText::GetEmpty();
}

FText SThreeDensityHUDWidget::GetBenchmarkText() const
{
	if (UThreeDensityGameUserSettings* S = UThreeDensityGameUserSettings::GetThreeDensitySettings())
	{
		return FText::FromString(S->GetBenchmarkSummary());
	}
	return FText::GetEmpty();
}

FText SThreeDensityHUDWidget::GetPresetLabel() const
{
	if (UThreeDensityGameUserSettings* S = UThreeDensityGameUserSettings::GetThreeDensitySettings())
	{
		const TCHAR* Names[] = { TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic") };
		const int32 Idx = FMath::Clamp(static_cast<int32>(S->GetGraphicsPreset()), 0, 3);
		return FText::FromString(FString::Printf(TEXT("Quality: %s%s"), Names[Idx],
			S->WasAutoDetected() ? TEXT("  (matched to this PC)") : TEXT("")));
	}
	return FText::FromString(TEXT("Quality"));
}

FText SThreeDensityHUDWidget::GetLumenLabel() const
{
	if (UThreeDensityGameUserSettings* S = UThreeDensityGameUserSettings::GetThreeDensitySettings())
	{
		return FText::FromString(S->IsLumenEnabled() ? TEXT("Lumen  ON") : TEXT("Lumen  OFF"));
	}
	return FText::FromString(TEXT("Lumen"));
}

FText SThreeDensityHUDWidget::GetVSyncLabel() const
{
	if (UThreeDensityGameUserSettings* S = UThreeDensityGameUserSettings::GetThreeDensitySettings())
	{
		return FText::FromString(S->IsVSyncEnabled() ? TEXT("VSync  ON") : TEXT("VSync  OFF"));
	}
	return FText::FromString(TEXT("VSync"));
}

FText SThreeDensityHUDWidget::GetFpsLabel() const
{
	if (UThreeDensityGameUserSettings* S = UThreeDensityGameUserSettings::GetThreeDensitySettings())
	{
		const int32 Cap = S->GetFrameCap();
		return FText::FromString(Cap <= 0
			? TEXT("Frame cap: Unlimited")
			: FString::Printf(TEXT("Frame cap: %d"), Cap));
	}
	return FText::FromString(TEXT("Frame cap"));
}

FText SThreeDensityHUDWidget::GetResolutionLabel() const
{
	if (UThreeDensityGameUserSettings* S = UThreeDensityGameUserSettings::GetThreeDensitySettings())
	{
		return FText::FromString(FString::Printf(TEXT("3D resolution scale  %d%%"),
			FMath::RoundToInt(S->GetResolutionScaleNormalized() * 100.f)));
	}
	return FText::FromString(TEXT("3D resolution scale"));
}

FText SThreeDensityHUDWidget::GetBuildVersionText() const
{
	FString Tag;

	const FString VersionPath = FPaths::Combine(
		FPlatformMisc::GetEnvironmentVariable(TEXT("LOCALAPPDATA")),
		TEXT("ThreeDensity"),
		TEXT("version.txt"));
	if (FPaths::FileExists(VersionPath))
	{
		FFileHelper::LoadFileToString(Tag, *VersionPath);
		Tag.TrimStartAndEndInline();
	}

	if (Tag.IsEmpty() && GConfig)
	{
		GConfig->GetString(TEXT("/Script/EngineSettings.GeneralProjectSettings"), TEXT("ProjectVersion"), Tag, GGameIni);
		Tag.TrimStartAndEndInline();
	}

	if (Tag.IsEmpty())
	{
		Tag = TEXT("dev");
	}

	return FText::FromString(FString::Printf(TEXT("Build %s"), *Tag));
}

FSlateColor SThreeDensityHUDWidget::NavColor(int32 Tab) const
{
	const ACombatPlayerController* PC = Owner.Get();
	if (PC && PC->GetMenuTab() == Tab)
	{
		return FSlateColor(FLinearColor(0.18f, 0.09f, 0.04f, 1.f));
	}
	return FSlateColor(IdleBtn);
}

FSlateColor SThreeDensityHUDWidget::PresetColor(int32 PresetIndex) const
{
	if (UThreeDensityGameUserSettings* S = UThreeDensityGameUserSettings::GetThreeDensitySettings())
	{
		if (static_cast<int32>(S->GetGraphicsPreset()) == PresetIndex && !S->WasAutoDetected())
		{
			return FSlateColor(FLinearColor(0.18f, 0.09f, 0.04f, 1.f));
		}
		if (static_cast<int32>(S->GetGraphicsPreset()) == PresetIndex && S->WasAutoDetected())
		{
			return FSlateColor(FLinearColor(0.18f, 0.09f, 0.04f, 1.f));
		}
	}
	return FSlateColor(IdleBtn);
}
