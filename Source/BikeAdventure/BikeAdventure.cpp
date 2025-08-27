#include "BikeAdventure.h"
#include "Modules/ModuleManager.h"

#if WITH_EDITOR
#include "ToolMenus.h"
#endif // WITH_EDITOR

/**
 * Game module used for BikeAdventure. A custom implementation is provided so
 * we can hook into editor initialisation and register any required ToolMenu
 * entries. In recent versions of Unreal Engine the \c ToggleFullscreen menu
 * item was removed, yet other commands still try to insert relative to it.
 * Registering a placeholder prevents repeated runtime warnings about a
 * missing menu entry when the editor launches.
 */
class FBikeAdventureModule : public FDefaultGameModuleImpl
{
public:
       virtual void StartupModule() override;
       virtual void ShutdownModule() override;

#if WITH_EDITOR
private:
       /** Callback used to register editor menus. */
       void RegisterMenus();

       /** Handle for unregistering the startup callback. */
       FDelegateHandle ToolMenusStartupHandle;
#endif // WITH_EDITOR
};

IMPLEMENT_PRIMARY_GAME_MODULE(FBikeAdventureModule, BikeAdventure, "BikeAdventure");

void FBikeAdventureModule::StartupModule()
{
#if WITH_EDITOR
       // Delay menu registration until the editor ToolMenus system is ready.
       ToolMenusStartupHandle = UToolMenus::RegisterStartupCallback(
               FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FBikeAdventureModule::RegisterMenus));
#endif // WITH_EDITOR
}

void FBikeAdventureModule::ShutdownModule()
{
#if WITH_EDITOR
       UToolMenus::UnRegisterStartupCallback(ToolMenusStartupHandle);
#endif // WITH_EDITOR
}

#if WITH_EDITOR
void FBikeAdventureModule::RegisterMenus()
{
       if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelViewportContextMenu"))
       {
               FToolMenuSection& Section = Menu->FindOrAddSection("LevelViewportOptions");

               // Only add the placeholder if it does not already exist.
               if (!Section.FindEntry("ToggleFullscreen"))
               {
                       FToolMenuEntry Entry = FToolMenuEntry::InitMenuEntry(
                               "ToggleFullscreen",
                               FText::FromString("Toggle Fullscreen"),
                               FText::FromString("Placeholder entry to maintain backwards compatibility."));

                       Section.AddEntry(Entry);
               }
       }
}
#endif // WITH_EDITOR

