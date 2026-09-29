// AppUtilities.mm

#import <Foundation/Foundation.h>
#include <JuceHeader.h>
#include "AppUtilities.h"

#import <UIKit/UIKit.h>
@class UIInteraction;

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
        options.osxLibrarySubFolder  = "Application Support"; // important also in iOS !

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
}

//==============================================================================
//==============================================================================
// Detecting iPad (vs. iPhone)

bool isRunningOnIPad()
{
    return UIDevice.currentDevice.userInterfaceIdiom == UIUserInterfaceIdiomPad;
}
//==============================================================================
// Auto scroll
int getIOSKeyboardOverlap (juce::Component& component, int clearance)
{
    if (auto* peer = component.getPeer())
    {
        if (auto* view = (UIView*) peer->getNativeHandle())
        {
            [view layoutIfNeeded];
            const auto keyboardFrame = view.keyboardLayoutGuide.layoutFrame;
            const auto keyboardHeight = CGRectGetHeight (keyboardFrame);
            // When the keyboard is hidden, the keyboard layout guide
            // only represents the bottom safe-area region.
            if (keyboardHeight <= view.safeAreaInsets.bottom + 1.0)
                return 0;
            const auto componentBounds = peer->getAreaCoveredBy (component);
            const auto componentBottom = static_cast<float> (componentBounds.getBottom());
            auto keyboardTop = static_cast<float> (CGRectGetMinY (keyboardFrame));

            const auto overlap = componentBottom + static_cast<float> (clearance) - keyboardTop;
            return overlap > 0.0f ? juce::roundToInt (overlap) : 0;
        }
    }
    return 0;
}

//==============================================================================
// iOS 16+ Edit Menu delegate (Cut/Copy/Paste)
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 160000

static juce::Component::SafePointer<juce::TextEditor> gCurrentMenuEditor;
API_AVAILABLE(ios(16.0))
static UIEditMenuInteraction* gCurrentEditMenuInteraction = nil;

@interface JuceEditMenuDelegate : NSObject <UIEditMenuInteractionDelegate>
@end

@implementation JuceEditMenuDelegate

- (UIMenu*)editMenuInteraction:(UIEditMenuInteraction*)interaction
          menuForConfiguration:(UIEditMenuConfiguration*)configuration
              suggestedActions:(NSArray<UIMenuElement*>*)suggestedActions API_AVAILABLE(ios(16.0))
{
    (void) interaction;
    (void) configuration;

    auto editor = gCurrentMenuEditor;

    const bool hasSelection = (editor != nullptr)
                           && (editor->getHighlightedRegion().getLength() > 0);
    
    const bool hasText = (editor != nullptr)
                      && editor->getTotalNumChars() > 0;

    const bool allTextSelected = hasText
                              && editor->getHighlightedRegion().getStart() == 0
                              && editor->getHighlightedRegion().getEnd()
                                     == editor->getTotalNumChars();

    const bool canPaste = [[UIPasteboard generalPasteboard] hasStrings]
                       || [[UIPasteboard generalPasteboard] hasURLs];

    UIAction* cutAction =
        [UIAction actionWithTitle:@"Cut"
                            image:nil
                       identifier:nil
                          handler:^(__kindof UIAction* action)
        {
            (void) action;
            if (editor != nullptr) editor->cutToClipboard();
        }];

    UIAction* copyAction =
        [UIAction actionWithTitle:@"Copy"
                            image:nil
                       identifier:nil
                          handler:^(__kindof UIAction* action)
        {
            (void) action;
            if (editor != nullptr) editor->copyToClipboard();
        }];

    UIAction* pasteAction =
        [UIAction actionWithTitle:@"Paste"
                            image:nil
                       identifier:nil
                          handler:^(__kindof UIAction* action)
        {
            (void) action;
            if (editor != nullptr) editor->pasteFromClipboard();
        }];
        
    UIAction* selectAllAction =
		[UIAction actionWithTitle:@"Select All"
							image:nil
					   identifier:nil
						  handler:^(__kindof UIAction* action)
		{
			(void) action;
			if (editor == nullptr) return;
			editor->selectAll();
	
			auto safeEditor = editor;
			// Reopen the menu after UIKit has finished dismissing the current one.
			juce::Timer::callAfterDelay (100, [safeEditor] {
				if (safeEditor == nullptr) return;
				if (safeEditor->getHighlightedRegion().getLength() > 0)
					showIOSMenuNative (*safeEditor);
			});
		}];
        
    // Disable items when they don't apply
    if (!hasSelection) {
        cutAction.attributes  = UIMenuElementAttributesDisabled;
        copyAction.attributes = UIMenuElementAttributesDisabled;
    }
    if (!canPaste)
        pasteAction.attributes = UIMenuElementAttributesDisabled;
        
    if (!hasText || allTextSelected)
        selectAllAction.attributes = UIMenuElementAttributesDisabled;

    NSMutableArray<UIMenuElement*>* children =
        [NSMutableArray arrayWithObjects:cutAction, copyAction, pasteAction, selectAllAction, nil];

    // Include Writing Tools and other Apple-suggested actions:
	[children addObjectsFromArray:suggestedActions];
	// If suggestedActions are not included, use this instead to avoid an unused-parameter warning:
	// (void) suggestedActions;

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
    #if __IPHONE_OS_VERSION_MAX_ALLOWED >= 160000
    if (@available(iOS 16.0, *))
    {
        gCurrentMenuEditor = nullptr;

        if (gCurrentEditMenuInteraction != nil)
        {
            [gCurrentEditMenuInteraction dismissMenu];
            gCurrentEditMenuInteraction = nil;
        }

        return;
    }
    #endif

    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"  // legacy method
    [[UIMenuController sharedMenuController] hideMenu];
    #pragma clang diagnostic pop
}

void showIOSMenuNative (juce::Component& component)
{
    if (auto* peer = component.getPeer())
    {
        if (auto* view = (UIView*) peer->getNativeHandle())
        {
            CGRect targetRect = view.bounds;
			
			if (auto* textEditor = dynamic_cast<juce::TextEditor*> (&component))
			{
				const auto editorBounds = peer->getAreaCoveredBy (*textEditor);
			
				targetRect = CGRectMake (
					static_cast<CGFloat> (editorBounds.getX()),
					static_cast<CGFloat> (editorBounds.getY() + 15),  // (+ x) Offset
					static_cast<CGFloat> (editorBounds.getWidth()),
					static_cast<CGFloat> (editorBounds.getHeight()));
			}
            #if __IPHONE_OS_VERSION_MAX_ALLOWED >= 160000
            if (@available(iOS 16.0, *))
            {
                // Track which JUCE editor should receive cut/copy/paste
                if (auto* textEditor = dynamic_cast<juce::TextEditor*> (&component))
				{
					gCurrentMenuEditor = juce::Component::SafePointer<juce::TextEditor> (textEditor);
				}
				else { gCurrentMenuEditor = {}; }

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
                gCurrentEditMenuInteraction = editInteraction;
                
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
