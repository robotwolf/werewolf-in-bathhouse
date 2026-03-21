#include "Modules/ModuleManager.h"

#include "Containers/Ticker.h"
#include "Engine/Engine.h"
#include "IPythonScriptPlugin.h"
#include "Misc/CoreDelegates.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY_STATIC(LogWerewolfNBHEditorModule, Log, All);

namespace
{
    constexpr int32 MaxPythonInitRetries = 20;
    constexpr float InitialAutoSyncDelaySeconds = 2.0f;
    constexpr float RetryDelaySeconds = 1.0f;

    FString BuildPythonRunCommand(const FString& ScriptPath)
    {
        const FString NormalizedPath = ScriptPath.Replace(TEXT("\\"), TEXT("/")).Replace(TEXT("'"), TEXT("\\'"));
        return FString::Printf(
            TEXT("import runpy; runpy.run_path(r'%s', run_name='__main__')"),
            *NormalizedPath);
    }
}

class FWerewolfNBHEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        if (IsRunningCommandlet())
        {
            return;
        }

        PostEngineInitHandle = FCoreDelegates::OnPostEngineInit.AddRaw(this, &FWerewolfNBHEditorModule::HandlePostEngineInit);
        if (GEngine != nullptr)
        {
            ScheduleAutoSync(InitialAutoSyncDelaySeconds);
        }
    }

    virtual void ShutdownModule() override
    {
        if (PostEngineInitHandle.IsValid())
        {
            FCoreDelegates::OnPostEngineInit.Remove(PostEngineInitHandle);
            PostEngineInitHandle.Reset();
        }

        if (AutoSyncTickerHandle.IsValid())
        {
            FTSTicker::GetCoreTicker().RemoveTicker(AutoSyncTickerHandle);
            AutoSyncTickerHandle.Reset();
        }
    }

private:
    void HandlePostEngineInit()
    {
        if (PostEngineInitHandle.IsValid())
        {
            FCoreDelegates::OnPostEngineInit.Remove(PostEngineInitHandle);
            PostEngineInitHandle.Reset();
        }

        ScheduleAutoSync(InitialAutoSyncDelaySeconds);
    }

    void ScheduleAutoSync(float DelaySeconds)
    {
        if (bAutoSyncCompleted || bAutoSyncScheduled || IsRunningCommandlet())
        {
            return;
        }

        bAutoSyncScheduled = true;
        AutoSyncTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
            FTickerDelegate::CreateRaw(this, &FWerewolfNBHEditorModule::HandleDeferredAutoSync),
            FMath::Max(0.1f, DelaySeconds));
    }

    bool HandleDeferredAutoSync(float)
    {
        bAutoSyncScheduled = false;
        AutoSyncTickerHandle.Reset();

        if (bAutoSyncCompleted || IsRunningCommandlet())
        {
            return false;
        }

        IPythonScriptPlugin* PythonScriptPlugin = FModuleManager::LoadModulePtr<IPythonScriptPlugin>(TEXT("PythonScriptPlugin"));
        if (!PythonScriptPlugin)
        {
            UE_LOG(LogWerewolfNBHEditorModule, Warning, TEXT("Automatic startup sync skipped because PythonScriptPlugin is unavailable."));
            bAutoSyncCompleted = true;
            return false;
        }

        PythonScriptPlugin->ForceEnablePythonAtRuntime();
        if (!PythonScriptPlugin->IsPythonInitialized())
        {
            ++PythonInitRetryCount;
            if (PythonInitRetryCount < MaxPythonInitRetries)
            {
                ScheduleAutoSync(RetryDelaySeconds);
            }
            else
            {
                UE_LOG(LogWerewolfNBHEditorModule, Warning, TEXT("Automatic startup sync gave up waiting for Python initialization."));
                bAutoSyncCompleted = true;
            }

            return false;
        }

        PythonInitRetryCount = 0;
        RunStartupSyncScripts(*PythonScriptPlugin);
        bAutoSyncCompleted = true;
        return false;
    }

    void RunStartupSyncScripts(IPythonScriptPlugin& PythonScriptPlugin)
    {
        const FString ScriptsDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT("Scripts")));
        const TArray<FString> ScriptNames =
        {
            TEXT("sync_stagehand_conversation_topics.py"),
            TEXT("sync_room_gameplay_markers.py"),
            TEXT("sync_stagehand_npc_profiles.py"),
        };

        for (const FString& ScriptName : ScriptNames)
        {
            const FString ScriptPath = FPaths::Combine(ScriptsDir, ScriptName);
            if (!FPaths::FileExists(ScriptPath))
            {
                UE_LOG(
                    LogWerewolfNBHEditorModule,
                    Warning,
                    TEXT("Automatic startup sync skipped missing script: %s"),
                    *ScriptPath);
                continue;
            }

            const bool bSucceeded = PythonScriptPlugin.ExecPythonCommand(*BuildPythonRunCommand(ScriptPath));
            if (bSucceeded)
            {
                UE_LOG(
                    LogWerewolfNBHEditorModule,
                    Log,
                    TEXT("Automatic startup sync ran: %s"),
                    *ScriptName);
            }
            else
            {
                UE_LOG(
                    LogWerewolfNBHEditorModule,
                    Warning,
                    TEXT("Automatic startup sync failed: %s"),
                    *ScriptName);
            }
        }
    }

    FDelegateHandle PostEngineInitHandle;
    FTSTicker::FDelegateHandle AutoSyncTickerHandle;
    bool bAutoSyncScheduled = false;
    bool bAutoSyncCompleted = false;
    int32 PythonInitRetryCount = 0;
};

IMPLEMENT_MODULE(FWerewolfNBHEditorModule, WerewolfNBHEditor)
