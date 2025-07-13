
//                 PluginProcessor.h    –   JUCE plugin processor

#pragma once

#include <JuceHeader.h>

//==============================================================================
class CallAppAudioProcessor  : public juce::AudioProcessor
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

    // for Button Attachments
    juce::AudioProcessorValueTreeState treeState;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    // for Save State
    juce::AudioProcessorValueTreeState& getValueTreeState() { return parameters; }
    juce::ValueTree getDeferredState() const { return deferredState; }
    void clearDeferredState() { deferredState = {}; }
    // for saveSettings and loadSettings
    const juce::ValueTree& getParametersValueTree() const { return parameters.state; }
    void setParametersValueTree (const juce::ValueTree& newState) { parameters.state = newState; }
    std::unique_ptr<juce::XmlElement> createParametersXml() const { return parameters.state.createXml(); }
    // for Midi Notes
    void sendMidiNoteOn (int midiNoteNumber, int velocity, int midiChannel);
    
private:
    juce::AudioProcessorValueTreeState parameters; // for Save State
    juce::ValueTree deferredState;  // Store state until the editor opens
    std::vector<juce::MidiMessage> midiQueue;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CallAppAudioProcessor)
};
