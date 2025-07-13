// AppUtilities.mm

#import <Foundation/Foundation.h>
#include <JuceHeader.h>
#include "AppUtilities.h"
#if JUCE_IOS
    #import <UIKit/UIKit.h>
#endif

juce::File getAppGroupDirectory() 
{
	#if JUCE_IOS
	    if (selectDirectoryToggleState)
	    {   // "Documents" Directory (Button On-state)
            return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory);
        } else {
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
//		return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getChildFile ("URL Beamer");
	#endif
}

#if JUCE_IOS
void hideIOSMenuNative()
{
    if (auto* peer = juce::Component::getCurrentlyFocusedComponent()->getPeer())
    {
        if (auto* view = (UIView*)peer->getNativeHandle())
        {
            [[UIMenuController sharedMenuController] hideMenu];
        }
    }
}

void showIOSMenuNative (juce::Component& component)
{
    if (auto* peer = component.getPeer())
    {
        if (auto* view = (UIView*)peer->getNativeHandle())
        {
            if (auto* textEditor = dynamic_cast<juce::TextEditor*> (&component))
            {
                juce::Point<int> editorTopLeft = textEditor->getScreenBounds().getPosition();
                juce::Point<float> localPoint = peer->globalToLocal(editorTopLeft.toFloat());

                CGFloat menuX = localPoint.x + (textEditor->getWidth() - 100) / 2.0f;  // centered horizontally
                CGFloat menuY = localPoint.y;  // (localPoint.y - 10) Offset, possibly
                CGRect rect = CGRectMake (menuX, menuY, 100, 30);

                UIMenuController* menuController = [UIMenuController sharedMenuController];
                [menuController showMenuFromView:view rect:rect];
                return;
            }
            // Fallback: centered, if no TextEditor
            CGRect fallbackRect = CGRectMake (0, 0, view.bounds.size.width, view.bounds.size.height);
            [[UIMenuController sharedMenuController] showMenuFromView:view rect:fallbackRect];
        }
    }
}
#endif
