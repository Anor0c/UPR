# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

UPR_Final is an Unreal Engine **5.8** game (Windows only), a multiplayer runner/racer. The repo root holds one folder, `UPR_Final/`, which contains the `.uproject`. Run every engine command from there. There is no README, test suite, or linter.

## Build and run

Use the engine's UnrealBuildTool (UBT) or open `UPR_Final/UPR_Final.sln` in Rider or Visual Studio. Targets are `UPR_Final` (game) and `UPR_FinalEditor` (editor).

```
# Editor build (adjust the engine path; UE_5.8 is the assumed install location)
"C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" UPR_FinalEditor Win64 Development -Project="D:\GitHub\UPR\UPR_Final\UPR_Final.uproject"
```

- Launch the editor by opening `UPR_Final.uproject`. Right-click it and pick "Generate Visual Studio project files" to regenerate the `.sln`.
- Close the editor before building from the command line. Otherwise use Live Coding (Ctrl+Alt+F11).
- Gameplay logic lives mostly in Blueprints under `Content/Core/`, so check there before assuming behaviour is in C++. Startup map is `/Game/Core/Maps/L_HyperSpace`. The default game map is the menu `/Game/Core/Maps/MenuScene/L_MapUI`, and the default game mode is `GM_RunMultipayer`. Both are set in `Config/DefaultEngine.ini`.

## Architecture

A single C++ module, `UPR_Final` (`Source/UPR_Final/`), is the native base layer. Blueprints subclass it. Keep the C++ API `UPROPERTY`/`UFUNCTION`-exposed so Blueprints can use it.

- `ABaseRunner` extends `ACharacter` and implements `IAbilitySystemInterface`. It is the playable character base. It owns a `UAbilitySystemComponent` (`GASComponent`) and uses `URunnerAttributeSet`. Movement abilities and attributes go through the Gameplay Ability System. The plugin and module dependencies (`GameplayAbilities`, `GameplayTags`, `GameplayTasks`) are set in `UPR_Final.Build.cs`.
- `URunnerAttributeSet` holds the GAS attributes for runners.
- `UAutoTargetSpringArm` extends `USpringArmComponent` for camera auto-targeting.
- `ASplineMeshBase` is a spline-mesh actor, newly added in the `TweakSplineMesh` branch.
- `UVoicelineManager` is an actor component that plays voice lines. It is driven by gameplay tags.
- `URunrSaveGame` extends `USaveGame` for persistence.
- `UBabylon` is a `UBlueprintFunctionLibrary` of shared Blueprint helpers (it forward-declares `UNavigationPath`).

### Gameplay tags

`Config/DefaultGameplayTags.ini` is the source of truth for tags. It uses `ImportTagsFromConfig=True`, and it has many `GameplayTagRedirects` for renamed tags. The hierarchy uses the `UPR.*` prefix (`UPR.Attribute`, `UPR.RunnerEvent`, `UPR.KeyMoments`, `UPR.CharacterListening.<Adler|Shred|DDos>`). When renaming a tag, add a redirect instead of editing the old entry, so existing assets keep resolving.

### Plugins

`.uproject` enables `GameplayAbilities`, `CommonUI`, `AudioModulation`, `MovieRenderPipeline`, `AsyncLoadingScreen` (a Marketplace plugin), `GPULightmass`, `CascadeToNiagaraConverter`, and `ModelingToolsEditorMode` (editor only).

## Repository notes

- `Content/` is mostly binary `.uasset`/`.umap` files, which cannot be diffed or merged. Avoid concurrent edits to the same asset, and don't try to edit them as text.
- `Binaries/`, `Intermediate/`, `Saved/`, and `DerivedDataCache/` are build output and cache. Don't edit them.
- `.idea/` and `UPR_Final.sln.DotSettings.user` are per-user Rider files. They are no longer tracked, so don't commit them.
