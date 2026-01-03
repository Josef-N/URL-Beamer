// AppUtilities.mm

#import <Foundation/Foundation.h>
#include <JuceHeader.h>
#include "AppUtilities.h"

#if JUCE_IOS
 #import <UIKit/UIKit.h>
 @class UIInteraction;
#endif

// Define the global (declared as extern in AppUtilities.h)
bool selectDirectoryToggleState = false;

//==============================================================================
// Standalone alert persistence helper (iOS-safe, sandboxed)

juce::PropertiesFile& getStandaloneProperties()
{
    static std::unique_ptr<juce::PropertiesFile> props;

    if (props == nullptr)
    {
        juce::PropertiesFile::Options options;
        options.applicationName   = "URL Beamer";     // base name of the file
        options.folderName        = "URL Beamer";     // subfolder inside the app data location
        options.filenameSuffix    = "properties";     // -> "URL Beamer.properties"
        options.storageFormat     = juce::PropertiesFile::storeAsXML;
        options.commonToAllUsers  = false;
       #if JUCE_MAC
        options.osxLibrarySubFolder = "Application Support";
       #endif

        props = std::make_unique<juce::PropertiesFile> (options);
//      juce::Logger::writeToLog ("Properties file: " + props->getFile().getFullPathName());
    }
    return *props;
}
// ---- Alert flag -------------------------------------------------------------
bool shouldSuppressStandaloneAlert()
{
    return getStandaloneProperties().getBoolValue ("suppressStandaloneAlert", false);
}
void setSuppressStandaloneAlert (bool shouldSuppress)
{
    auto& pf = getStandaloneProperties();
    pf.setValue ("suppressStandaloneAlert", shouldSuppress);
    pf.saveIfNeeded();
}
// ---- selectDirectoryToggleState --------------------------------------------
bool getStandaloneSelectDirectoryToggleState()
{
    return getStandaloneProperties().getBoolValue ("selectDirectoryToggleState", false);
}
void setStandaloneSelectDirectoryToggleState (bool state)
{
    auto& pf = getStandaloneProperties();
    pf.setValue ("selectDirectoryToggleState", state);
    pf.saveIfNeeded();
}
// ---- lastUsedFile -----------------------------------------------------------
juce::String getStandaloneLastUsedFilePath()
{
    return getStandaloneProperties().getValue ("lastUsedFile", "");
}
void setStandaloneLastUsedFilePath (const juce::String& path)
{
    auto& pf = getStandaloneProperties();
    pf.setValue ("lastUsedFile", path);
    pf.saveIfNeeded();
}
// ---- open last file on startup ---------------------------------------------
bool getStandaloneOpenLastUsedFileOnStartup()
{
    return getStandaloneProperties().getBoolValue ("openLastUsedFileOnStartup", true);
}
void setStandaloneOpenLastUsedFileOnStartup (bool shouldOpen)
{
    auto& pf = getStandaloneProperties();
    pf.setValue ("openLastUsedFileOnStartup", shouldOpen);
    pf.saveIfNeeded();
}
// ---- Safe Mode Check on startup ---------------------------------------------
bool getStandaloneAutoloadInProgress()
{
    return getStandaloneProperties().getBoolValue ("autoloadInProgress", false);
}
void setStandaloneAutoloadInProgress (bool inProgress)
{
    auto& pf = getStandaloneProperties();
    pf.setValue ("autoloadInProgress", inProgress);
    pf.saveIfNeeded();
}

//==============================================================================
// File / app-group utilities

juce::File getAppGroupDirectory()
{
    #if JUCE_IOS
        if (selectDirectoryToggleState)
        {   // "Documents" Directory (Button On-state)
            return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory);
        }
        else
        {
            // Default: AppGroup Directory (Off-state)
            NSFileManager* fileManager = [NSFileManager defaultManager];
            NSURL* groupURL = [fileManager containerURLForSecurityApplicationGroupIdentifier:@"group.com.JosefNovotny.URLBeamer"];
            if (groupURL == nil)
            {
                juce::Logger::writeToLog ("Failed to locate App Group directory.");
                return juce::File();
            }
            return juce::File ([groupURL.path UTF8String]);
        }
    #elif JUCE_MAC
        return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("URL Beamer");
    //  return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getChildFile ("URL Beamer");
    #endif
}

//==============================================================================
//==============================================================================
#if JUCE_IOS

// Detecting iPad (vs. iPhone)
bool isRunningOnIPad()
{
    return UIDevice.currentDevice.userInterfaceIdiom == UIUserInterfaceIdiomPad;
}
//==============================================================================
// iOS 16+ Edit Menu delegate (Cut/Copy/Paste)
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 160000

static juce::TextEditor* gCurrentMenuEditor = nullptr;

@interface JuceEditMenuDelegate : NSObject <UIEditMenuInteractionDelegate>
@end

@implementation JuceEditMenuDelegate

- (UIMenu*)editMenuInteraction:(UIEditMenuInteraction*)interaction
          menuForConfiguration:(UIEditMenuConfiguration*)configuration
              suggestedActions:(NSArray<UIMenuElement*>*)suggestedActions API_AVAILABLE(ios(16.0))
{
    (void) interaction;
    (void) configuration;

    juce::TextEditor* editor = gCurrentMenuEditor;

    const bool hasSelection = (editor != nullptr)
                           && (editor->getHighlightedRegion().getLength() > 0);

    const bool canPaste = [[UIPasteboard generalPasteboard] hasStrings]
                       || [[UIPasteboard generalPasteboard] hasURLs]
                       || [[UIPasteboard generalPasteboard] hasImages]
                       || [[UIPasteboard generalPasteboard] hasColors];

    UIAction* cutAction =
        [UIAction actionWithTitle:@"Cut"
                            image:nil
                       identifier:nil
                          handler:^(__kindof UIAction* action)
        {
            (void) action;
            if (editor != nullptr)
                editor->cutToClipboard();
        }];

    UIAction* copyAction =
        [UIAction actionWithTitle:@"Copy"
                            image:nil
                       identifier:nil
                          handler:^(__kindof UIAction* action)
        {
            (void) action;
            if (editor != nullptr)
                editor->copyToClipboard();
        }];

    UIAction* pasteAction =
        [UIAction actionWithTitle:@"Paste"
                            image:nil
                       identifier:nil
                          handler:^(__kindof UIAction* action)
        {
            (void) action;
            if (editor != nullptr)
                editor->pasteFromClipboard();
        }];

    // Disable items when they don't apply
    if (!hasSelection) {
        cutAction.attributes  = UIMenuElementAttributesDisabled;
        copyAction.attributes = UIMenuElementAttributesDisabled;
    }
    if (!canPaste)
        pasteAction.attributes = UIMenuElementAttributesDisabled;

    NSMutableArray<UIMenuElement*>* children =
        [NSMutableArray arrayWithObjects:cutAction, copyAction, pasteAction, nil];

    // If you WANT Writing Tools / Apple suggestions as well, uncomment:
    [children addObjectsFromArray:suggestedActions];
    (void) suggestedActions;

    return [UIMenu menuWithTitle:@"" children:children];
}

@end

static JuceEditMenuDelegate* getJuceEditMenuDelegate()
{
    static JuceEditMenuDelegate* delegate = [JuceEditMenuDelegate new];
    return delegate;
}

#endif // __IPHONE_OS_VERSION_MAX_ALLOWED >= 160000

//==============================================================================
// iOS native edit menu show/hide

void hideIOSMenuNative()
{
    auto* focused = juce::Component::getCurrentlyFocusedComponent();
    auto* peer    = (focused != nullptr ? focused->getPeer() : nullptr);

    if (peer != nullptr)
    {
        if (auto* view = (UIView*) peer->getNativeHandle())
        {
            #if __IPHONE_OS_VERSION_MAX_ALLOWED >= 160000
            if (@available(iOS 16.0, *))
            {
                gCurrentMenuEditor = nullptr;

                for (UIInteraction* interaction in view.interactions)
                {
                    if ([interaction isKindOfClass:[UIEditMenuInteraction class]])
                    {
                        UIEditMenuInteraction* editInteraction = (UIEditMenuInteraction*) interaction;
                        [editInteraction dismissMenu];
                    }
                }
            }
            else
            #endif
            {
                #pragma clang diagnostic push
                #pragma clang diagnostic ignored "-Wdeprecated-declarations"  // legacy method
                [[UIMenuController sharedMenuController] hideMenu];
                #pragma clang diagnostic pop
            }
        }
    }
}

void showIOSMenuNative (juce::Component& component)
{
    if (auto* peer = component.getPeer())
    {
        if (auto* view = (UIView*) peer->getNativeHandle())
        {
            // Compute target rect similar to previous (legacy) logic 
            CGRect targetRect = CGRectMake (0, 0, view.bounds.size.width, view.bounds.size.height);

            if (auto* textEditor = dynamic_cast<juce::TextEditor*> (&component))
            {
                juce::Point<int> editorTopLeft = textEditor->getScreenBounds().getPosition();
                juce::Point<float> localPoint  = peer->globalToLocal (editorTopLeft.toFloat());

                CGFloat menuX = localPoint.x + (textEditor->getWidth() - 100) * 0.5f;  // centered horizontally
                CGFloat menuY = localPoint.y + 15;  // (+ x) Offset, possibly

                targetRect = CGRectMake (menuX, menuY, 100, 30);
            }
            #if __IPHONE_OS_VERSION_MAX_ALLOWED >= 160000
            if (@available(iOS 16.0, *))
            {
                // Track which JUCE editor should receive cut/copy/paste
                gCurrentMenuEditor = dynamic_cast<juce::TextEditor*> (&component);

                UIEditMenuInteraction* editInteraction = nil;

                for (UIInteraction* interaction in view.interactions) {
                    if ([interaction isKindOfClass:[UIEditMenuInteraction class]]) {
                        editInteraction = (UIEditMenuInteraction*) interaction;
                        break;
                    }
                }
                if (editInteraction == nil) {
                    editInteraction = [[UIEditMenuInteraction alloc] initWithDelegate:getJuceEditMenuDelegate()];
                    [view addInteraction:editInteraction];
                }
                UIEditMenuConfiguration* config =
                    [UIEditMenuConfiguration configurationWithIdentifier:nil
                                                             sourcePoint:CGPointMake (CGRectGetMidX (targetRect),
                                                                                     CGRectGetMinY (targetRect))];
                [editInteraction presentEditMenuWithConfiguration:config];
            }
            else
            #endif
            {
                #pragma clang diagnostic push
                #pragma clang diagnostic ignored "-Wdeprecated-declarations"  // legacy method
                if (dynamic_cast<juce::TextEditor*> (&component) != nullptr)
                {
                    [[UIMenuController sharedMenuController] showMenuFromView:view rect:targetRect];
                }
                else {   // Fallback: centered, if no TextEditor
                    CGRect fallbackRect = CGRectMake (0, 0, view.bounds.size.width, view.bounds.size.height);
                    [[UIMenuController sharedMenuController] showMenuFromView:view rect:fallbackRect];
                }
                #pragma clang diagnostic pop
            }
        }
    }
}

#endif // JUCE_IOS
