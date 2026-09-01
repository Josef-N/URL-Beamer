//                 PluginProcessor.cpp    –   JUCE plugin processor

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
namespace // Anonymous namespace; visible only in this .cpp file
{
    const juce::Identifier appStateType       { "PARAMETERS" };
    const juce::Identifier inputNoteProperty  { "InNote" };
    const juce::Identifier inputChannelProperty { "InChannel" };
    const juce::Identifier midiThruProperty   { "MidiThru" };
}

//==============================================================================
CallAppAudioProcessor::CallAppAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input", juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
      treeState  (*this, nullptr, "PARAMETER", createParameterLayout()),
      parameters (*this, nullptr, appStateType, {})
#endif
{
    // The processor owns all persistent default state.
    parameters.state.setProperty ("button1Text", "Midi Designer", nullptr);
    parameters.state.setProperty ("button2Text", "PdParty",       nullptr);
    parameters.state.setProperty ("button3Text", "Apple Books",   nullptr);
    parameters.state.setProperty ("button4Text", "Web",           nullptr);
    parameters.state.setProperty ("button5Text", "...",           nullptr);
    parameters.state.setProperty ("button6Text", "...",           nullptr);
    parameters.state.setProperty ("button7Text", "...",           nullptr);
    parameters.state.setProperty ("button8Text", "...",           nullptr);
    parameters.state.setProperty ("link1URL", "MidiDesignerPro.audiobus://", nullptr);
    parameters.state.setProperty ("link2URL", "pdparty://",                  nullptr);
    parameters.state.setProperty ("link3URL", "ibooks://",                   nullptr);
    parameters.state.setProperty ("link4URL", "https://novotny.klingt.org/Apps/URLBeamer/Support", nullptr);
    parameters.state.setProperty ("link5URL", "", nullptr);
    parameters.state.setProperty ("link6URL", "", nullptr);
    parameters.state.setProperty ("link7URL", "", nullptr);
    parameters.state.setProperty ("link8URL", "", nullptr);
    parameters.state.setProperty ("Color1", "FF283338", nullptr);  // Def. JUCE button color
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
    parameters.state.setProperty ("button1Data", "120_0", nullptr);  // Def, Channel Off
    parameters.state.setProperty ("button2Data", "121_0", nullptr);
    parameters.state.setProperty ("button3Data", "122_0", nullptr);
    parameters.state.setProperty ("button4Data", "123_0", nullptr);
    parameters.state.setProperty ("button5Data", "124_0", nullptr);
    parameters.state.setProperty ("button6Data", "125_0", nullptr);
    parameters.state.setProperty ("button7Data", "126_0", nullptr);
    parameters.state.setProperty ("button8Data", "127_0", nullptr);
    parameters.state.setProperty (inputNoteProperty,    1,     nullptr);  // Default root note = 1
    parameters.state.setProperty (inputChannelProperty, 0,     nullptr);  // Default = Off
    parameters.state.setProperty (midiThruProperty,     false, nullptr);  // Default = OFF (Pass Notes)

    parameters.state.setProperty ("ShowRows", false, nullptr);
    parameters.state.setProperty ("ColorRangeToggle", false, nullptr);    // "Full" (off)
    parameters.state.setProperty ("lastUsedFile", "", nullptr);           // Default empty state

    updateRealtimeStateFromAppState();
}

CallAppAudioProcessor::~CallAppAudioProcessor() {}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout CallAppAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
									         // parameterID, parameter name, default value
    params.push_back (std::make_unique<juce::AudioParameterBool> ("app1", "App 1", false));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("app2", "App 2", false));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("app3", "App 3", false));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("app4", "App 4", false));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("app5", "App 5", false));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("app6", "App 6", false));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("app7", "App 7", false));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("app8", "App 8", false));

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
    juce::ignoreUnused (index);
}

const juce::String CallAppAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void CallAppAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void CallAppAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (sampleRate, samplesPerBlock);
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
    { return false; }

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    { return false; }
   #endif

    return true;
  #endif
}
#endif

//==============================================================================
void CallAppAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (buffer);

    const auto rootNote = inputNoteRoot.load (std::memory_order_relaxed);  // Offset slider
    const auto inChannel = inputChannel.load (std::memory_order_relaxed);  // 0 = Off, 17 = Omni
    const auto shouldBlockMappedNotes = blockMappedNotes.load (std::memory_order_relaxed);
    const auto canSendEditorTriggers = editorEventConsumerActive.load (std::memory_order_acquire);

    juce::MidiBuffer filteredMidi;

    for (const auto metadata : midiMessages)
    {
        const auto message = metadata.getMessage();
        const auto samplePosition = metadata.samplePosition;

        bool isMappedNote = false;

        if (inChannel != 0 && (message.isNoteOn() || message.isNoteOff()))
        {
            const auto note = message.getNoteNumber();
            const auto channel = message.getChannel();

            const bool channelMatches = inChannel == 17 || channel == inChannel;
            const bool noteMatches = note >= rootNote && note <= rootNote + 7;  // Inside the triggerable range

            isMappedNote = channelMatches && noteMatches;

            if (isMappedNote && message.isNoteOn() && canSendEditorTriggers)
            {
                pushIncomingTrigger (note - rootNote);
            }
        }
                            // shouldBlockMappedNotes: // Block notes when MidiThru is ON (true)
        if (! (isMappedNote && shouldBlockMappedNotes))
            filteredMidi.addEvent (message, samplePosition);
    }

    midiMessages.swapWith (filteredMidi);
    PendingMidiEvent event;

    while (popOutgoingMidiEvent (event))
    {
        const auto message =
            juce::MidiMessage::noteOn (event.channel, event.note, static_cast<juce::uint8> (event.velocity));

        midiMessages.addEvent (message, 0);
    }
}

//==============================================================================
void CallAppAudioProcessor::sendMidiNoteOn (int midiNoteNumber, int velocity, int midiChannel)
{
    if (! juce::isPositiveAndBelow (midiChannel, 17)) return;
    if (! juce::isPositiveAndBelow (midiNoteNumber, 128)) return;

    PendingMidiEvent event;

    event.note = midiNoteNumber;
    event.velocity = juce::jlimit (0, 127, velocity);
    event.channel = midiChannel;

    int startIndex1 = 0;
    int blockSize1 = 0;
    int startIndex2 = 0;
    int blockSize2 = 0;

    outgoingMidiFifo.prepareToWrite (1, startIndex1, blockSize1, startIndex2, blockSize2);

    juce::ignoreUnused (startIndex2, blockSize2);

    if (blockSize1 == 0) return;

    outgoingMidiEvents[static_cast<std::size_t> (startIndex1)] = event;

    outgoingMidiFifo.finishedWrite (1);
}

bool CallAppAudioProcessor::popOutgoingMidiEvent (PendingMidiEvent& event) noexcept
{
    int startIndex1 = 0;
    int blockSize1 = 0;
    int startIndex2 = 0;
    int blockSize2 = 0;

    outgoingMidiFifo.prepareToRead (1, startIndex1, blockSize1, startIndex2, blockSize2);

    juce::ignoreUnused (startIndex2, blockSize2);

    if (blockSize1 == 0) return false;

    event = outgoingMidiEvents[static_cast<std::size_t> (startIndex1)];

    outgoingMidiFifo.finishedRead (1);

    return true;
}

//==============================================================================
bool CallAppAudioProcessor::pushIncomingTrigger (int buttonIndex) noexcept
{
    if (! juce::isPositiveAndBelow (buttonIndex, 8)) return false;

    int startIndex1 = 0;
    int blockSize1 = 0;
    int startIndex2 = 0;
    int blockSize2 = 0;

    incomingTriggerFifo.prepareToWrite (1, startIndex1, blockSize1, startIndex2, blockSize2);

    juce::ignoreUnused (startIndex2, blockSize2);

    if (blockSize1 == 0) return false;

    incomingTriggers[static_cast<std::size_t> (startIndex1)] = buttonIndex;

    incomingTriggerFifo.finishedWrite (1);

    return true;
}

bool CallAppAudioProcessor::popIncomingTrigger (int& buttonIndex) noexcept
{
    int startIndex1 = 0;
    int blockSize1 = 0;
    int startIndex2 = 0;
    int blockSize2 = 0;

    incomingTriggerFifo.prepareToRead (1, startIndex1, blockSize1, startIndex2, blockSize2);

    juce::ignoreUnused (startIndex2, blockSize2);

    if (blockSize1 == 0) return false;

    buttonIndex = incomingTriggers[static_cast<std::size_t> (startIndex1)];

    incomingTriggerFifo.finishedRead (1);

    return true;
}

void CallAppAudioProcessor::setEditorEventConsumerActive (bool isActive) noexcept
{
    editorEventConsumerActive.store (isActive, std::memory_order_release);
}

//==============================================================================
juce::ValueTree CallAppAudioProcessor::getAppStateCopy()
{
    const juce::ScopedLock lock (appStateLock);
    return parameters.copyState();
}

void CallAppAudioProcessor::replaceAppState (const juce::ValueTree& newState, bool notifyEditor)
{
    if (! newState.isValid() || newState.getType() != appStateType)
    { return; }

    {
        const juce::ScopedLock lock (appStateLock);
        parameters.replaceState (newState.createCopy());
    }

    updateRealtimeStateFromAppState();

    if (notifyEditor)
    {
        stateRevision.fetch_add (1, std::memory_order_release);
    }
}

void CallAppAudioProcessor::setAppProperty (const juce::Identifier& property, const juce::var& value)
{
    {
        const juce::ScopedLock lock (appStateLock);
        parameters.state.setProperty (property, value, nullptr);
    }

    if (property == inputNoteProperty)
    {
        inputNoteRoot.store (
            juce::jlimit (0, 120, static_cast<int> (value)), std::memory_order_relaxed);
    }
    else if (property == inputChannelProperty)
    {
        inputChannel.store (
            juce::jlimit (0, 17, static_cast<int> (value)), std::memory_order_relaxed);
    }
    else if (property == midiThruProperty)
    {
        blockMappedNotes.store (
            static_cast<bool> (value), std::memory_order_relaxed);
    }
}

juce::var CallAppAudioProcessor::getAppProperty (
    const juce::Identifier& property, const juce::var& defaultValue) const
{
    const juce::ScopedLock lock (appStateLock);

    return parameters.state.getProperty (property, defaultValue);
}

std::unique_ptr<juce::XmlElement> CallAppAudioProcessor::createAppStateXml()
{
    return getAppStateCopy().createXml();
}

std::uint32_t
CallAppAudioProcessor::getStateRevision() const noexcept
{
    return stateRevision.load (std::memory_order_acquire);
}

//==============================================================================
void CallAppAudioProcessor::setInputNoteRoot (int note)
{
    setAppProperty (inputNoteProperty, juce::jlimit (0, 120, note));
}

void CallAppAudioProcessor::setInputChannel (int channel)
{
    setAppProperty (inputChannelProperty, juce::jlimit (0, 17, channel));
}

void CallAppAudioProcessor::setBlockMappedNotes (bool shouldBlock)
{
    setAppProperty (midiThruProperty, shouldBlock);
}

void CallAppAudioProcessor::updateRealtimeStateFromAppState()
{
    int newRootNote = 1;
    int newInputChannel = 0;
    bool newBlockMappedNotes = false;
    
    {
        const juce::ScopedLock lock (appStateLock);

        newRootNote = juce::jlimit (
            0, 120, static_cast<int> (parameters.state.getProperty (inputNoteProperty, 1)));

        newInputChannel = juce::jlimit (
            0, 17, static_cast<int> (parameters.state.getProperty (inputChannelProperty, 0)));

        newBlockMappedNotes =
            static_cast<bool> (parameters.state.getProperty (midiThruProperty, false));
    }
    
    inputNoteRoot.store (newRootNote, std::memory_order_relaxed);
    inputChannel.store (newInputChannel, std::memory_order_relaxed);
    blockMappedNotes.store (newBlockMappedNotes, std::memory_order_relaxed);
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
    if (auto xmlState = createAppStateXml())
        copyXmlToBinary (*xmlState, destData);
}

void CallAppAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xmlState = getXmlFromBinary (data, sizeInBytes);

    if (xmlState == nullptr || ! xmlState->hasTagName (appStateType))
    {
        return;
    }

    const auto newState = juce::ValueTree::fromXml (*xmlState);

    replaceAppState (newState, true);
}

//==============================================================================
// This creates new instances of the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CallAppAudioProcessor();
}
