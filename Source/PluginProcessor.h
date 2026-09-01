//                 PluginProcessor.h    –   JUCE plugin processor

#pragma once

#include <JuceHeader.h>
#include <array>
#include <atomic>
#include <cstdint>

//==============================================================================
class CallAppAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    CallAppAudioProcessor();
    ~CallAppAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Host parameters used by button attachments.
    juce::AudioProcessorValueTreeState treeState;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //==============================================================================
    // Persistent application state.
    juce::ValueTree getAppStateCopy();

    void replaceAppState (const juce::ValueTree& newState, bool notifyEditor);

    void setAppProperty (const juce::Identifier& property, const juce::var& value);

    juce::var getAppProperty (const juce::Identifier& property,
                              const juce::var& defaultValue = {}) const;

    std::unique_ptr<juce::XmlElement> createAppStateXml();

    std::uint32_t getStateRevision() const noexcept;

    //==============================================================================
    // Realtime MIDI state.
    void setInputNoteRoot (int note);
    void setInputChannel (int channel);
    void setBlockMappedNotes (bool shouldBlock);

    //==============================================================================
    // Message thread -> audio thread.
    void sendMidiNoteOn (int midiNoteNumber,
                         int velocity,
                         int midiChannel);

    //==============================================================================
    // Audio thread -> message thread.
    bool popIncomingTrigger (int& buttonIndex) noexcept;

    void setEditorEventConsumerActive (bool isActive) noexcept;

private:
    //==============================================================================
    // Persistent state for button texts, URLs, colours, MIDI settings,
    // lastUsedFile and all other saved properties.
    juce::AudioProcessorValueTreeState parameters;

    mutable juce::CriticalSection appStateLock;

    //==============================================================================
    // Realtime-safe copies of the MIDI properties stored in parameters.state.
    std::atomic<int> inputNoteRoot { 1 };
    std::atomic<int> inputChannel { 0 };
    std::atomic<bool> blockMappedNotes { false };

    // Increased whenever a host preset or settings file replaces the state.
    std::atomic<std::uint32_t> stateRevision { 0 };

    // Indicates whether an editor is available to consume button triggers.
    std::atomic<bool> editorEventConsumerActive { false };

    //==============================================================================
    // Outgoing MIDI queue: message thread -> audio thread.
    struct PendingMidiEvent
    {
        int note = 0;
        int velocity = 0;
        int channel = 0;
    };

    static constexpr int midiQueueCapacity = 128;

    juce::AbstractFifo outgoingMidiFifo { midiQueueCapacity };

    std::array<PendingMidiEvent, midiQueueCapacity> outgoingMidiEvents {};

    //==============================================================================
    // Incoming button-trigger queue: audio thread -> message thread.
    static constexpr int triggerQueueCapacity = 64;

    juce::AbstractFifo incomingTriggerFifo { triggerQueueCapacity };

    std::array<int, triggerQueueCapacity> incomingTriggers {};

    //==============================================================================
    bool popOutgoingMidiEvent (PendingMidiEvent& event) noexcept;
    bool pushIncomingTrigger (int buttonIndex) noexcept;

    void updateRealtimeStateFromAppState();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CallAppAudioProcessor)
};
