
//                 PluginProcessor.cpp    –   JUCE plugin processor

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CallAppAudioProcessor::CallAppAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
       treeState (*this, nullptr, "PARAMETER", createParameterLayout()),
       parameters(*this, nullptr, "PARAMETERS", {})
#endif
{
    // Initialize all state properties directly in the ValueTree
    parameters.state.setProperty ("button1Text", "...", nullptr);
    parameters.state.setProperty ("button2Text", "...", nullptr);
    parameters.state.setProperty ("button3Text", "...", nullptr);
    parameters.state.setProperty ("button4Text", "...", nullptr);
    parameters.state.setProperty ("button5Text", "...", nullptr);
    parameters.state.setProperty ("button6Text", "...", nullptr);
    parameters.state.setProperty ("button7Text", "...", nullptr);
    parameters.state.setProperty ("button8Text", "...", nullptr);
    parameters.state.setProperty ("link1URL", "", nullptr);
    parameters.state.setProperty ("link2URL", "", nullptr);
    parameters.state.setProperty ("link3URL", "", nullptr);
    parameters.state.setProperty ("link4URL", "", nullptr);
    parameters.state.setProperty ("link5URL", "", nullptr);
    parameters.state.setProperty ("link6URL", "", nullptr);
    parameters.state.setProperty ("link7URL", "", nullptr);
    parameters.state.setProperty ("link8URL", "", nullptr);
    parameters.state.setProperty ("Color1", "FF283338", nullptr); // Def. JUCE button color
	parameters.state.setProperty ("Color2", "FF283338", nullptr);
	parameters.state.setProperty ("Color3", "FF283338", nullptr);
	parameters.state.setProperty ("Color4", "FF283338", nullptr);
	parameters.state.setProperty ("Color5", "FF283338", nullptr);
	parameters.state.setProperty ("Color6", "FF283338", nullptr);
	parameters.state.setProperty ("Color7", "FF283338", nullptr);
	parameters.state.setProperty ("Color8", "FF283338", nullptr);
	parameters.state.setProperty ("TextColor1", juce::Colours::white.toString(), nullptr);
	parameters.state.setProperty ("TextColor2", juce::Colours::white.toString(), nullptr);
	parameters.state.setProperty ("TextColor3", juce::Colours::white.toString(), nullptr);
	parameters.state.setProperty ("TextColor4", juce::Colours::white.toString(), nullptr);
	parameters.state.setProperty ("TextColor5", juce::Colours::white.toString(), nullptr);
	parameters.state.setProperty ("TextColor6", juce::Colours::white.toString(), nullptr);
	parameters.state.setProperty ("TextColor7", juce::Colours::white.toString(), nullptr);
	parameters.state.setProperty ("TextColor8", juce::Colours::white.toString(), nullptr);
	parameters.state.setProperty ("button1Data", "120_0", nullptr); // Def, Channel Off
    parameters.state.setProperty ("button2Data", "121_0", nullptr);
    parameters.state.setProperty ("button3Data", "122_0", nullptr);
    parameters.state.setProperty ("button4Data", "123_0", nullptr);
    parameters.state.setProperty ("button5Data", "124_0", nullptr);
    parameters.state.setProperty ("button6Data", "125_0", nullptr);
    parameters.state.setProperty ("button7Data", "126_0", nullptr);
    parameters.state.setProperty ("button8Data", "127_0", nullptr);
    if (!parameters.state.hasProperty ("InNote")) // // Avoids overriding values from a loaded preset
		parameters.state.setProperty  ("InNote", 1, nullptr);       // Default root note = 1
	if (!parameters.state.hasProperty ("InChannel"))
		parameters.state.setProperty  ("InChannel", 0, nullptr);    // Default = Off
	if (!parameters.state.hasProperty ("MidiThru"))
		parameters.state.setProperty  ("MidiThru", false, nullptr); // Default = OFF (Pass Notes)
    
    parameters.state.setProperty ("ShowRows", false, nullptr);
    parameters.state.setProperty ("ColorRangeToggle", false, nullptr);  // "Full" (off)
    parameters.state.setProperty ("lastUsedFile", "", nullptr);  // Default empty state
} 

CallAppAudioProcessor::~CallAppAudioProcessor() {}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout CallAppAudioProcessor::createParameterLayout()
{
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
									        // parameterID, parameter name, default value
    auto link1Param = std::make_unique<juce::AudioParameterBool> ("app1", "App 1", false);
    auto link2Param = std::make_unique<juce::AudioParameterBool> ("app2", "App 2", false);
    auto link3Param = std::make_unique<juce::AudioParameterBool> ("app3", "App 3", false);
    auto link4Param = std::make_unique<juce::AudioParameterBool> ("app4", "App 4", false);
    auto link5Param = std::make_unique<juce::AudioParameterBool> ("app5", "App 5", false);
    auto link6Param = std::make_unique<juce::AudioParameterBool> ("app6", "App 6", false);
    auto link7Param = std::make_unique<juce::AudioParameterBool> ("app7", "App 7", false);
    auto link8Param = std::make_unique<juce::AudioParameterBool> ("app8", "App 8", false);
    
    params.push_back(std::move(link1Param));
    params.push_back(std::move(link2Param));
    params.push_back(std::move(link3Param));
    params.push_back(std::move(link4Param));
    params.push_back(std::move(link5Param));
    params.push_back(std::move(link6Param));
    params.push_back(std::move(link7Param));
    params.push_back(std::move(link8Param));
    
    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String CallAppAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CallAppAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CallAppAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CallAppAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CallAppAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CallAppAudioProcessor::getNumPrograms()
{
    return 1;
}
int CallAppAudioProcessor::getCurrentProgram()
{
    return 0;
}
void CallAppAudioProcessor::setCurrentProgram (int index)
{
}
const juce::String CallAppAudioProcessor::getProgramName (int index)
{
    return {};
}
void CallAppAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CallAppAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void CallAppAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CallAppAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void CallAppAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (buffer);

    juce::MidiBuffer filteredMidi;

    if (auto* editor = dynamic_cast<CallAppAudioProcessorEditor*> (getActiveEditor()))
    {
        int rootNote  = editor->getInputNoteRoot();
		int inChannel = editor->getInputChannel();

        for (const auto metadata : midiMessages)
        {
            const auto msg = metadata.getMessage();
            const int samplePos = metadata.samplePosition;

            bool blockThis = false;

            if (msg.isNoteOn())
            {
                int note = msg.getNoteNumber();
                int chan = msg.getChannel();

                bool chanMatch = (inChannel == 17 || chan == inChannel);
                bool noteMatch = (note >= rootNote && note <= rootNote + 7);

                if (chanMatch && noteMatch)
                {
                    editor->handleIncomingMidiNote (note, chan); // triggers the hyperlink
                    
                    if (editor->shouldBlockNotes())
			            blockThis = true; // block this Note-On from passing through
                }
            }
            else if (msg.isNoteOff())
            {
                int note = msg.getNoteNumber();
                int chan = msg.getChannel();

                bool chanMatch = (inChannel == 17 || chan == inChannel);
                bool noteMatch = (note >= rootNote && note <= rootNote + 7);

                if (chanMatch && noteMatch && editor->shouldBlockNotes())
                    blockThis = true; // block matching Note-Off
            }

            if (!blockThis)
                filteredMidi.addEvent (msg, samplePos);
        }
    }
    else
    {
        filteredMidi = midiMessages;      // No editor → pass all MIDI through untouched
    }

    midiMessages.swapWith (filteredMidi); // Replace incoming MIDI with filtered version

    // Add any queued outgoing MIDI messages (from sendMidiNoteOn)
    for (const auto& message : midiQueue)
        midiMessages.addEvent (message, 0);

    midiQueue.clear(); // Clear after sending
}

void CallAppAudioProcessor::sendMidiNoteOn (int midiNoteNumber, int velocity, int midiChannel)
{
    if (midiChannel > 0 && midiChannel <= 16) // Ensure valid MIDI channel
    {
        juce::MidiMessage note = juce::MidiMessage::noteOn (midiChannel, midiNoteNumber, (juce::uint8)velocity);
        midiQueue.push_back (note);
    }
}

//==============================================================================
bool CallAppAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* CallAppAudioProcessor::createEditor()
{
    return new CallAppAudioProcessorEditor (*this);
}

//==============================================================================
void CallAppAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{ 
    if (auto* editor = dynamic_cast<CallAppAudioProcessorEditor*>(getActiveEditor()))
    {
//      editor->saveStateToValueTree(); // Public method without wrapper
        editor->triggerStateSave();  // Save UI state before preset save
    }                                // Call the public wrapper
    
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}
void CallAppAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState && xmlState->hasTagName (parameters.state.getType()))
    {
        parameters.replaceState (juce::ValueTree::fromXml(*xmlState));
        deferredState = parameters.state;  // Store the state if editor is not open
        
        // Defer UI updates if editor is open
        if (auto* editor = dynamic_cast<CallAppAudioProcessorEditor*>(getActiveEditor()))
        {
            juce::MessageManager::callAsync([editor, state = parameters.state]()
            {
//              editor->loadStateFromValueTree (parameters.state); // Public method without wrapper
                editor->triggerStateLoad(); // Load state on the UI thread through the wrapper
                
                // If CustomFileChooser is open, refresh the list, Access through the getter
                if (auto* chooser = editor->getFileChooser())
                {
                    juce::String lastPath = state.getProperty ("lastUsedFile", "").toString();
                    if (!lastPath.isEmpty())
                    {
                        juce::File lastFile (lastPath);
                        if (lastFile.existsAsFile())
                        {
                            chooser->setLastUsedFile (lastFile);  // Update dynamically
                        }
                    }
                }
            });
        }
    }
/*  else    // No valid preset found → Apply default values
    {       // prevents the plugin to crash
        parameters.state.setProperty ("button1Data", "120_0", nullptr);
        parameters.state.setProperty ("button2Data", "121_0", nullptr);
        parameters.state.setProperty ("button3Data", "122_0", nullptr);
        parameters.state.setProperty ("button4Data", "123_0", nullptr);
        parameters.state.setProperty ("button5Data", "124_0", nullptr);
        parameters.state.setProperty ("button6Data", "125_0", nullptr);
        parameters.state.setProperty ("button7Data", "126_0", nullptr);
        parameters.state.setProperty ("button8Data", "127_0", nullptr);
    }
*/
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CallAppAudioProcessor();
}
