//                 PluginEditor.cpp    –   JUCE plugin editor

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "CustomFileChooser.h"
#include "AppUtilities.h" // for AppGroup

//==============================================================================
TextEditorPopup::TextEditorPopup
    (juce::TextButton& button1, juce::TextButton& button2, juce::TextButton& button3, juce::TextButton& button4, 
     juce::TextButton& button5, juce::TextButton& button6, juce::TextButton& button7, juce::TextButton& button8,
     juce::HyperlinkButton& Link1, juce::HyperlinkButton& Link2, juce::HyperlinkButton& Link3, juce::HyperlinkButton& Link4, 
     juce::HyperlinkButton& Link5, juce::HyperlinkButton& Link6, juce::HyperlinkButton& Link7, juce::HyperlinkButton& Link8)
  : button1Ref (button1), button2Ref (button2), button3Ref (button3), button4Ref (button4), 
    button5Ref (button5), button6Ref (button6), button7Ref (button7), button8Ref (button8), 
    Link1Ref (Link1), Link2Ref (Link2), Link3Ref (Link3), Link4Ref (Link4), 
    Link5Ref (Link5), Link6Ref (Link6), Link7Ref (Link7), Link8Ref (Link8)
{   
    setOpaque (true);
    setWantsKeyboardFocus (true);
    
    addAndMakeVisible (scrollView);
	scrollView.setViewedComponent (&contentArea, false); // false = don't delete
	scrollView.setScrollBarsShown (false, true);         // only vertical scroll
    contentArea.setWantsKeyboardFocus (true);

    auto setupTextEditor = [this](juce::TextEditor& editor)
	{
        editor.setMultiLine (false);
        editor.setScrollToShowCursor (false);
		editor.setJustification (juce::Justification::centredLeft);  // 0xFF122566 blue, 0xFF161C3C dark blue
		editor.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xFF161C3C));
		editor.setHasFocusOutline (true);
		editor.onFocusLost = [&editor, this] { 
		    editor.setCaretPosition (0); // deselect All
			if (expandedEditor == &editor) expandedEditor = nullptr;
		    lastScrollY = scrollView.getVerticalScrollBar().getCurrentRangeStart();
		    resized(); };
        juce::Timer::callAfterDelay (50, [this, &editor] { editor.setScrollToShowCursor (true);
                                                           editor.setCaretPosition (0); } );
        editor.addMouseListener (this, false); // Register mouseUp + mouseEnter as MouseListener
#if JUCE_IOS
		editor.onTextChange = []() { hideIOSMenuNative(); };
#endif
	};
	allEditors = {
	    &Name1, &text1, &Name2, &text2, &Name3, &text3, &Name4, &text4,
		&Name5, &text5, &Name6, &text6, &Name7, &text7, &Name8, &text8
	};
	
	int focusOrder = 1; // Start with 1
	
	for (auto* editor : allEditors) {  // Apply to all TextEditors
		setupTextEditor (*editor);
		contentArea.addAndMakeVisible (*editor);
		editor->setExplicitFocusOrder (focusOrder++);  // Order for Tab key
		editor->onReturnKey = [this] { OK.triggerClick(); };
	}
	
    Name1.setText (button1.getButtonText()); // Load existing button text    
    Name2.setText (button2.getButtonText());
    Name3.setText (button3.getButtonText());
    Name4.setText (button4.getButtonText());
    Name5.setText (button5.getButtonText());
    Name6.setText (button6.getButtonText());
    Name7.setText (button7.getButtonText());
    Name8.setText (button8.getButtonText());
    text1.setText (Link1.getURL().toString (true)); // Load existing URL
    text2.setText (Link2.getURL().toString (true));
    text3.setText (Link3.getURL().toString (true)); // .toString (bool includeGetParameters)
    text4.setText (Link4.getURL().toString (true)); // false: returns the “sanitized” version of the URL
    text5.setText (Link5.getURL().toString (true));
    text6.setText (Link6.getURL().toString (true));
    text7.setText (Link7.getURL().toString (true));
    text8.setText (Link8.getURL().toString (true));

    contentArea.addAndMakeVisible (OK);                        // darkorchid
	OK.setColour (juce::TextButton::buttonColourId, juce::Colour (0xFFA100C4));
    OK.setButtonText ("OK");
    OK.onClick = [this]() {
        button1Ref.setButtonText (Name1.getText()); // Update button texts
        button2Ref.setButtonText (Name2.getText());
        button3Ref.setButtonText (Name3.getText());
        button4Ref.setButtonText (Name4.getText());
        button5Ref.setButtonText (Name5.getText());
        button6Ref.setButtonText (Name6.getText());
        button7Ref.setButtonText (Name7.getText());
        button8Ref.setButtonText (Name8.getText());
        Link1Ref.setURL (juce::URL::createWithoutParsing (text1.getText())); // Update Link1 URL
        Link2Ref.setURL (juce::URL::createWithoutParsing (text2.getText()));
        Link3Ref.setURL (juce::URL::createWithoutParsing (text3.getText()));
        Link4Ref.setURL (juce::URL::createWithoutParsing (text4.getText()));
        Link5Ref.setURL (juce::URL::createWithoutParsing (text5.getText()));
        Link6Ref.setURL (juce::URL::createWithoutParsing (text6.getText()));
        Link7Ref.setURL (juce::URL::createWithoutParsing (text7.getText()));
        Link8Ref.setURL (juce::URL::createWithoutParsing (text8.getText()));
        // Sync the updated state to the ValueTree
		if (auto* editor = dynamic_cast<CallAppAudioProcessorEditor*>(getParentComponent()))
		{
			editor->triggerStateSave();
		}
#if JUCE_IOS
		hideIOSMenuNative();
#endif
        if (onClose) onClose(); // removeChildComponent, textEditorPopup.reset (= delete)
    };

    contentArea.addAndMakeVisible (CancelText);
    CancelText.setText ("Cancel", juce::dontSendNotification);
    
    contentArea.addAndMakeVisible (Cancel);
    Cancel.setColour (juce::TextButton::buttonColourId, juce::Colour (0x00000000));
	Cancel.onClick = [this] { if (onClose) onClose(); };  // close + reset
	
    contentArea.addAndMakeVisible (Color);
    Color.setColour (juce::TextButton::buttonColourId, juce::Colour (0xFF122566));
    Color.setButtonText ("Color");
    Color.onClick = [this]() {
		if (onColorButtonClick) onColorButtonClick();
		OK.triggerClick(); };

	contentArea.addAndMakeVisible (Note);
	Note.setColour (juce::TextButton::buttonColourId, juce::Colour (0xFF122566));
	Note.setButtonText (juce::CharPointer_UTF8 ("\u266B"));  // ♫ \u266B
	Note.onClick = [this]() {
		if (onNoteButtonClick) onNoteButtonClick();
		OK.triggerClick(); };
	
    if (juce::JUCEApplicationBase::isStandaloneApp() && shouldSuppressStandaloneAlert())
	    { contentArea.addAndMakeVisible (reset); }
	reset.setColour (juce::TextButton::textColourOffId, juce::Colour (0xFF0096FF));
    reset.setButtonText ("reset");
    reset.onClick = [] { setSuppressStandaloneAlert (false); };
    
	contentArea.addAndMakeVisible (Commit);
	Commit.setText ("OK, Color, Note: commit changes in text fields", juce::dontSendNotification);
	Commit.setJustificationType	(juce::Justification::topRight);
	
	if (juce::JUCEApplicationBase::isStandaloneApp() && shouldSuppressStandaloneAlert())
	    { contentArea.addAndMakeVisible (resetLabel); }
	resetLabel.setText ("Show startup message again", juce::dontSendNotification);
	resetLabel.setJustificationType (juce::Justification::topLeft);
	
	resized(); // Safe default layout
}

TextEditorPopup::~TextEditorPopup() {}

void TextEditorPopup::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void TextEditorPopup::resized()
{ 
    lastScrollY = scrollView.getViewPositionY(); // Capture current scroll position
    
    auto top = 8;   // Default for Plugin (content padding inside the viewport)
    auto border = 8;
    int pinnedTopInset = 0; // NON-scrolling inset (viewport pushed down)
    
    #if JUCE_IOS
    if (juce::JUCEApplicationBase::isStandaloneApp()) {  // if Standalone:
		const bool portrait = (getHeight() > getWidth());
	              // iPad
		if (isRunningOnIPad()) {
            pinnedTopInset = 76;  // Stage Manager / gesture safe area
            top = 6;  // Keep content padding small, because the big space is now "pinned"
            border = portrait ? 8 : 64;  // Portrait : Landscape
		} else {  // iPhone
            if (portrait) {  // Portrait
                top = 64;         // Notch: 44 points, Dynamic Island: 48 points
                border = 8;
            } else {         // Landscape
                top = 30;
                border = 64;
        }   }
    }  // if Plugin:
    else if (getWidth() > 560) {  // Save for GarageBand + iPhone SE 4' (w = 568 pt)
		top = 8;
		border = 64;
	}
	#else  // (#elif) JUCE_MAC
	top = 8;
	if (getWidth() > 560) { border = 54; }
	#endif
	// IMPORTANT: apply pinned top inset to the viewport so it doesn't scroll away
    scrollView.setBounds (getLocalBounds().withTrimmedTop (pinnedTopInset));
	
    auto gap = 10;
	auto textHeight = 30;  // Editor: 22
    auto buttonHeight = 44;
	auto rightButtonW = 50;
	auto oneRowWidth =  (getWidth() - (2 * border + gap + rightButtonW));          // 1 button row
	auto twoRowsWidth = (getWidth() - (2 * border + 2 * gap + rightButtonW)) / 2;  // 2 button rows
	auto xPosRow2 = twoRowsWidth + border + gap;
	
	auto getWidthFor = [&](juce::TextEditor* editor) -> int {
        return (editor == expandedEditor) ? oneRowWidth : twoRowsWidth;
    };
    auto getXPosFor = [&](juce::TextEditor* editor, bool isRow1) -> int {
        return (editor == expandedEditor) ? border : (isRow1 ? border : xPosRow2);
    };
	for (auto* editor : allEditors) {
		if (editor != expandedEditor) editor->toBack();
	}	
	
    Name1.setBounds (getXPosFor (&Name1, true), top,                     getWidthFor (&Name1), textHeight);
    text1.setBounds (getXPosFor (&text1, true), top + textHeight,        getWidthFor (&text1), textHeight);

    Name2.setBounds (getXPosFor (&Name2, true), text1.getBottom() + gap, getWidthFor (&Name2), textHeight);
    text2.setBounds (getXPosFor (&text2, true), Name2.getBottom(),       getWidthFor (&text2), textHeight);

    Name3.setBounds (getXPosFor (&Name3, true), text2.getBottom() + gap, getWidthFor (&Name3), textHeight);
    text3.setBounds (getXPosFor (&text3, true), Name3.getBottom(),       getWidthFor (&text3), textHeight);

    Name4.setBounds (getXPosFor (&Name4, true), text3.getBottom() + gap, getWidthFor (&Name4), textHeight);
    text4.setBounds (getXPosFor (&text4, true), Name4.getBottom(),       getWidthFor (&text4), textHeight);

    Name5.setBounds (getXPosFor (&Name5, false), Name1.getY(), getWidthFor (&Name5), textHeight);
    text5.setBounds (getXPosFor (&text5, false), text1.getY(), getWidthFor (&text5), textHeight);

    Name6.setBounds (getXPosFor (&Name6, false), Name2.getY(), getWidthFor (&Name6), textHeight);
    text6.setBounds (getXPosFor (&text6, false), text2.getY(), getWidthFor (&text6), textHeight);

    Name7.setBounds (getXPosFor (&Name7, false), Name3.getY(), getWidthFor (&Name7), textHeight);
    text7.setBounds (getXPosFor (&text7, false), text3.getY(), getWidthFor (&text7), textHeight);

    Name8.setBounds (getXPosFor (&Name8, false), Name4.getY(), getWidthFor (&Name8), textHeight);
    text8.setBounds (getXPosFor (&text8, false), text4.getY(), getWidthFor (&text8), textHeight);
    
    OK.setBounds (getWidth() - border - rightButtonW, top,    rightButtonW, buttonHeight);
    Cancel.setBounds     (OK.getX(), OK.getBottom() + gap,    rightButtonW, buttonHeight);
    CancelText.setBounds (OK.getX(), Cancel.getY(),           rightButtonW, buttonHeight);
    Note.setBounds       (OK.getX(), Name4.getY() + 16,       rightButtonW, buttonHeight);
    Color.setBounds      (OK.getX(), Note.getY()  - 54,       rightButtonW, buttonHeight);
    reset.setBounds      (border,    text4.getBottom() + 30,  60,           30);
    
    Commit.setBounds     (border,           text4.getBottom() + gap,            getWidth() - (2 * border),  16);
    resetLabel.setBounds (reset.getRight(), reset.getY() + 8, getWidth() - (2 * border + reset.getWidth()), 70);
    
    int contentHeight = resetLabel.getBottom() + 90;  // final vertical extent
    // Use viewport width (safe if viewport is trimmed or later you change widths)
    contentArea.setBounds (0, 0, scrollView.getWidth(), contentHeight);
    scrollView.setViewPosition (0, static_cast<int> (lastScrollY)); // Restore scroll position
}

//==============================================================================
CallAppAudioProcessorEditor::CallAppAudioProcessorEditor (CallAppAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    link1Value = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.treeState, "app1", Link1);
	link2Value = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.treeState, "app2", Link2);
	link3Value = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.treeState, "app3", Link3);
	link4Value = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.treeState, "app4", Link4);
    link5Value = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.treeState, "app5", Link5);
	link6Value = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.treeState, "app6", Link6);
	link7Value = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.treeState, "app7", Link7);
	link8Value = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.treeState, "app8", Link8);
	
	// Apply deferred state (editor is not open) if it exists, using the public getter
    if (audioProcessor.getDeferredState().isValid())
    {
        loadStateFromValueTree (audioProcessor.getDeferredState());
        audioProcessor.clearDeferredState();  // Clear after loading
    }
 
    setSize (500, 360);
    setResizable (true, false);
    setOpaque (true);
    
    addAndMakeVisible (scrollView);
    scrollView.setViewedComponent (&scrollContent, false); // false = don't delete
    scrollView.setScrollBarsShown (false, true);           // Only vertical scroll
    
    auto setupButton = [this](juce::TextButton& button, int index) {
		button.onClick = [this, &button, index]() {
			colorRange.getToggleState(); 
			selectButtonLabels (index); 
			selectedButton = &button; 
			updateSliderFromButton(); 
			syncTextColorToggle(); 
			updateNoteSliderFromButton(); 
			updateChannelSliderFromButton(); };
		scrollContent.addAndMakeVisible (button);
		button.setVisible (index <= 4); // Show only buttons 1–4 initially
    };  // Apply to buttons 1–8:
	setupButton (button1, 1); setupButton (button2, 2); setupButton (button3, 3); setupButton (button4, 4);
	setupButton (button5, 5); setupButton (button6, 6); setupButton (button7, 7); setupButton (button8, 8);
    
    button1.setButtonText ("Midi Designer");  // mediumspringgreen  juce::Colour (0xff19FAA4)
    button2.setButtonText ("PdParty");
    button3.setButtonText ("Apple Books");
    button4.setButtonText ("Web");
    button5.setButtonText ("...");
    button6.setButtonText ("...");
    button7.setButtonText ("...");
    button8.setButtonText ("...");
    
    auto setupLink = [this] (juce::HyperlinkButton& link) {
		link.setClickingTogglesState (true);
		link.setTriggeredOnMouseDown (true);
		link.addMouseListener (this, false);  // mouseDown + mouseUp
		scrollContent.addAndMakeVisible (link);
	};  // Array for all links:
	juce::HyperlinkButton* allLinks[] = { &Link1, &Link2, &Link3, &Link4, &Link5, &Link6, &Link7, &Link8 };
	for (auto* link : allLinks) { setupLink (*link); }
                                 
    Link1.setURL (juce::URL ("MidiDesignerPro.audiobus://")); // flashButton: simulates button1 clicked (visual feedback)
    Link1.onClick = [this]() { flashButton (&button1); updateNoteNumberFromLink (&button1);
                                                    CallAppAudioProcessorEditor::repaint(); };
    Link2.setURL (juce::URL ("pdparty://"));
    Link2.onClick = [this]() { flashButton (&button2); updateNoteNumberFromLink (&button2); 
                                                    CallAppAudioProcessorEditor::repaint(); };
    Link3.setURL (juce::URL ("ibooks://"));
    Link3.onClick = [this]() { flashButton (&button3); updateNoteNumberFromLink (&button3); 
                                                    CallAppAudioProcessorEditor::repaint(); };
    Link4.setURL (juce::URL ("https://novotny.klingt.org/Apps/URLBeamer/Support"));
    Link4.onClick = [this]() { flashButton (&button4); updateNoteNumberFromLink (&button4); 
                                                    CallAppAudioProcessorEditor::repaint(); };
    Link5.setVisible (false);
    Link5.setURL (juce::URL (""));
    Link5.onClick = [this]() { flashButton (&button5); updateNoteNumberFromLink (&button5); 
                                                    CallAppAudioProcessorEditor::repaint(); };
    Link6.setVisible (false);
    Link6.setURL (juce::URL (""));
    Link6.onClick = [this]() { flashButton (&button6); updateNoteNumberFromLink (&button6); 
                                                    CallAppAudioProcessorEditor::repaint(); };
    Link7.setVisible (false);
    Link7.setURL (juce::URL (""));
    Link7.onClick = [this]() { flashButton (&button7); updateNoteNumberFromLink (&button7); 
                                                    CallAppAudioProcessorEditor::repaint(); };
    Link8.setVisible (false);
    Link8.setURL (juce::URL (""));
    Link8.onClick = [this]() { flashButton (&button8); updateNoteNumberFromLink (&button8); 
                                                    CallAppAudioProcessorEditor::repaint(); };
    // Repaint Editor because: if opening another app via hyperlink and coming back, also other buttons are dirty
    
    scrollContent.addAndMakeVisible (ColorSlider);
    ColorSlider.setViewportIgnoreDragFlag (true);
    ColorSlider.setVisible (false);            // (MouseListener = for iOS)
    ColorSlider.addMouseListener (this, true); // true: Listen to any child component within this component
    ColorSlider.setRange (0, 0xFFFFFF, 1);     // RGB range
    ColorSlider.setTextBoxStyle (juce::Slider::TextBoxAbove, false, 80, 22);
    ColorSlider.onValueChange = [this] { updateButtonColorFromSlider(); };
    ColorSlider.textFromValueFunction = [](double value) { return ""; }; // No disply for 1st Nr
    ColorSlider.valueFromTextFunction = [this](const juce::String& text) 
    { return static_cast<double>(text.getHexValue32() & 0xFFFFFF); };
    
    scrollContent.addAndMakeVisible (ColorLabel);
    ColorLabel.setVisible (false);
    ColorLabel.setText ("Select button for color change", juce::dontSendNotification);
    ColorLabel.setJustificationType (juce::Justification::left);
    
    scrollContent.addAndMakeVisible (TextColor);
    TextColor.setVisible (false);
    TextColor.setButtonText ("Text color b/w");
    TextColor.onClick = [this]() { updateButtonTextColor(); };
    TextColor.setToggleState (false, juce::dontSendNotification); // false = white (off)
    
    scrollContent.addAndMakeVisible (DummyLabel);
    DummyLabel.setVisible (false);
    
    scrollContent.addAndMakeVisible (colorRange);
    colorRange.setVisible (false);
    colorRange.setButtonText ("Full");  // Default state
    colorRange.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xFF69697F)); // dimgrey
    colorRange.setClickingTogglesState (true);
    colorRange.setToggleState (false, juce::dontSendNotification); // Default = Full (off)
    colorRange.onClick = [this] { updateColorSliderRange(); };
    
    scrollContent.addAndMakeVisible (RangeLabel);
    RangeLabel.setVisible (false);
    RangeLabel.setText ("Full RGB Range / 118 Colors (temporary)", juce::dontSendNotification);
    
    scrollContent.addAndMakeVisible (midiNoteSlider);
    midiNoteSlider.setViewportIgnoreDragFlag (true);
    midiNoteSlider.setVisible (false);            // (MouseListener = for iOS)
    midiNoteSlider.addMouseListener (this, true); // true: Listen to any child component within this component
    midiNoteSlider.setRange (0, 127, 1);
    midiNoteSlider.setTextBoxStyle (juce::Slider::TextBoxAbove, false, 80, 22);
    midiNoteSlider.onValueChange = [this] { updateNoteNumberFromSlider(); };
    midiNoteSlider.textFromValueFunction = [](double value) -> juce::String {
		static const juce::StringArray noteNames = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
		int midiNote = static_cast<int> (value);
		int octave = (midiNote / 12) - 1; // MIDI note 60 = C4
		juce::String noteName = noteNames[midiNote % 12] + juce::String (octave);
		return noteName + " (" + juce::String (midiNote) + ")"; // Example: "C4 (60)"
	};
	midiNoteSlider.valueFromTextFunction = [](const juce::String& text) -> double {
		static const juce::StringArray noteNames = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
		juce::String input = text.trim().toUpperCase();
		for (int i = 0; i < 128; ++i) {  // Check if input is a valid note name
			int octave = (i / 12) - 1;
			juce::String expectedNote = noteNames[i % 12] + juce::String (octave);
			if (input == expectedNote) {
				return static_cast<double>(i);
			}
		}
		if (input.containsOnly ("0123456789")) {  // If input is purely a number
			int midiNumber = input.getIntValue();
			if (midiNumber >= 0 && midiNumber <= 127) {
				return static_cast<double> (midiNumber);
			}
		}
		return 60.0; // Default to C4 if input is invalid
	};
    
    scrollContent.addAndMakeVisible (midiNoteLabel);
    midiNoteLabel.setVisible (false);
    midiNoteLabel.setText ("Select button for Midi Note", juce::dontSendNotification);
    midiNoteLabel.setJustificationType (juce::Justification::left);

    scrollContent.addAndMakeVisible (ChannelLabel);
    ChannelLabel.setVisible (false);
    ChannelLabel.setText ("Channel", juce::dontSendNotification);
    
    scrollContent.addAndMakeVisible (ChannelSlider);
    ChannelSlider.setViewportIgnoreDragFlag (true);
    ChannelSlider.setVisible (false);
    ChannelSlider.setRange (0, 16, 1);
    ChannelSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 30, 22);
    ChannelSlider.onValueChange = [this] { updateChannelNumberFromSlider(); };
    ChannelSlider.textFromValueFunction = [](double value) -> juce::String 
    { return (static_cast<int> (value) == 0) ? "Off" : juce::String (static_cast<int> (value)); };
    
    scrollContent.addAndMakeVisible (OffsetLabel);
    OffsetLabel.setVisible (false);
    OffsetLabel.setText ("Offset", juce::dontSendNotification);
    
    scrollContent.addAndMakeVisible (MidiThruLabel);
    MidiThruLabel.setVisible (false);
    MidiThruLabel.setText ("Notes in range:", juce::dontSendNotification);
    MidiThruLabel.setJustificationType (juce::Justification::topRight);
    
    scrollContent.addAndMakeVisible (InNoteSlider);
    InNoteSlider.setViewportIgnoreDragFlag (true);
    InNoteSlider.setVisible (false);
    InNoteSlider.setRange (0, 120, 1); // Offset
    InNoteSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 40, 22);
    InNoteSlider.setValue (1, juce::sendNotificationSync);
    InNoteSlider.onValueChange = [this]() { updateInInfoLabel(); };
    
    scrollContent.addAndMakeVisible (InChannelLabel);
    InChannelLabel.setVisible (false);
    InChannelLabel.setText ("Channel", juce::dontSendNotification);
    
    scrollContent.addAndMakeVisible (InChannelSlider);
    InChannelSlider.setViewportIgnoreDragFlag (true);
    InChannelSlider.setVisible (false);
    InChannelSlider.setRange (0, 17, 1); // 0 = Off, 17 = Omni
    InChannelSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 30, 22);
    InChannelSlider.textFromValueFunction = [](double value) -> juce::String {
		int val = static_cast<int> (value);
		if (val == 0) return "Off"; if (val == 17) return "All";
		return juce::String (val); };
    InChannelSlider.valueFromTextFunction = [](const juce::String& text) -> double {
		juce::String t = text.trim().toLowerCase();
		if (t == "off") return 0; if (t == "all") return 17;
		return t.getIntValue(); }; // fallback for numbers
    
    scrollContent.addAndMakeVisible (InInfoLabel);
    InInfoLabel.setVisible (false);
    InInfoLabel.setText (juce::CharPointer_UTF8
    ("Receive: Midi Note# 1–8 \xe2\x9e\x9e Button 1–8"), juce::dontSendNotification); // ➞ (➜)
    
    scrollContent.addAndMakeVisible (MidiThru);
    MidiThru.setVisible (false);
	MidiThru.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xFF69697F)); // dimgrey
    MidiThru.setClickingTogglesState (true);
    MidiThru.setToggleState (false, juce::dontSendNotification); // Default = Pass Notes (off)
    MidiThru.setButtonText (MidiThru.getToggleState() ? "Block" : "Pass");
	MidiThru.onClick = [this]() 
	{ MidiThru.setButtonText (MidiThru.getToggleState() ? "Block" : "Pass"); };

    scrollContent.addAndMakeVisible (Edit);
    Edit.setColour (juce::TextButton::buttonColourId, juce::Colour (0xFF122566)); // steelblue
    Edit.setButtonText ("Edit");
    Edit.onClick = [this] { editButtonClicked(); };
    
    scrollContent.addAndMakeVisible (ToggleRows);
    ToggleRows.setColour (juce::TextButton::buttonColourId, juce::Colour (0xFF69697F)); // dimgrey
    ToggleRows.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    ToggleRows.setColour (juce::TextButton::buttonOnColourId, juce::Colours::lightgrey);
    ToggleRows.setColour (juce::TextButton::textColourOnId, juce::Colours::black);
    ToggleRows.setButtonText (juce::CharPointer_UTF8 ("\u2B95\n8")); // ➡ ︎\u2B95
    ToggleRows.setClickingTogglesState (true); // automatically flip the toggle state
    ToggleRows.setToggleState (false, juce::dontSendNotification); // on/off (sendNotification)
    ToggleRows.onClick = [this] { rowsCount(), resized(); };           // dontSendNotification
    
    scrollContent.addAndMakeVisible (menu);                                       // darkslategrey
    menu.setColour (juce::TextButton::buttonColourId, juce::Colour (0xFF122566));
    menu.setButtonText (juce::CharPointer_UTF8 ("\u2630"));  // ☰ \2630 Hamburger menu
    menu.onClick = [this]() { openMenu(); };

    scrollContent.addAndMakeVisible (Info);
    Info.setColour (juce::TextButton::textColourOffId, juce::Colours::yellow);
    Info.setButtonText ("?");
    Info.onClick = [] { juce::URL ("https://novotny.klingt.org/Apps/URLBeamer/UserGuide").launchInDefaultBrowser(); };

    scrollContent.addAndMakeVisible (exitColor);
    exitColor.setVisible (false);
    exitColor.setColour (juce::TextButton::buttonColourId, juce::Colour (0xFFA100C4));
    exitColor.setButtonText (juce::CharPointer_UTF8 ("\u2718 color")); // ✘ \u2718 \n new line
    exitColor.onClick = [this] { exitColorButtonClicked(); };          // ❌ \u274C 
    
    scrollContent.addAndMakeVisible (exitMidi);
    exitMidi.setVisible (false);
    exitMidi.setColour (juce::TextButton::buttonColourId, juce::Colour (0xFFA100C4));
    exitMidi.setButtonText (juce::CharPointer_UTF8 ("\u2718 midi"));
    exitMidi.onClick = [this] { exitMidiButtonClicked(); };
    
    scrollContent.addAndMakeVisible (MidiSendLabel);
    MidiSendLabel.setVisible (false);
    MidiSendLabel.setText ("Midi Send", juce::dontSendNotification);
    
    scrollContent.addAndMakeVisible (lockedLabel);
    lockedLabel.setVisible (false);
    lockedLabel.setJustificationType (juce::Justification::centred);
    
    scrollContent.addAndMakeVisible (editLabel);
    editLabel.setVisible (false);
    editLabel.setText ("Edit Mode: buttons inactive", juce::dontSendNotification);
    editLabel.setFont (juce::Font (juce::FontOptions().withPointHeight (14.0f)));
    editLabel.setColour (juce::Label::textColourId, juce::Colours::yellow);
    
    messageLabel.setJustificationType (juce::Justification::centred);
    messageLabel.setColour (juce::Label::textColourId, juce::Colours::red);
    messageLabel.setColour (juce::Label::backgroundColourId, juce::Colour (0x96FFFFFF)); // opacity 150 dez
    messageLabel.setColour (juce::Label::outlineColourId, juce::Colours::white);
    messageLabel.setFont (juce::Font (juce::FontOptions().withPointHeight (18.0f)));
    addChildComponent (messageLabel); // Fixed position, not part of scrollContent
    
    // Defaults on load
  	ChannelSlider.updateText();   // Ensure "Off" (0) is shown
	InChannelSlider.updateText();
	
	resized(); // ensure immediate layout on load
	
	if (juce::JUCEApplicationBase::isStandaloneApp())
	{	// Load toggle from persistent settings into the global used by getAppGroupDirectory()
		selectDirectoryToggleState = getStandaloneSelectDirectoryToggleState();
		// Show alert first, then autoload from its callback
		if (! shouldSuppressStandaloneAlert()) {
			juce::MessageManager::callAsync ([safe = SafePointer<CallAppAudioProcessorEditor>(this)]
			{	if (safe == nullptr) return;
				safe->showAlertWindow();
			});
		} else { triggerStandaloneAutoload(); }  // No alert -> autoload immediately
	}
}

CallAppAudioProcessorEditor::~CallAppAudioProcessorEditor() {}

//==============================================================================
void CallAppAudioProcessorEditor::showAlertWindow()
{
    const auto message = juce::CharPointer_UTF8 (
        "This is an AUv3 plugin intended to be loaded into a host app such as AUM."
        "\n\nIt can however be run in standalone mode with limited features:"
        "\n\u2713 HyperLink Buttons" "\n\u2713 Save/Load settings" "\n   (shared with plugin)"
        "\n\u2718 No MIDI output"
    );
    auto options = juce::MessageBoxOptions()
        .withIconType (juce::MessageBoxIconType::NoIcon)
        .withTitle ("URL Beamer")
        .withMessage (message)
        .withButton ("OK")                          // index 0 (blue)
        .withButton ("Don't show this again")       // index 1
        .withAssociatedComponent (this);
    SafePointer<CallAppAudioProcessorEditor> safeThis (this);
    
    juce::NativeMessageBox::showAsync (options, [safeThis] (int result)
    {
        if (safeThis == nullptr)
            return;
        if (result == 0) { }                    // "OK"
        else if (result == 1)
        { setSuppressStandaloneAlert (true); }  // "Don't show again"
        // Trigger autoload after the dialog is dismissed
        safeThis->triggerStandaloneAutoload();
    });
}
//==============================================================================
void CallAppAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void CallAppAudioProcessorEditor::resized()
{
    lastScrollY = scrollView.getViewPositionY(); // ← capture before layout
    
    if (textEditorPopup)
        textEditorPopup->setBounds (getLocalBounds());
        
    if (fileChooser != nullptr && fileChooser->isVisible()) {
        fileChooser->setBounds (getLocalBounds());
    }
    
    bool toggleState = ToggleRows.getToggleState(); // toggle state: on/off
    if   (toggleState == true) { buttonRows = 2; }
    else                       { buttonRows = 1; }
    
    auto top = 8;   // Default for Plugin (content padding inside the viewport)
    auto border = 8;
    int pinnedTopInset = 0; // NON-scrolling inset (viewport pushed down)
    
    #if JUCE_IOS
    if (juce::JUCEApplicationBase::isStandaloneApp()) {  // if Standalone:
		const bool portrait = (getHeight() > getWidth());
	              // iPad
		if (isRunningOnIPad()) {
            pinnedTopInset = 76;  // Stage Manager / gesture safe area
            top = 6;  // Keep content padding small, because the big space is now "pinned"
            border = portrait ? 8 : 64;  // Portrait : Landscape
		} else {  // iPhone
            if (portrait) {  // Portrait
                top = 64;         // Notch: 44 points, Dynamic Island: 48 points
                border = 8;
            } else {         // Landscape
                top = 30;
                border = 64;
        }   }
    }  // if Plugin:
    else if (getWidth() > 560) {  // Save for GarageBand + iPhone SE 4' (w = 568 pt)
		top = 8;
		border = 64;
	}
	#else  // (#elif) JUCE_MAC
	top = 8;
	if (getWidth() > 560) { border = 54; }
	#endif
	// IMPORTANT: apply pinned top inset to the viewport so it doesn't scroll away
    scrollView.setBounds (getLocalBounds().withTrimmedTop (pinnedTopInset));
	
    auto gap = 10;
	auto textHeight = 22;
    auto buttonHeight = 44;
	auto rightButtonW = 50;
	auto oneRowWidth =   getWidth() - (2 * border + gap + rightButtonW);          // 1 button row
	auto twoRowsWidth = (getWidth() - (2 * border + 2 * gap + rightButtonW)) / 2; // 2 button rows
	auto xPosRow2 = twoRowsWidth + border + gap;

    if (buttonRows == 1) { buttonWidth = oneRowWidth; }
    if (buttonRows == 2) { buttonWidth = twoRowsWidth; }

    button1.setBounds (border, top, buttonWidth, buttonHeight);
    Link1.setBounds   (border, top, buttonWidth, buttonHeight);

	button2.setBounds (border, button1.getBottom() + gap, buttonWidth, buttonHeight);
	Link2.setBounds   (border, button1.getBottom() + gap, buttonWidth, buttonHeight);

	button3.setBounds (border, button2.getBottom() + gap, buttonWidth, buttonHeight);
	Link3.setBounds   (border, button2.getBottom() + gap, buttonWidth, buttonHeight);

	button4.setBounds (border, button3.getBottom() + gap, buttonWidth, buttonHeight);
	Link4.setBounds   (border, button3.getBottom() + gap, buttonWidth, buttonHeight);
	
    button5.setBounds (xPosRow2, button1.getY(), buttonWidth, buttonHeight);
    Link5.setBounds   (xPosRow2, button1.getY(), buttonWidth, buttonHeight);
    
    button6.setBounds (xPosRow2, button2.getY(), buttonWidth, buttonHeight);
    Link6.setBounds   (xPosRow2, button2.getY(), buttonWidth, buttonHeight);
    
    button7.setBounds (xPosRow2, button3.getY(), buttonWidth, buttonHeight);
    Link7.setBounds   (xPosRow2, button3.getY(), buttonWidth, buttonHeight);
    
    button8.setBounds (xPosRow2, button4.getY(), buttonWidth, buttonHeight);
    Link8.setBounds   (xPosRow2, button4.getY(), buttonWidth, buttonHeight);
    
    ColorSlider.setBounds (border,         button4.getBottom() + 25,      oneRowWidth,        buttonHeight);
    ColorLabel.setBounds  (border,         button4.getBottom() + 25,      twoRowsWidth - 36,  textHeight);
    TextColor.setBounds   (xPosRow2 + 40,  ColorSlider.getY(),            twoRowsWidth - 40,  textHeight);
    DummyLabel.setBounds  (xPosRow2 + 160, ColorSlider.getY(),            twoRowsWidth - 160, textHeight);
    colorRange.setBounds  (border,         ColorSlider.getBottom() + gap, 60,                 40);
    RangeLabel.setBounds  (colorRange.getRight() + 4, colorRange.getY(),  oneRowWidth - colorRange.getWidth() - 4, 40);
    
    midiNoteSlider.setBounds (border,        ColorSlider.getY(), oneRowWidth,       buttonHeight);
    midiNoteLabel.setBounds  (border,        ColorSlider.getY(), twoRowsWidth - 36, textHeight);
    ChannelLabel.setBounds   (xPosRow2 + 36, ColorSlider.getY(), 60,                textHeight);
    ChannelSlider.setBounds  (xPosRow2 + 94, ColorSlider.getY(), twoRowsWidth - 94, textHeight);

    InNoteSlider.setBounds    (border,      ColorSlider.getBottom() + gap,       twoRowsWidth + 50, buttonHeight);
    InInfoLabel.setBounds     (border - 5,           InNoteSlider.getY() - 5,    oneRowWidth,       16);
    OffsetLabel.setBounds     (border + 57,          InNoteSlider.getY() + 14,   70,                buttonHeight);    
    InChannelLabel.setBounds  (ChannelLabel.getX(),  InNoteSlider.getY(),        60,                buttonHeight);
    InChannelSlider.setBounds (ChannelSlider.getX(), InNoteSlider.getY(), ChannelSlider.getWidth(), buttonHeight);
    
    Edit.setBounds          (getWidth() - border - rightButtonW, top,      rightButtonW, buttonHeight);
    ToggleRows.setBounds    (Edit.getX(),       button2.getY(),            rightButtonW, buttonHeight);
    menu.setBounds          (Edit.getX(),       button3.getY(),            rightButtonW, buttonHeight);
    Info.setBounds          (Edit.getX(),       button4.getY(),            rightButtonW, buttonHeight);
    exitColor.setBounds     (Edit.getX(),       button4.getBottom() + 20,  rightButtonW, buttonHeight);
    exitMidi.setBounds      (Edit.getX(),       button4.getBottom() + 20,  rightButtonW, buttonHeight);
    MidiThru.setBounds      (Edit.getX() - 10,  InNoteSlider.getY() + 20,  50,           30);
    MidiThruLabel.setBounds (Edit.getX() - 128, MidiThru.getBottom() - 16, 118,          16);
    
    lockedLabel.setBounds   (Edit.getX() - 56,  top + 2,                   50,           40);
    editLabel.setBounds     (Edit.getX() - 162, button4.getBottom() + 8,   152,          12);
    MidiSendLabel.setBounds (border - 5,        button4.getBottom() + 8,   80,           16);
    messageLabel.setBounds  (border,            getHeight() - 50,          oneRowWidth,  44); // Fixed Y-position
    
    int contentHeight = button4.getBottom() + 40; // final vertical extent
    if (exitColor.isVisible() || exitMidi.isVisible())
        { contentHeight = MidiThru.getBottom() + 120; }
    // Use viewport width (safe if viewport is trimmed or later you change widths)
    scrollContent.setBounds (0, 0, scrollView.getWidth(), contentHeight);
    scrollView.setViewPosition (0, static_cast<int> (lastScrollY)); // ← restore after layout
}

void CallAppAudioProcessorEditor::flashButton (juce::TextButton* button)
{
    if (button == nullptr) return;
    juce::HyperlinkButton* link = nullptr;     // Find the corresponding link for the button
    if      (button == &button1) link = &Link1;
    else if (button == &button2) link = &Link2;
    else if (button == &button3) link = &Link3;
    else if (button == &button4) link = &Link4;
    else if (button == &button5) link = &Link5;
    else if (button == &button6) link = &Link6;
    else if (button == &button7) link = &Link7;
    else if (button == &button8) link = &Link8;

    if (link != nullptr)  // Block (return) if the mouse or finger is actually pressed
    {
        for (const auto& src : juce::Desktop::getInstance().getMouseSources())
        {
            if (src.isDragging() && src.getComponentUnderMouse() == link)
            { return; }   // Actual mouse/touch press – no flash!
        }
    }
    // Visual flash for MIDI or automation events
    button->setState (juce::TextButton::buttonDown);
    juce::Timer::callAfterDelay (100, [this, button]
    { button->setState (juce::TextButton::buttonNormal); } );
}

void CallAppAudioProcessorEditor::updateButtonState (juce::TextButton* button, bool flashState)
{
    if (button != nullptr)
        button->setState (flashState ? juce::TextButton::buttonDown : juce::TextButton::buttonNormal);
}

void CallAppAudioProcessorEditor::selectButtonLabels (int index)
{
    ColorLabel.setText    ("Color for Button " + juce::String (index) + ":", juce::dontSendNotification);
    midiNoteLabel.setText ("Note# for Button " + juce::String (index) + ":", juce::dontSendNotification);
    ColorLabel.setJustificationType    (juce::Justification::right);
    midiNoteLabel.setJustificationType (juce::Justification::right);
}

void CallAppAudioProcessorEditor::updateButtonColorFromSlider()
{
    if (selectedButton != nullptr)
    {
        auto colorValue = static_cast<int> (ColorSlider.getValue());
        juce::String hexValue = juce::String::toHexString (colorValue).paddedLeft ('0', 6).toUpperCase();
        juce::String fullHex = "FF" + hexValue;  // Full opacity
        selectedButton->setColour (juce::TextButton::buttonColourId, juce::Colour::fromString (fullHex));
        ColorSlider.setTextValueSuffix ("" + hexValue);     
    }
    else { showMessage ("No button selected for color change.", juce::Colour (0xFFA100C4)); }
}

void CallAppAudioProcessorEditor::updateSliderFromButton()
{   
    if (selectedButton != nullptr)
    {
        auto color = selectedButton->findColour (juce::TextButton::buttonColourId);
        auto colorValue = color.getARGB() & 0xFFFFFF; // Extract RGB value (without alpha)
        if (colorRange.getToggleState() == true) {    // 118 steps
            // temporarily set the color to "0" to force slider value change
            ColorSlider.setValue (0, juce::sendNotificationSync); }
        updateColorSliderRange(); // if the range is changed by the toggle
        ColorSlider.setValue (static_cast<double> (colorValue), juce::sendNotificationSync);
    }
}

void CallAppAudioProcessorEditor::updateButtonTextColor()
{
    if (selectedButton != nullptr)
    {
        juce::Colour textColor = TextColor.getToggleState() ? juce::Colours::black : juce::Colours::white;
        selectedButton->setColour (juce::TextButton::textColourOffId, textColor);
    }
    else { showMessage ("No button selected for text color change.", juce::Colour (0xFFA100C4)); }
}

void CallAppAudioProcessorEditor::syncTextColorToggle()
{
    if (selectedButton != nullptr)
    {
        bool isBlack = selectedButton->findColour(juce::TextButton::textColourOffId) == juce::Colours::black;
        TextColor.setToggleState (isBlack, juce::dontSendNotification);
    }
}

void CallAppAudioProcessorEditor::updateColorSliderRange()
{   
    const bool toggleState = colorRange.getToggleState(); // true = 118, false = Full (off, default)
    if (toggleState) {
        ColorSlider.setRange (0, 0xFFFFFF, 143395); // 118 steps
        colorRange.setButtonText ("118");
    } else {
        ColorSlider.setRange (0, 0xFFFFFF, 1);
        colorRange.setButtonText ("Full");
    }
}

void CallAppAudioProcessorEditor::updateNoteNumberFromSlider()
{
    if (selectedButton != nullptr)
    {
        int midiNote = static_cast<int> (midiNoteSlider.getValue());
        int midiChannel = selectedButton->getComponentID().fromFirstOccurrenceOf ("_", false, false).getIntValue();
        // Store both note and channel inside the ComponentID
        selectedButton->setComponentID (juce::String (midiNote) + "_" + juce::String (midiChannel));
    }
    else { showMessage ("No button selected for Note Number change.", juce::Colour (0xFFA100C4)); }
}

void CallAppAudioProcessorEditor::updateNoteSliderFromButton()
{
    if (selectedButton != nullptr)
    {
        juce::String storedData = selectedButton->getComponentID();
        if (storedData.contains ("_"))
        {
            int midiNote = storedData.upToFirstOccurrenceOf ("_", false, false).getIntValue();
            if (midiNote >= 0 && midiNote <= 127)
            {
                midiNoteSlider.setValue (midiNote, juce::sendNotificationSync);
                return;
            }
        }
        midiNoteSlider.setValue (60, juce::sendNotificationSync); // Default to C4
    }
}

void CallAppAudioProcessorEditor::updateChannelNumberFromSlider()
{
    if (selectedButton != nullptr)
    {
        int midiChannel = static_cast<int> (ChannelSlider.getValue());
        int midiNote = selectedButton->getComponentID().upToFirstOccurrenceOf ("_", false, false).getIntValue();
        // Store both channel and note inside the ComponentID
        selectedButton->setComponentID (juce::String (midiNote) + "_" + juce::String (midiChannel));
    }
    else { showMessage ("No button selected for Channel change.", juce::Colour (0xFFA100C4)); }
}

void CallAppAudioProcessorEditor::updateChannelSliderFromButton()
{
    if (selectedButton != nullptr)
    {
        juce::String storedData = selectedButton->getComponentID();
        if (storedData.contains ("_"))
        {
            int midiChannel = storedData.fromFirstOccurrenceOf ("_", false, false).getIntValue();
            if (midiChannel >= 0 && midiChannel <= 16)
            {
                ChannelSlider.setValue (midiChannel, juce::sendNotificationSync);
                ChannelSlider.updateText(); // Ensure text is refreshed
                return;
            }
        }
        ChannelSlider.setValue (0, juce::sendNotificationSync); // Default to "Off"
        ChannelSlider.updateText(); // Ensure "Off" is shown
    }
}

void CallAppAudioProcessorEditor::updateNoteNumberFromLink (juce::TextButton* linkedButton)
{
    if (linkedButton == nullptr) return;
    juce::HyperlinkButton* link = nullptr;  // Find the corresponding link for the button
    if      (linkedButton == &button1) link = &Link1;
    else if (linkedButton == &button2) link = &Link2;
    else if (linkedButton == &button3) link = &Link3;
    else if (linkedButton == &button4) link = &Link4;
    else if (linkedButton == &button5) link = &Link5;
    else if (linkedButton == &button6) link = &Link6;
    else if (linkedButton == &button7) link = &Link7;
    else if (linkedButton == &button8) link = &Link8;

    if (link != nullptr)  // Block (return) if the mouse or finger is actually pressed
    {
        for (const auto& src : juce::Desktop::getInstance().getMouseSources())
        {
            if (src.isDragging() && src.getComponentUnderMouse() == link)
            { return; }   // Actual mouse/touch press
        }
    }
	juce::String storedData = linkedButton->getComponentID();
	
	if (storedData.contains ("_"))
	{
		int midiNote = storedData.upToFirstOccurrenceOf ("_", false, false).getIntValue();
		int midiChannel = storedData.fromFirstOccurrenceOf ("_", false, false).getIntValue();

		if (midiChannel > 0)  // Only send if channel is NOT "Off" (0)
		{
			audioProcessor.sendMidiNoteOn (midiNote, 100, midiChannel);
			juce::Timer::callAfterDelay (100, [this, midiNote, midiChannel]()
			{ audioProcessor.sendMidiNoteOn (midiNote, 0, midiChannel); } );
		}
	}
}

void CallAppAudioProcessorEditor::NoteOnOff (juce::TextButton* linkedButton, bool isNoteOn)
{
    if (linkedButton != nullptr)
    {
        juce::String storedData = linkedButton->getComponentID();
        
        if (storedData.contains ("_"))
        {
            int midiNote = storedData.upToFirstOccurrenceOf ("_", false, false).getIntValue();
            int midiChannel = storedData.fromFirstOccurrenceOf ("_", false, false).getIntValue();

            if (midiChannel > 0)  // Only send if channel is NOT "Off" (0)
            {
                int velocity = isNoteOn ? 100 : 0;
                audioProcessor.sendMidiNoteOn (midiNote, velocity, midiChannel);
            }
        }
    }
}

void CallAppAudioProcessorEditor::updateInInfoLabel()
{
    int rootNote = static_cast<int> (InNoteSlider.getValue());
    int lastNote = rootNote + 7;

    auto noteNameFor = [](int midiNote) -> juce::String {
        static const juce::StringArray names = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        int octave = (midiNote / 12) - 1;
        return names[midiNote % 12] + juce::String (octave);
    };

    juce::String rootName = noteNameFor (rootNote);
    juce::String lastName = noteNameFor (lastNote);

    juce::String text = "Receive: Midi Note# " + rootName + "–" + lastName
    	+ " (" + juce::String (rootNote)
    	+ "–"  + juce::String (lastNote) + ")"
    	+ juce::String (juce::CharPointer_UTF8 (" \xe2\x9e\x9e Button 1–8")); // ➞

    InInfoLabel.setText (text, juce::dontSendNotification);
}

void CallAppAudioProcessorEditor::editButtonClicked()
{
    if (!textEditorPopup)
    {
        textEditorPopup = std::make_unique<TextEditorPopup>
        (button1, button2, button3, button4, button5, button6, button7, button8, 
         Link1, Link2, Link3, Link4, Link5, Link6, Link7, Link8);
        addAndMakeVisible (textEditorPopup.get());
         
        textEditorPopup->onColorButtonClick = [this]() {
            ColorButtonClicked();
        };
        textEditorPopup->onNoteButtonClick = [this]() {
            NoteButtonClicked();
        };
        textEditorPopup->onClose = [this]() {
            removeChildComponent (textEditorPopup.get());
            textEditorPopup.reset();
        };
        resized(); // Ensure proper layout
    }
}

void CallAppAudioProcessorEditor::showMessage (const juce::String& message, juce::Colour colour)
{
    messageLabel.setText (message, juce::dontSendNotification);
    messageLabel.setColour (juce::Label::textColourId, colour);
    messageLabel.setVisible (true);
    // Remove message after 3 seconds
    juce::Timer::callAfterDelay (3000, [this]()
    { messageLabel.setVisible (false); } );
}

//==============================================================================
//==============================================================================
void CallAppAudioProcessorEditor::handleIncomingMidiNote (int midiNoteNumber, int midiChannel)
{
    int inChannel = static_cast<int>(InChannelSlider.getValue()); // 0 = Off, 17 = Omni
    if (inChannel == 0) return;
    if (inChannel != 17 && midiChannel != inChannel) return;

    int rootNote = static_cast<int>(InNoteSlider.getValue()); // Offset slider
    int index = midiNoteNumber - rootNote;

    if (index < 0 || index >= 8)
        return; // Outside the triggerable range

    juce::HyperlinkButton* targetLink = nullptr;

    static juce::HyperlinkButton* links[8] = {
        &Link1, &Link2, &Link3, &Link4, &Link5, &Link6, &Link7, &Link8
    };

    targetLink = links[index];

    if (targetLink) // is equal to: if (targetLink != nullptr)
        targetLink->triggerClick();
}

int CallAppAudioProcessorEditor::getInputChannel() const
{ return static_cast<int> (InChannelSlider.getValue()); }

int CallAppAudioProcessorEditor::getInputNoteRoot() const
{ return static_cast<int> (InNoteSlider.getValue()); }

bool CallAppAudioProcessorEditor::shouldBlockNotes() const
{ return MidiThru.getToggleState(); } // Block notes when MidiThru is ON (true)

//==============================================================================
//==============================================================================
void CallAppAudioProcessorEditor::createFileChooser (const juce::String& title,
        const juce::String& filePattern)
{
    juce::File initialDir = getAppGroupDirectory(); // Use the helper function
/*
// This will prevent methods in load/saveSettings, if the directory is not created
    if (!initialDir.exists()) {
        if (!initialDir.createDirectory()) {
            juce::Logger::writeToLog ("Failed to create directory at: " + initialDir.getFullPathName());
            showMessage ("Failed to create directory for file chooser.", juce::Colours::red);
            return; // Exit if we cannot create the directory, no point in proceeding
        }
    }   // Better to delete this and continue here:
*/
    fileChooser = std::make_unique<CustomFileChooser> (title, initialDir, filePattern);
       
    addAndMakeVisible (fileChooser.get());
    fileChooser->setBounds (getLocalBounds());
}

void CallAppAudioProcessorEditor::closeFileChooser()
{
    fileChooser.reset(); // delete FileChooser
}

//==============================================================================
void CallAppAudioProcessorEditor::triggerStandaloneAutoload()
{
    if (! getStandaloneOpenLastUsedFileOnStartup()) return;
    // Safe Mode: if the last startup likely crashed during autoload, skip it once.
    if (getStandaloneAutoloadInProgress()) {
        setStandaloneAutoloadInProgress (false);
        showMessage ("Autoload skipped (previous startup may have failed).", juce::Colours::red);
        return;
    }
    if (! getStandaloneOpenLastUsedFileOnStartup()) return;

    const auto path = getStandaloneLastUsedFilePath();
    if (path.isEmpty()) return;

    const juce::File f (path);
    if (! f.existsAsFile()) return;
    // Mark autoload as in-progress BEFORE actually loading.
    setStandaloneAutoloadInProgress (true);

    juce::MessageManager::callAsync ([safe = SafePointer<CallAppAudioProcessorEditor>(this), f]
    {   if (safe == nullptr) return;
        safe->loadSettings (f, false);  // Tell loadSettings to be quiet on success
        // NOTE: we clear the in-progress flag after loadSettings returns.
        setStandaloneAutoloadInProgress (false);
    });
}
//==============================================================================
//==============================================================================
void CallAppAudioProcessorEditor::openMenu()
{
    createFileChooser ("File Manager", "*.xml");
    
    if (juce::JUCEApplicationBase::isStandaloneApp()) {
        fileChooser->setToggleState (selectDirectoryToggleState);   // iOS standalone
    }                               // if not standalone: Default = false (Off/Plugin)

    juce::String lastPath; // Get the last used file from Standalone-FilePath OR ValueTreeState
	
	if (juce::JUCEApplicationBase::isStandaloneApp()) {
		lastPath = getStandaloneLastUsedFilePath();
	} else {  // if Plugin:
		auto& state = audioProcessor.getValueTreeState();
		lastPath = state.state.getProperty ("lastUsedFile", "").toString();
	}
    if (!lastPath.isEmpty())
    {
        juce::File lastFile (lastPath);
        if (lastFile.existsAsFile())
        {
            fileChooser->setLastUsedFile (lastFile); // Pass the last used file
        }
    }
    fileChooser->onFileSelected = [this](juce::File selectedFile) 
    {
        lastUsedFile = selectedFile; // Update last used file locally
        
        // Store the path in Standalone-FilePath / ValueTreeState for future recall
        if (juce::JUCEApplicationBase::isStandaloneApp()) {
			setStandaloneLastUsedFilePath (selectedFile.getFullPathName());
		} else {
			audioProcessor.getValueTreeState().state.setProperty
			("lastUsedFile", selectedFile.getFullPathName(), nullptr);
		}
        if (fileChooser->isInSaveMode()) // Dynamically check save mode
        {
            saveSettings (selectedFile);
        } else {
            loadSettings (selectedFile, true); // showSuccessMessage = true
        }
        closeFileChooser(); // fileChooser.reset (delete FileChooser)
    };

    fileChooser->onDone = [this]() { closeFileChooser(); };
}
//==============================================================================
//==============================================================================

void CallAppAudioProcessorEditor::saveSettings (const juce::File& saveFile)
{   
    // Get the specific directory based on the platform (iOS or macOS)
    juce::File AppGroupDirectory = getAppGroupDirectory();
    juce::File saveFileInGroup = AppGroupDirectory.getChildFile (saveFile.getFileName());

        // Check if directory exists, create if not
    if (!AppGroupDirectory.exists()) {
        if (!AppGroupDirectory.createDirectory()) {
            juce::Logger::writeToLog ("Failed to create directory at: " + AppGroupDirectory.getFullPathName());
            showMessage ("Failed to create directory.", juce::Colours::red);
            return;
        }
    }   // Check File Existence and Write Permission
    if (saveFileInGroup.exists()) {
        if (!saveFileInGroup.hasWriteAccess()) {
            juce::Logger::writeToLog ("File exists but is not writable: " + saveFileInGroup.getFullPathName());
            showMessage ("Cannot write to the file: \n" + saveFileInGroup.getFullPathName(), juce::Colours::red);
            return;
        }
    } else {   // Check if you can create a new file in this directory
        juce::File testFile = AppGroupDirectory.getChildFile ("testWriteAccess.txt");
        if (testFile.create()) {
            testFile.deleteFile(); // Clean up after test
        } else {
            juce::Logger::writeToLog ("Cannot write to directory: " + AppGroupDirectory.getFullPathName());
            showMessage ("Cannot write to directory: \n" + AppGroupDirectory.getFullPathName(), juce::Colours::red);
            return;
        }
    }   // Attempt to write to the file
    if (!saveFileInGroup.replaceWithText ("Test content")) {
        juce::Logger::writeToLog ("Failed to write to file: " + saveFileInGroup.getFullPathName());
        showMessage ("Failed to write to file: \n" + saveFileInGroup.getFullPathName(), juce::Colours::red);
    } else { showMessage ("File saved successfully @\n" + saveFileInGroup.getFullPathName(), juce::Colours::green); }
    
    saveStateToValueTree(); // Update the ValueTree before saving
    
    // Create an XML representation of the ValueTree 'parameters'
    if (auto xml = audioProcessor.createParametersXml()) 
    {
        if (!saveFile.replaceWithText (xml->toString()))
             { showMessage ("Failed to save settings.", juce::Colours::red); }
        else { showMessage ("Settings saved successfully.", juce::Colours::blue); }
    } else { showMessage ("Failed to serialize settings.", juce::Colours::red); }
}

void CallAppAudioProcessorEditor::loadSettings (const juce::File& loadFile, bool showSuccessMessage)
{
    if (!loadFile.existsAsFile()) {
        showMessage ("Error: Selected file does not exist.", juce::Colours::red);
        return;  // Prevent crash if file doesn't exist
    }
    // Parse the XML from the file
    auto xml = juce::parseXML (loadFile);
    if (xml == nullptr) {
        showMessage ("Failed to parse XML from file.", juce::Colours::red);
        return;
    }
    // Convert the XML to a ValueTree 'parameters'
    juce::ValueTree newState = juce::ValueTree::fromXml (*xml);
    if (!newState.isValid()) {
        showMessage ("Invalid preset file.", juce::Colours::red);
        return;
    }
    // Apply the loaded state to the processor's tree state ('parameters')
    audioProcessor.setParametersValueTree (newState);
    
    // Synchronize UI with the loaded state
    loadStateFromValueTree (audioProcessor.getParametersValueTree());
    if (showSuccessMessage)  // showSuccessMessage = true
        showMessage ("Settings loaded successfully.", juce::Colours::blue);
}

void CallAppAudioProcessorEditor::saveStateToValueTree()
{
    auto& state = audioProcessor.getValueTreeState().state;

    // Save button and URL states to the ValueTree
    state.setProperty ("button1Text", button1.getButtonText(), nullptr);
    state.setProperty ("button2Text", button2.getButtonText(), nullptr);
    state.setProperty ("button3Text", button3.getButtonText(), nullptr);
    state.setProperty ("button4Text", button4.getButtonText(), nullptr);
    state.setProperty ("button5Text", button5.getButtonText(), nullptr);
    state.setProperty ("button6Text", button6.getButtonText(), nullptr);
    state.setProperty ("button7Text", button7.getButtonText(), nullptr);
    state.setProperty ("button8Text", button8.getButtonText(), nullptr);
    state.setProperty ("link1URL", Link1.getURL().toString (true), nullptr);
    state.setProperty ("link2URL", Link2.getURL().toString (true), nullptr);
    state.setProperty ("link3URL", Link3.getURL().toString (true), nullptr);
    state.setProperty ("link4URL", Link4.getURL().toString (true), nullptr);
    state.setProperty ("link5URL", Link5.getURL().toString (true), nullptr);
    state.setProperty ("link6URL", Link6.getURL().toString (true), nullptr);
    state.setProperty ("link7URL", Link7.getURL().toString (true), nullptr);
    state.setProperty ("link8URL", Link8.getURL().toString (true), nullptr);
    state.setProperty ("Color1", button1.findColour (juce::TextButton::buttonColourId).toString(), nullptr);
    state.setProperty ("Color2", button2.findColour (juce::TextButton::buttonColourId).toString(), nullptr);
    state.setProperty ("Color3", button3.findColour (juce::TextButton::buttonColourId).toString(), nullptr);
    state.setProperty ("Color4", button4.findColour (juce::TextButton::buttonColourId).toString(), nullptr);
    state.setProperty ("Color5", button5.findColour (juce::TextButton::buttonColourId).toString(), nullptr);
    state.setProperty ("Color6", button6.findColour (juce::TextButton::buttonColourId).toString(), nullptr);
    state.setProperty ("Color7", button7.findColour (juce::TextButton::buttonColourId).toString(), nullptr);
    state.setProperty ("Color8", button8.findColour (juce::TextButton::buttonColourId).toString(), nullptr);
    state.setProperty ("TextColor1", button1.findColour (juce::TextButton::textColourOffId).toString(), nullptr);
	state.setProperty ("TextColor2", button2.findColour (juce::TextButton::textColourOffId).toString(), nullptr);
	state.setProperty ("TextColor3", button3.findColour (juce::TextButton::textColourOffId).toString(), nullptr);
	state.setProperty ("TextColor4", button4.findColour (juce::TextButton::textColourOffId).toString(), nullptr);
	state.setProperty ("TextColor5", button5.findColour (juce::TextButton::textColourOffId).toString(), nullptr);
	state.setProperty ("TextColor6", button6.findColour (juce::TextButton::textColourOffId).toString(), nullptr);
	state.setProperty ("TextColor7", button7.findColour (juce::TextButton::textColourOffId).toString(), nullptr);
	state.setProperty ("TextColor8", button8.findColour (juce::TextButton::textColourOffId).toString(), nullptr);
	state.setProperty ("button1Data", button1.getComponentID(), nullptr);
    state.setProperty ("button2Data", button2.getComponentID(), nullptr);
    state.setProperty ("button3Data", button3.getComponentID(), nullptr);
    state.setProperty ("button4Data", button4.getComponentID(), nullptr);
    state.setProperty ("button5Data", button5.getComponentID(), nullptr);
    state.setProperty ("button6Data", button6.getComponentID(), nullptr);
    state.setProperty ("button7Data", button7.getComponentID(), nullptr);
    state.setProperty ("button8Data", button8.getComponentID(), nullptr);
    state.setProperty ("InNote",    InNoteSlider.getValue(),    nullptr);
	state.setProperty ("InChannel", InChannelSlider.getValue(), nullptr);
	state.setProperty ("MidiThru",  MidiThru.getToggleState(),  nullptr);
	
    state.setProperty ("ShowRows", ToggleRows.getToggleState(), nullptr);
    state.setProperty ("ColorRangeToggle", false, nullptr);  // Always save as "Full" (off)
    state.setProperty ("lastUsedFile", lastUsedFile.getFullPathName(), nullptr);
}

void CallAppAudioProcessorEditor::loadStateFromValueTree (const juce::ValueTree& state)
{  
    colorRange.setToggleState (false, juce::sendNotificationSync);  // Always reset to default (Full)
    button1.setButtonText (state.getProperty ("button1Text", "...").toString());
    button2.setButtonText (state.getProperty ("button2Text", "...").toString());
    button3.setButtonText (state.getProperty ("button3Text", "...").toString());
    button4.setButtonText (state.getProperty ("button4Text", "...").toString());
    button5.setButtonText (state.getProperty ("button5Text", "...").toString());
    button6.setButtonText (state.getProperty ("button6Text", "...").toString());
    button7.setButtonText (state.getProperty ("button7Text", "...").toString());
    button8.setButtonText (state.getProperty ("button8Text", "...").toString());
    Link1.setURL (juce::URL::createWithoutParsing (state.getProperty ("link1URL", "").toString()));
    Link2.setURL (juce::URL::createWithoutParsing (state.getProperty ("link2URL", "").toString()));
    Link3.setURL (juce::URL::createWithoutParsing (state.getProperty ("link3URL", "").toString()));
    Link4.setURL (juce::URL::createWithoutParsing (state.getProperty ("link4URL", "").toString()));
    Link5.setURL (juce::URL::createWithoutParsing (state.getProperty ("link5URL", "").toString()));
    Link6.setURL (juce::URL::createWithoutParsing (state.getProperty ("link6URL", "").toString()));
    Link7.setURL (juce::URL::createWithoutParsing (state.getProperty ("link7URL", "").toString()));
    Link8.setURL (juce::URL::createWithoutParsing (state.getProperty ("link8URL", "").toString()));
    button1.setColour(juce::TextButton::buttonColourId, juce::Colour::fromString(state.getProperty("Color1").toString()));
    button2.setColour(juce::TextButton::buttonColourId, juce::Colour::fromString(state.getProperty("Color2").toString()));
    button3.setColour(juce::TextButton::buttonColourId, juce::Colour::fromString(state.getProperty("Color3").toString()));
    button4.setColour(juce::TextButton::buttonColourId, juce::Colour::fromString(state.getProperty("Color4").toString()));
    button5.setColour(juce::TextButton::buttonColourId, juce::Colour::fromString(state.getProperty("Color5").toString()));
    button6.setColour(juce::TextButton::buttonColourId, juce::Colour::fromString(state.getProperty("Color6").toString()));
    button7.setColour(juce::TextButton::buttonColourId, juce::Colour::fromString(state.getProperty("Color7").toString()));
    button8.setColour(juce::TextButton::buttonColourId, juce::Colour::fromString(state.getProperty("Color8").toString()));
	button1.setColour(juce::TextButton::textColourOffId, juce::Colour::fromString(state.getProperty("TextColor1", "white").toString()));
	button2.setColour(juce::TextButton::textColourOffId, juce::Colour::fromString(state.getProperty("TextColor2", "white").toString()));
	button3.setColour(juce::TextButton::textColourOffId, juce::Colour::fromString(state.getProperty("TextColor3", "white").toString()));
	button4.setColour(juce::TextButton::textColourOffId, juce::Colour::fromString(state.getProperty("TextColor4", "white").toString()));
	button5.setColour(juce::TextButton::textColourOffId, juce::Colour::fromString(state.getProperty("TextColor5", "white").toString()));
	button6.setColour(juce::TextButton::textColourOffId, juce::Colour::fromString(state.getProperty("TextColor6", "white").toString()));
	button7.setColour(juce::TextButton::textColourOffId, juce::Colour::fromString(state.getProperty("TextColor7", "white").toString()));
	button8.setColour(juce::TextButton::textColourOffId, juce::Colour::fromString(state.getProperty("TextColor8", "white").toString()));
	button1.setComponentID (state.getProperty ("button1Data", "120_0").toString()); // Default, Ch Off
    button2.setComponentID (state.getProperty ("button2Data", "121_0").toString());
    button3.setComponentID (state.getProperty ("button3Data", "122_0").toString());
    button4.setComponentID (state.getProperty ("button4Data", "123_0").toString());
    button5.setComponentID (state.getProperty ("button5Data", "124_0").toString());
    button6.setComponentID (state.getProperty ("button6Data", "125_0").toString());
    button7.setComponentID (state.getProperty ("button7Data", "126_0").toString());
    button8.setComponentID (state.getProperty ("button8Data", "127_0").toString());
    InNoteSlider.setValue    (state.getProperty ("InNote", 1),    juce::sendNotificationSync);
	InChannelSlider.setValue (state.getProperty ("InChannel", 0), juce::sendNotificationSync);
	bool thruState = static_cast<bool> (state.getProperty ("MidiThru", false));
	MidiThru.setToggleState (thruState, juce::sendNotificationSync);
    MidiThru.setButtonText  (thruState ? "Block" : "Pass");
	
    if (textEditorPopup) {
		textEditorPopup->Name1.setText (state.getProperty ("button1Text", "...").toString());
		textEditorPopup->Name2.setText (state.getProperty ("button2Text", "...").toString());
		textEditorPopup->Name3.setText (state.getProperty ("button3Text", "...").toString());
		textEditorPopup->Name4.setText (state.getProperty ("button4Text", "...").toString());
		textEditorPopup->Name5.setText (state.getProperty ("button5Text", "...").toString());
		textEditorPopup->Name6.setText (state.getProperty ("button6Text", "...").toString());
		textEditorPopup->Name7.setText (state.getProperty ("button7Text", "...").toString());
		textEditorPopup->Name8.setText (state.getProperty ("button8Text", "...").toString());
		textEditorPopup->text1.setText (state.getProperty ("link1URL", "").toString());
		textEditorPopup->text2.setText (state.getProperty ("link2URL", "").toString());
		textEditorPopup->text3.setText (state.getProperty ("link3URL", "").toString());
		textEditorPopup->text4.setText (state.getProperty ("link4URL", "").toString());
		textEditorPopup->text5.setText (state.getProperty ("link5URL", "").toString());
		textEditorPopup->text6.setText (state.getProperty ("link6URL", "").toString());
		textEditorPopup->text7.setText (state.getProperty ("link7URL", "").toString());
		textEditorPopup->text8.setText (state.getProperty ("link8URL", "").toString());
	}
	
	updateSliderFromButton(); // update slider, if a button is selected
  	syncTextColorToggle(); // update ToggleButton, if a button is selected
  	updateNoteSliderFromButton();
  	updateChannelSliderFromButton();
    
    ToggleRows.setToggleState (state.getProperty ("ShowRows", false), juce::dontSendNotification);
    rowsCount();
    resized();

    // Auto-select last used file if the chooser is open
    juce::String lastPath = state.getProperty ("lastUsedFile", "").toString();
    if (fileChooser && !lastPath.isEmpty())
    {
        juce::File lastFile (lastPath);
        if (lastFile.existsAsFile())
        {
            fileChooser->setLastUsedFile (lastFile);  // Update file list while open
        }
    }
}

void TextEditorPopup::mouseEnter (const juce::MouseEvent& e)
{
    if (auto* editor = dynamic_cast<juce::TextEditor*> (e.eventComponent))
    {
        expandedEditor = editor;  // Expand editor to full width
        lastScrollY = scrollView.getVerticalScrollBar().getCurrentRangeStart();
        resized();
    }
}

void CallAppAudioProcessorEditor::mouseEnter (const juce::MouseEvent& e)
{
    if (auto* editor = dynamic_cast<juce::TextEditor*> (e.eventComponent))
    { colorRange.setToggleState (false, juce::sendNotificationSync); }  // Color range = Full RGB
}

void CallAppAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    if      (e.eventComponent == &Link1) { updateButtonState (&button1, true); NoteOnOff (&button1, true); }
    else if (e.eventComponent == &Link2) { updateButtonState (&button2, true); NoteOnOff (&button2, true); }
    else if (e.eventComponent == &Link3) { updateButtonState (&button3, true); NoteOnOff (&button3, true); }
    else if (e.eventComponent == &Link4) { updateButtonState (&button4, true); NoteOnOff (&button4, true); }
    else if (e.eventComponent == &Link5) { updateButtonState (&button5, true); NoteOnOff (&button5, true); }
    else if (e.eventComponent == &Link6) { updateButtonState (&button6, true); NoteOnOff (&button6, true); }
    else if (e.eventComponent == &Link7) { updateButtonState (&button7, true); NoteOnOff (&button7, true); }
    else if (e.eventComponent == &Link8) { updateButtonState (&button8, true); NoteOnOff (&button8, true); }
}

void CallAppAudioProcessorEditor::mouseUp (const juce::MouseEvent& e)
{
    if      (e.eventComponent == &Link1) { updateButtonState (&button1, false); NoteOnOff (&button1, false); }
    else if (e.eventComponent == &Link2) { updateButtonState (&button2, false); NoteOnOff (&button2, false); }
    else if (e.eventComponent == &Link3) { updateButtonState (&button3, false); NoteOnOff (&button3, false); }
    else if (e.eventComponent == &Link4) { updateButtonState (&button4, false); NoteOnOff (&button4, false); }
    else if (e.eventComponent == &Link5) { updateButtonState (&button5, false); NoteOnOff (&button5, false); }
    else if (e.eventComponent == &Link6) { updateButtonState (&button6, false); NoteOnOff (&button6, false); }
    else if (e.eventComponent == &Link7) { updateButtonState (&button7, false); NoteOnOff (&button7, false); }
    else if (e.eventComponent == &Link8) { updateButtonState (&button8, false); NoteOnOff (&button8, false); }
#if JUCE_IOS
    if (auto* textEditor = dynamic_cast<juce::TextEditor*> (e.eventComponent))
    {   
        int clicks = e.getNumberOfClicks(); 
        if (clicks > 1) 
        {
            juce::Timer::callAfterDelay (10, [this, textEditor]
             { showIOSContextMenu (*textEditor); });
        }
        else { showIOSContextMenu (*textEditor); }
        textEditor->onTextChange = []() { hideIOSMenuNative(); };
    }
#endif
}

#if JUCE_IOS
void TextEditorPopup::mouseUp (const juce::MouseEvent& e)
{
    if (auto* textEditor = dynamic_cast<juce::TextEditor*> (e.eventComponent))
    {
        int clicks = e.getNumberOfClicks();
        if (clicks > 1) 
        {
            juce::Timer::callAfterDelay (10, [this, textEditor]
             { showIOSContextMenu (*textEditor); });
        }
        else { showIOSContextMenu (*textEditor); }
    }
}

void TextEditorPopup::showIOSContextMenu (juce::TextEditor& editor)
{
    int selectionLength = editor.getHighlightedRegion().getLength();
    int currentCursorPosition = editor.getCaretPosition();

    bool hasSelection = selectionLength > 0;
    bool cursorUnchanged = (currentCursorPosition == lastCursorPosition);

    if (hasSelection || cursorUnchanged)
    {
        showIOSMenuNative (editor);
    }
    lastCursorPosition = currentCursorPosition; // Updates the stored position
}

void CallAppAudioProcessorEditor::showIOSContextMenu (juce::TextEditor& editor)
{
    int selectionLength = editor.getHighlightedRegion().getLength();
    int currentCursorPosition = editor.getCaretPosition();

    bool hasSelection = selectionLength > 0;
    bool cursorUnchanged = (currentCursorPosition == lastCursorPosition);

    if (hasSelection || cursorUnchanged)
    {
        showIOSMenuNative (editor);
    }
    lastCursorPosition = currentCursorPosition; // Updates the stored position
}
#endif

