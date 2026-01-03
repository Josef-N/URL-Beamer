// AppUtilities.h

#pragma once
#include <JuceHeader.h>

// Declare the global variable for access in other files
extern bool selectDirectoryToggleState;

// Declare the function to get the appropriate directory based on the platform
juce::File getAppGroupDirectory();

// --------------------------------------------------
// Standalone settings (stored in app sandbox on iOS)
juce::PropertiesFile& getStandaloneProperties();

// Startup alert
bool shouldSuppressStandaloneAlert();
void setSuppressStandaloneAlert (bool shouldSuppress);

// File chooser UX
bool  getStandaloneSelectDirectoryToggleState();
void  setStandaloneSelectDirectoryToggleState (bool state);

juce::String getStandaloneLastUsedFilePath();
void         setStandaloneLastUsedFilePath (const juce::String& path);

bool getStandaloneOpenLastUsedFileOnStartup();
void setStandaloneOpenLastUsedFileOnStartup (bool shouldOpen);

// Safe Mode Check on startup
bool getStandaloneAutoloadInProgress();
void setStandaloneAutoloadInProgress (bool inProgress);

// --------------------------------------------------
#if JUCE_IOS
void hideIOSMenuNative();
void showIOSMenuNative (juce::Component& component);

bool isRunningOnIPad();
#endif
