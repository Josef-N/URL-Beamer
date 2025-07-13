// AppUtilities.h

#pragma once
#include <JuceHeader.h>

// Declare the global variable for access in other files
extern bool selectDirectoryToggleState;

// Declare the function to get the appropriate directory based on the platform
juce::File getAppGroupDirectory();

#if JUCE_IOS
void hideIOSMenuNative();
void showIOSMenuNative (juce::Component& component);
#endif
