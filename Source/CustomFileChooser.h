
//                 CustomFileChooser.h    –   JUCE plugin editor

#pragma once

#include <JuceHeader.h>
#include "AppUtilities.h" // for AppGroup
#include <algorithm> // for sorting filenames
//==============================================================================
class CustomFileChooser : public juce::Component, public juce::ListBoxModel
{
public:
    CustomFileChooser (const juce::String& title, const juce::File& initialDirectory, 
                       const juce::String& filePattern)
        : directory (initialDirectory), pattern (filePattern)
    {  
        setOpaque (true);
        
        if (juce::JUCEApplicationBase::isStandaloneApp()) {
            titleLabel.setText ("Select Directory: Plugin (Shared) / Documents (Standalone only)", juce::dontSendNotification);
            titleLabel.setJustificationType (juce::Justification::topLeft);
        } else {
        titleLabel.setText (title, juce::dontSendNotification);
        titleLabel.setJustificationType (juce::Justification::centredTop); }
        addAndMakeVisible (titleLabel);

        if (juce::JUCEApplicationBase::isStandaloneApp()) {
            selectDirectory.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff122566));
            selectDirectory.setClickingTogglesState (true); // automatically flip the toggle state
            // Restore button state and label from global toggle state
            selectDirectory.setToggleState (selectDirectoryToggleState, juce::dontSendNotification);
            selectDirectory.setButtonText (selectDirectoryToggleState ? "Documents" : "Plugin");
            selectDirectory.onClick = [this]() {
				selectDirectoryToggleState = selectDirectory.getToggleState();  // Update global state
				setStandaloneSelectDirectoryToggleState (selectDirectoryToggleState);  // for Autoload
				selectDirectory.setButtonText (selectDirectoryToggleState ? "Documents" : "Plugin");
				// Refresh the directory shown in the file chooser
  				directory = getAppGroupDirectory();  // This function will use the updated global state
  				fileList.deselectAllRows();  // Clear any current selection
  				refreshFileList();  // Refresh file list based on new directory
			};
			addAndMakeVisible (selectDirectory);
        }
        
		fileNameEditor.setTextToShowWhenEmpty ("Selected File ...", juce::Colour (0xFF0096FF));
  		fileNameEditor.setInterceptsMouseClicks (false, false);  // Block mouse clicks (component, children)
        fileNameEditor.setReadOnly (true);
  		fileNameEditor.setSelectAllWhenFocused (false);
        fileNameEditor.setMultiLine (true);
        fileNameEditor.setReturnKeyStartsNewLine (false);
        fileNameEditor.setJustification (juce::Justification::centred);
        fileNameEditor.setFont (juce::Font (juce::FontOptions().withPointHeight(18.0f)));
        fileNameEditor.onReturnKey = [this]() { handleFileNameInput(); };
		fileNameEditor.addMouseListener (this, false);  // Register mouseUp as MouseListener
		fileNameEditor.onTextChange = [] { hideIOSMenuNative(); };
		addAndMakeVisible (fileNameEditor);

        Save.setButtonText ("Save");
        Save.setColour (juce::TextButton::buttonColourId, juce::Colour (0xFFA100C4));
        Save.onClick = [this]() { handleFileNameInput(); };  // Call for save
        addChildComponent (Save);
        
        Cancel.setButtonText ("Cancel");
        Cancel.onClick = [this]() { exitSaveMode(); };
        addChildComponent (Cancel);
        
        Delete.setButtonText ("Delete");
        Delete.onClick = [this]() { deleteSelectedFile(); };
        Delete.setColour (juce::TextButton::buttonColourId, juce::Colour (0xFFA100C4));
        addChildComponent (Delete);
        
        CancelDelete.setButtonText ("Cancel");
        CancelDelete.onClick = [this]() { exitDeleteMode(); };
        addChildComponent (CancelDelete);
        
        fileList.setModel (this);
        fileList.setRowHeight (35);
        fileList.setColour (juce::ListBox::backgroundColourId, juce::Colour (0xFF8093A6));
        addAndMakeVisible (fileList);
        
        saveButton.setButtonText ("Save As");
        saveButton.onClick = [this]() { enterSaveMode(); };
        addAndMakeVisible (saveButton);
        
        loadButton.setButtonText ("Load");
        loadButton.setColour (juce::TextButton::textColourOffId, juce::Colour (0xFF0096FF));
        loadButton.onClick = [this]() {
            exitSaveMode();
            if (onFileSelected) onFileSelected (selectedFile); }; // Call for load
        addAndMakeVisible (loadButton);
        
        deleteButton.setButtonText ("Delete");
        deleteButton.onClick = [this]() { enterDeleteMode(); };
        addAndMakeVisible (deleteButton);

        doneButton.setButtonText ("Done");
        doneButton.onClick = [this]() { if (onDone) onDone(); };
        addAndMakeVisible (doneButton);
        
        refreshFileList(); // Populate file list initially
    }
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    }
    
	void resized() override
	{
		const bool isStandalone = juce::JUCEApplicationBase::isStandaloneApp();
		const bool portrait = getHeight() > getWidth();
	
		auto titleTop = 8;   // Default for Plugin
		auto border = 8;
		auto bottomMargin = 8;
	
		constexpr int titleHeight = 22;
		constexpr int spacing = 2;
		constexpr int buttonHeight = 40;
		constexpr int rightButtonWidth = 60;
		constexpr int extraBottomPadding = 14;
	
		const auto* display = juce::Desktop::getInstance().getDisplays()
								  .getDisplayForRect (getScreenBounds());
		// Standalone:
		if (isStandalone) {
			if (display != nullptr) {
				const auto safeInsets = display->safeAreaInsets;
				bottomMargin = safeInsets.getBottom() + extraBottomPadding;
				// iPad
				if (isRunningOnIPad()) {
					// Align selectDirectory with button1 in the main editor.
					const auto protectedTop = juce::jmax (76, safeInsets.getTop());
					const auto selectDirectoryTop = protectedTop + 6;
					titleTop = selectDirectoryTop - titleHeight;
					border = portrait ? 8 : 64;  // Portrait : Landscape
				// iPhone
				} else if (portrait) {    // Portrait
					// Align titleLabel with button1 in the main editor.
					titleTop = safeInsets.getTop() + 8;
					border = 8;
				} else {                  // Landscape
					// Keep the existing top position in iPhone landscape.
					const auto horizontalSafeInset = juce::jmax (safeInsets.getLeft(), safeInsets.getRight());
					titleTop = 8;
					border = horizontalSafeInset + 8;
			}	}
			else {		// Fallback: Use fixed safe margins if no display information is available.
				juce::Logger::writeToLog ("CustomFileChooser: no display found for screen bounds");
				if (isRunningOnIPad()) {        // iPad
					titleTop = 60;
					border = portrait ? 8 : 64;
					bottomMargin = 28;
				} else if (portrait) {          // iPhone / portrait
					titleTop = 64;
					border = 8;
					bottomMargin = 42;
				} else {                        // iPhone / landscape
					titleTop = 8;
					border = 64;
					bottomMargin = 28;
		}	}	}
		// Plugin:
		else if (! isRunningOnIPad() && ! portrait) {    // iPhone – landscape
			if (display != nullptr) {
				const auto safeInsets = display->safeAreaInsets;
				const auto pluginBounds = getScreenBounds();
				const auto displayBounds = display->logicalBounds;
	
				constexpr int edgeTolerance = 2;
	
				const bool usesFullDisplayWidth =
					   std::abs (pluginBounds.getX() - displayBounds.getX()) <= edgeTolerance
					&& std::abs (pluginBounds.getRight() - displayBounds.getRight()) <= edgeTolerance;
	
				if (usesFullDisplayWidth) {     // Plugin with Full Display Width, e.g. GarageBand
					const auto horizontalSafeInset = juce::jmax (safeInsets.getLeft(), safeInsets.getRight());
					border = horizontalSafeInset + 8;
				}
				else if (getWidth() > 560) {    // if NOT Full Display Width
					border = 64;
			}	}
			else if (getWidth() > 560) {    // Fallback, if display == nullptr
				border = 64;
		}	}
		else if (isRunningOnIPad() && getWidth() > 560) {    // iPad – wide plugin layout
			border = 64;
		}   // Otherwise: defaults for plugin (top, border = 8), iPhone – portrait & iPad
	
		const auto contentWidth = getWidth() - 2 * border;
		const auto bottomButtonsWidth = (contentWidth - 3 * spacing) / 4;
        
		titleLabel.setBounds (border, titleTop, contentWidth, titleHeight);
	
		const auto upperRowY = titleLabel.getBottom();
		selectDirectory.setBounds                                (border, upperRowY, bottomButtonsWidth, buttonHeight);
		Cancel.setBounds           (getWidth() - border - rightButtonWidth, upperRowY, rightButtonWidth, buttonHeight);
		Save.setBounds         (Cancel.getX() - spacing - rightButtonWidth, upperRowY, rightButtonWidth, buttonHeight);
		CancelDelete.setBounds     (getWidth() - border - rightButtonWidth, upperRowY, rightButtonWidth, buttonHeight);
		Delete.setBounds (CancelDelete.getX() - spacing - rightButtonWidth, upperRowY, rightButtonWidth, buttonHeight);
	
		if (isStandalone) {     // Standalone
			const auto editorX = border + bottomButtonsWidth + spacing;
			if (isSaveMode || DeleteMode) {
				fileNameEditor.setBounds (editorX, upperRowY, 
					contentWidth - bottomButtonsWidth - spacing - 2 * rightButtonWidth - 2 * spacing, buttonHeight);
			} else {
				fileNameEditor.setBounds (
					editorX, upperRowY, contentWidth - bottomButtonsWidth - spacing, buttonHeight);
			} 
		} else {                // Plugin
			if (isSaveMode || DeleteMode) {
				fileNameEditor.setBounds (
					border, upperRowY, contentWidth - 2 * rightButtonWidth - spacing, buttonHeight);
			} else {
				fileNameEditor.setBounds (border, upperRowY, contentWidth, buttonHeight);
		}   }
		const auto bottomButtonsY = getHeight() - bottomMargin - buttonHeight;
		
		saveButton  .setBounds (border,                            bottomButtonsY, bottomButtonsWidth, buttonHeight);
		loadButton  .setBounds (saveButton.getRight()   + spacing, bottomButtonsY, bottomButtonsWidth, buttonHeight);
		deleteButton.setBounds (loadButton.getRight()   + spacing, bottomButtonsY, bottomButtonsWidth, buttonHeight);
		doneButton  .setBounds (deleteButton.getRight() + spacing, bottomButtonsY, bottomButtonsWidth, buttonHeight);
	
		const auto fileListTop = fileNameEditor.getBottom() + spacing;
		const auto fileListBottom = bottomButtonsY - spacing;
		fileList.setBounds (border, fileListTop, contentWidth, juce::jmax (0, fileListBottom - fileListTop));
	}
    
    // callback functions
    std::function<void (juce::File)> onFileSelected;
    std::function<void()> onDone;
    
    // Expose a method to query the current mode
    bool isInSaveMode() const { return isSaveMode; }
    
    //==============================================================================
    // Set the last used file (to be called from the editor)
    void setLastUsedFile (const juce::File& file)
    {
        lastUsedFile = file;
        refreshFileList(); // Refresh and apply selection immediately
    }
    
    juce::String removeExtension (const juce::String& fileName) 
    {
        if (fileName.endsWithIgnoreCase (".xml")) {
            return fileName.dropLastCharacters (4);
        }
        return fileName; // displays Text in fileNameEditor without Extension
    }
    
private:
    // Components
    juce::Label titleLabel;
    juce::TextEditor fileNameEditor;
    juce::ListBox fileList;
    juce::TextButton saveButton, loadButton, deleteButton, doneButton, 
                     Save, Cancel, Delete, CancelDelete, selectDirectory;
    
    // State and Data
    juce::File directory;
    juce::String pattern;
    juce::File selectedFile;
    juce::File lastUsedFile; // Tracks the last used file
    juce::Array<juce::File> files;
    
    // Helper methods
    bool isSaveMode = false; // Tracks current mode dynamically
    bool DeleteMode = false;
    
    void enterSaveMode()
    {
        isSaveMode = true;
        if (!selectedFile.existsAsFile())
        { fileNameEditor.setText ("Enter File Name"); }
        else
        { fileNameEditor.setText (removeExtension (selectedFile.getFileName())); } // remove ".xml"
        fileNameEditor.setInterceptsMouseClicks (true, true); // Re-enable mouse clicks
        fileNameEditor.setReadOnly (false);
        fileNameEditor.setSelectAllWhenFocused (true);
        fileNameEditor.grabKeyboardFocus();
        fileNameEditor.setColour (juce::TextEditor::backgroundColourId, juce::Colours::white);
        fileNameEditor.setColour (juce::TextEditor::highlightedTextColourId, juce::Colours::black);
        fileNameEditor.applyColourToAllText (juce::Colours::black, true); // changeCurrentTextColour = true
        Save.setVisible (true);
        Save.toFront (false); // shouldAlsoGainKeyboardFocus = false
        Cancel.setVisible (true);
        Cancel.toFront (false);
        resized();
    }
    
    void exitSaveMode()
    {
        isSaveMode = false;
        DeleteMode = false;
        if (!selectedFile.existsAsFile())
        { fileNameEditor.setText ("Select File"); }
        else { fileNameEditor.setText (removeExtension (selectedFile.getFileName())); }
        fileNameEditor.setReadOnly (true);
        fileNameEditor.setSelectAllWhenFocused (false);
        fileNameEditor.setCaretPosition (0); // deselect All
        fileNameEditor.setInterceptsMouseClicks (false, false); // Block mouse clicks (component, children)
        fileNameEditor.setColour (juce::TextEditor::backgroundColourId, 
                        getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
        fileNameEditor.setColour (juce::TextEditor::highlightedTextColourId, juce::Colours::white);
        fileNameEditor.applyColourToAllText (juce::Colours::white, true);
        Save.setVisible (false);
        Cancel.setVisible (false);
        Delete.setVisible (false);
        CancelDelete.setVisible (false);
        resized();
    }
    
    void enterDeleteMode()
    {
        DeleteMode = true;
        if (!selectedFile.existsAsFile())
        { fileNameEditor.setText ("Select File"); }
        fileNameEditor.setReadOnly (true);
        fileNameEditor.setSelectAllWhenFocused (false);
        fileNameEditor.setCaretPosition (0);
        fileNameEditor.setInterceptsMouseClicks (false, false); // Block mouse clicks
        fileNameEditor.setColour (juce::TextEditor::backgroundColourId, 
                        getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
        fileNameEditor.applyColourToAllText (juce::Colour (0xFF0096FF), true);
        Delete.setVisible (true);
        Delete.toFront (false);
        CancelDelete.setVisible (true);
        CancelDelete.toFront (false);
        resized();
        repaint();
    }
    
    void exitDeleteMode()
    {
        isSaveMode = false;
        DeleteMode = false;
        if (!selectedFile.existsAsFile())
        { fileNameEditor.setText ("Select File"); }
        fileNameEditor.setReadOnly (true);
        fileNameEditor.setInterceptsMouseClicks (false, false); // Block mouse clicks
        fileNameEditor.setColour (juce::TextEditor::backgroundColourId, 
                        getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
        fileNameEditor.applyColourToAllText (juce::Colours::white, true);
        Save.setVisible (false);
        Cancel.setVisible (false);
        Delete.setVisible (false);
        CancelDelete.setVisible (false);
        resized();
        repaint();
    }
    
    void handleFileNameInput() // (Call for save)
    {
        auto enteredName = fileNameEditor.getText().trim();
        if (enteredName.isEmpty()) {
            juce::Logger::writeToLog ("Error: The file name cannot be empty.");
            return;
        }
        if (!enteredName.endsWithIgnoreCase (".xml")) {
            enteredName += ".xml";
        }
        selectedFile = directory.getChildFile (enteredName);
        
        if (onFileSelected) { onFileSelected (selectedFile); }
    }
    
    void deleteSelectedFile()
    {
        if (!selectedFile.existsAsFile()) {
            juce::Logger::writeToLog ("No file selected or file does not exist.");
            return;
        }
        if (!selectedFile.deleteFile()) {
            juce::Logger::writeToLog ("Failed to delete file: " + selectedFile.getFullPathName());
            return;
        }
		if (selectedFile == lastUsedFile) // Reset lastUsedFile if it matches the deleted file
		{   lastUsedFile = juce::File(); }

        refreshFileList(); // Refresh file list after deletion
        fileNameEditor.setText (removeExtension (selectedFile.getFileName())); // remove ".xml"
    }

    void refreshFileList()
    {
        files = directory.findChildFiles (juce::File::TypesOfFileToFind::findFiles, false, pattern);
        
		// Natural, case-insensitive alphabetical sort: "File2" < "File10" (stable keeps order of equal names)
		std::stable_sort (files.begin(), files.end(), [] (const juce::File& a, const juce::File& b)
			{
				return a.getFileName().compareNatural (b.getFileName(), /*isCaseSensitive=*/false) < 0;
			});
		
        fileList.updateContent();
        
        // Automatically select the last used file if it exists
        if (lastUsedFile.existsAsFile())
        {
            int index = files.indexOf (lastUsedFile);
            if (index != -1)
            {
                fileList.selectRow (index);
                selectedFile = lastUsedFile; // Update the currently selected file
                fileList.repaint(); // not necessary, jus in case ...
                fileNameEditor.setText (removeExtension (selectedFile.getFileName())); // remove ".xml"
            } else {
                selectedFile = juce::File(); // Clear selection if the file isn't found
            }
        } else {
            selectedFile = juce::File();     // Clear selection if no lastUsedFile exists
        }
    }
    
    // ListBoxModel overrides
    int getNumRows() override { return files.size(); } // number of files in the list
    
    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override
    {
        if (rowIsSelected)
            g.fillAll (juce::Colour (0xFF0096FF));
        
        g.setColour (juce::Colours::black);
        g.setFont (juce::Font (juce::FontOptions().withPointHeight (18.0f)));
        
        g.drawText (files[rowNumber].getFileName(), 10, 0, width - 10, height, juce::Justification::centredLeft);
    }
    
    void listBoxItemClicked (int row, const juce::MouseEvent&) override
    {
        selectedFile = files[row];
        fileNameEditor.setText (removeExtension (selectedFile.getFileName())); // remove ".xml"
    }
    
    // IOSContextMenu
	int lastCursorPosition = -1;
	
	void mouseUp (const juce::MouseEvent& e) override
	{
		if (auto* editor = dynamic_cast<juce::TextEditor*> (e.eventComponent))
		{   
			int clicks = e.getNumberOfClicks(); 
			if (clicks > 1)
			{
				juce::Timer::callAfterDelay (10, [this, editor]
				 { showIOSContextMenu (*editor); });
			}
			else { showIOSContextMenu (*editor); }
		}
	}
	
	void showIOSContextMenu (juce::TextEditor& editor)
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
	
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomFileChooser)
};
