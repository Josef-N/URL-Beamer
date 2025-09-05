
//                 PluginEditor.h    –   JUCE plugin editor

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CustomFileChooser.h"
//==============================================================================
class TextEditorPopup : public juce::Component
{
public:
    TextEditorPopup
    (juce::TextButton& button1, juce::TextButton& button2, juce::TextButton& button3, 
    juce::TextButton& button4, juce::TextButton& button5, juce::TextButton& button6, 
    juce::TextButton& button7, juce::TextButton& button8,
    juce::HyperlinkButton& Link1, juce::HyperlinkButton& Link2, juce::HyperlinkButton& Link3, 
    juce::HyperlinkButton& Link4, juce::HyperlinkButton& Link5, juce::HyperlinkButton& Link6, 
    juce::HyperlinkButton& Link7, juce::HyperlinkButton& Link8);
    ~TextEditorPopup() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    juce::Viewport scrollView;
	juce::Component contentArea;
    
    juce::TextEditor* expandedEditor = nullptr;
    
    juce::TextEditor Name1, Name2, Name3, Name4, Name5, Name6, Name7, Name8,
                     text1, text2, text3, text4, text5, text6, text7, text8;
    juce::TextButton OK, Cancel, Color, Note, List;
    juce::Label Commit, CancelText, ListLabel;
    
    std::function<void()> onClose;
    std::function<void()> onColorButtonClick;
    std::function<void()> onNoteButtonClick;
    
    std::array<juce::TextEditor*, 16> allEditors;
    
    void mouseEnter (const juce::MouseEvent& e) override;
#if JUCE_IOS
    void showIOSContextMenu (juce::TextEditor& editor);
    void mouseUp (const juce::MouseEvent& event) override;
#endif

private:
    juce::TextButton& button1Ref; juce::TextButton& button2Ref; juce::TextButton& button3Ref; 
    juce::TextButton& button4Ref; juce::TextButton& button5Ref; juce::TextButton& button6Ref;
    juce::TextButton& button7Ref; juce::TextButton& button8Ref;
    
    juce::HyperlinkButton& Link1Ref; juce::HyperlinkButton& Link2Ref; juce::HyperlinkButton& Link3Ref;
    juce::HyperlinkButton& Link4Ref; juce::HyperlinkButton& Link5Ref; juce::HyperlinkButton& Link6Ref;
    juce::HyperlinkButton& Link7Ref; juce::HyperlinkButton& Link8Ref;
    
    double lastScrollY = 0.0;

#if JUCE_IOS
    int lastCursorPosition = -1; // Saves the last click position (-1 as default value)
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextEditorPopup)
};

//==============================================================================
class CallAppAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    CallAppAudioProcessorEditor (CallAppAudioProcessor&);
    ~CallAppAudioProcessorEditor() override;

    //==============================================================================
    void showAlertWindow();  // for Standalone only
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void rowsCount()
    {
        bool toggleState = ToggleRows.getToggleState(); // toggle state: on/off
        
        // Sync state to ValueTree immediately
        audioProcessor.getValueTreeState().state.setProperty ("ShowRows", toggleState, nullptr);
        
        if (toggleState == true) // ON (= 8 buttons)
        {
            buttonRows = 2;
            ToggleRows.setButtonText (juce::CharPointer_UTF8 ("\u2B95\n4")); // ➡ ︎\u2B95 \n new line
            button5.setVisible (true);  // ("show  4  ")
            button6.setVisible (true);  // (➡ u2B95)
            button7.setVisible (true);  // (← u2190)
            button8.setVisible (true);  // (⇐ u21D0 Leftwards Double Arrow)
            Link5.setVisible (true);    // (⬅️ u2B05 LEFTWARDS BLACK ARROW)
            Link6.setVisible (true);
            Link7.setVisible (true);
            Link8.setVisible (true);
        }
        else // OFF (= 4 buttons)
        {
            buttonRows = 1;
            ToggleRows.setButtonText (juce::CharPointer_UTF8 ("\u2B95\n8")); // ➡ ︎\u2B95
            button5.setVisible (false); // ("show  8  ")
            button6.setVisible (false);
            button7.setVisible (false);
            button8.setVisible (false);
            Link5.setVisible (false);
            Link6.setVisible (false);
            Link7.setVisible (false);
            Link8.setVisible (false);
        }
    }

    void ColorButtonClicked()
    {
        ColorSlider.setVisible (true);
        ColorLabel.setVisible (true);
        TextColor.setVisible (true);
        DummyLabel.setVisible (true);
        colorRange.setVisible (true);
        RangeLabel.setVisible (true);
        midiNoteSlider.setVisible (false);
        midiNoteLabel.setVisible (false);
        ChannelSlider.setVisible (false);
        ChannelLabel.setVisible (false);
        OffsetLabel.setVisible (false);
        InNoteSlider.setVisible (false);
        InChannelLabel.setVisible (false);
        InChannelSlider.setVisible (false);
        InInfoLabel.setVisible (false);
        MidiSendLabel.setVisible (false);
        MidiThru.setVisible (false);
        MidiThruLabel.setVisible (false);
        editLabel.setVisible (true);
        Link1.toBack(); Link2.toBack(); Link3.toBack(); Link4.toBack(); 
        Link5.toBack(); Link6.toBack(); Link7.toBack(); Link8.toBack();
        exitColor.setVisible (true);
        exitMidi.setVisible (false);
        lockedLabel.setText (juce::CharPointer_UTF8 ("\u26A0\ncolor"), juce::dontSendNotification);
        lockedLabel.setVisible (true);             // ⚠ 26A0 or ⛔ 26D4 
        resized(); // force scrollContent height update
    }
    void NoteButtonClicked()
    {
        ColorSlider.setVisible (false);
        ColorLabel.setVisible (false);
        TextColor.setVisible (false);
        DummyLabel.setVisible (false);
        colorRange.setVisible (false);
        RangeLabel.setVisible (false);
        midiNoteSlider.setVisible (true);
        midiNoteLabel.setVisible (true);
        ChannelSlider.setVisible (true);
        ChannelLabel.setVisible (true);
        OffsetLabel.setVisible (true);
        InNoteSlider.setVisible (true);
        InChannelLabel.setVisible (true);
        InChannelSlider.setVisible (true);
        InInfoLabel.setVisible (true);
        MidiSendLabel.setVisible (true);
        MidiThru.setVisible (true);
        MidiThruLabel.setVisible (true);
        editLabel.setVisible (true);
        Link1.toBack(); Link2.toBack(); Link3.toBack(); Link4.toBack();
        Link5.toBack(); Link6.toBack(); Link7.toBack(); Link8.toBack();
        exitColor.setVisible (false);
        exitMidi.setVisible (true);
        lockedLabel.setText (juce::CharPointer_UTF8 ("\u26A0\nmidi"), juce::dontSendNotification);
        lockedLabel.setVisible (true);
        colorRange.setToggleState (false, juce::sendNotificationSync);
        resized(); // force scrollContent height update
    }
    
    void exitColorButtonClicked()
    {
        ColorSlider.setVisible (false);
        ColorLabel.setVisible (false);
        TextColor.setVisible (false);
        DummyLabel.setVisible (false);
        colorRange.setVisible (false);
        RangeLabel.setVisible (false);
        editLabel.setVisible (false);              // toFront (false): shouldAlsoGainKeyboardFocus
        Link1.toFront (false); Link2.toFront (false); Link3.toFront (false); Link4.toFront (false);
        Link5.toFront (false); Link6.toFront (false); Link7.toFront (false); Link8.toFront (false);
        exitColor.setVisible (false);
        lockedLabel.setVisible (false);
        colorRange.setToggleState (false, juce::sendNotificationSync);
        resized(); // force scrollContent height update
    }
    void exitMidiButtonClicked()
    {
        midiNoteSlider.setVisible (false);
        midiNoteLabel.setVisible (false);
        ChannelSlider.setVisible (false);
        ChannelLabel.setVisible (false);
        OffsetLabel.setVisible (false);
        InNoteSlider.setVisible (false);
        InChannelLabel.setVisible (false);
        InChannelSlider.setVisible (false);
        InInfoLabel.setVisible (false);
        MidiSendLabel.setVisible (false);
        MidiThru.setVisible (false);
        MidiThruLabel.setVisible (false);
        editLabel.setVisible (false);
        Link1.toFront (false); Link2.toFront (false); Link3.toFront (false); Link4.toFront (false);
        Link5.toFront (false); Link6.toFront (false); Link7.toFront (false); Link8.toFront (false);
        exitMidi.setVisible (false);
        lockedLabel.setVisible (false);
        resized(); // force scrollContent height update
    }

    void triggerStateSave() { saveStateToValueTree(); }  // Public Save wrapper
    void triggerStateLoad() 
        { loadStateFromValueTree (audioProcessor.getValueTreeState().state); }  // Load wrapper

    void handleIncomingMidiNote (int midiNoteNumber, int midiChannel);
    int getInputChannel() const;   // InChannelSlider wrapper (getter)
	int getInputNoteRoot() const;  // InNoteSlider wrapper
	bool shouldBlockNotes() const; // MidiThru getter
    
    CustomFileChooser* getFileChooser() { return fileChooser.get(); }

    void mouseEnter (const juce::MouseEvent& e) override;
    void mouseDown  (const juce::MouseEvent& event) override;
    void mouseUp (const juce::MouseEvent& event) override;
#if JUCE_IOS
    void showIOSContextMenu (juce::TextEditor& editor);
#endif
    
private:
    CallAppAudioProcessor& audioProcessor;
    int buttonWidth;
    int buttonRows;
    
    juce::Viewport scrollView;
    juce::Component scrollContent;
    
    // Components
    juce::HyperlinkButton Link1, Link2, Link3, Link4, Link5, Link6, Link7, Link8;
    juce::TextButton button1, button2, button3, button4, button5, button6, button7, button8,
                     Edit, ToggleRows, menu, Info, exitColor, exitMidi, colorRange, MidiThru;
                     
    juce::Slider ColorSlider, midiNoteSlider, ChannelSlider, InNoteSlider, InChannelSlider;
    juce::Label messageLabel, ColorLabel, DummyLabel, midiNoteLabel, ChannelLabel, OffsetLabel,
                InInfoLabel, InChannelLabel, RangeLabel, lockedLabel, editLabel, MidiSendLabel, MidiThruLabel; 
    juce::ToggleButton TextColor;
    
    juce::TextButton* selectedButton = nullptr;  // Reference to the currently selected button
    
    juce::File lastUsedFile; // Tracks the last file used (saved or loaded)
    
    // Helper methods                 
    void editButtonClicked();
    void saveSettings (const juce::File& saveFile);
    void loadSettings (const juce::File& loadFile);
    void openMenu();
    void createFileChooser (const juce::String& title, const juce::String& filePattern);
    void closeFileChooser();
    void showMessage (const juce::String& message, juce::Colour colour = juce::Colours::red);
    
    void updateButtonState (juce::TextButton* button, bool flashState);
    void flashButton (juce::TextButton* button); // Visual click for button 1–8
    void selectButtonLabels (int index); // Lambda for ColorLabel + midiNoteLabel-Text
    
    void updateButtonColorFromSlider();  // Process input via the slider text field
    void updateSliderFromButton();       // Set slider to the button color
    void updateButtonTextColor();
    void syncTextColorToggle();          // syncTextColorToggle With Selected Button
    void updateNoteNumberFromSlider();
    void updateNoteSliderFromButton();
    void updateNoteNumberFromLink (juce::TextButton* linkedButton);
    void NoteOnOff (juce::TextButton* linkedButton, bool isNoteOn);
    void updateChannelNumberFromSlider();
    void updateChannelSliderFromButton();
    void updateColorSliderRange();
    void updateInInfoLabel();
    
    void saveStateToValueTree();
    void loadStateFromValueTree (const juce::ValueTree& state);
    
    std::unique_ptr<TextEditorPopup> textEditorPopup;
    std::unique_ptr<CustomFileChooser> fileChooser;
    
    std::unique_ptr <juce::AudioProcessorValueTreeState::ButtonAttachment> link1Value,
    link2Value, link3Value, link4Value, link5Value, link6Value, link7Value, link8Value;
    
    double lastScrollY = 0.0;
    
#if JUCE_IOS
    int lastCursorPosition = -1; // Saves the last click position (-1 as default value)
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CallAppAudioProcessorEditor)
};
