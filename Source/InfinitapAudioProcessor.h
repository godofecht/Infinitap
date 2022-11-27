/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/

struct PanLeft
{
    // TODO maybe convert to a sin approximate function for better performance
    static inline float process (float pan)
    {
        return sinf ((float)((1.0f - pan) * juce::MathConstants<float>::twoPi));
    }
};

struct PanRight
{
    // TODO maybe convert to a sin approximate function for better performance
    static inline float process (float pan)
    {
        return sinf ((float)(pan * juce::MathConstants<float>::twoPi));
    }
};

class TapDelayProcessor
{
    
    float maxDelayInSeconds { 2.0 };
    AudioBuffer<float> delayLine;
    AudioBuffer<float> feedbackBuffer;
    double sampleRate = 0.0;
    int samplesPerBlock = 0;
    dsp::ProcessorDuplicator <dsp::IIR::Filter<float>, dsp::IIR::Coefficients <float>> lowPassFilter;

    int writeSampleIndex = 0;



    float lastWetDryValue { 0 };
    float lastTapMixValue { 0 };
    float lastTimeValue { 0 };
    float lastFeedbackValue { 0 };
    float lastCutoffValue { 0 };

public:

    std::atomic<float>* paramWetDry = nullptr;
    std::atomic<float>* paramTapMix = nullptr;
    std::atomic<float>* paramCutoff = nullptr;
    std::atomic<float>* paramFeedback = nullptr;
    std::atomic<float>* paramTime = nullptr;

    std::vector < std::atomic<float>*> paramTaps;
    std::vector < std::atomic<float>*> paramPans;

    int numTaps;

    TapDelayProcessor (int newNumTaps, AudioProcessorValueTreeState& parameters) : lowPassFilter (dsp::IIR::Coefficients<float>::makeLowPass (44100, 20000.0f))
    {
        sampleRate = 0.0;
        samplesPerBlock = 0;
        numTaps = newNumTaps;



        paramWetDry = parameters.getRawParameterValue ("wetDry");
        paramTapMix = parameters.getRawParameterValue ("tapMix");
        paramCutoff = parameters.getRawParameterValue ("cutoff");
        paramFeedback = parameters.getRawParameterValue ("feedback");
        paramTime = parameters.getRawParameterValue ("time");

        for (int i = 0; i < numTaps; i++)
        {
            String tapID ("tap" + String(i));
            String capitalTapID ("Tap" + String(i));
            paramTaps.push_back (parameters.getRawParameterValue (tapID));
        }

        for (int i = 0; i < numTaps; i++)
        {
            String panID ("pan" + String(i));
            String capitalPanID ("Pan" + String(i));
            paramPans.push_back (parameters.getRawParameterValue (panID));
        }
    }

    void prepare (double newSampleRate, int newSamplesPerBlock, int numInputChannels, int numOutputChannels)
    {
        sampleRate = newSampleRate;
        samplesPerBlock = newSamplesPerBlock;


        // ========== Delay Line ==========
        auto bufferSize = maxDelayInSeconds * (sampleRate + samplesPerBlock);
        delayLine.setSize(numInputChannels, bufferSize);
        delayLine.clear();

        feedbackBuffer.setSize(numInputChannels, bufferSize);
        feedbackBuffer.clear();

        // ========== Filter ==========
        dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = samplesPerBlock;
        spec.numChannels = numOutputChannels;

        lowPassFilter.prepare(spec);
        lowPassFilter.reset();



        // ========== Other ==========

    }



    void ProcessData (AudioBuffer<float>& buffer, int numInputChannels, int numOutputChannels, int tapIndex)
    {
        // Process input
        for (int channel = 0; channel < numInputChannels; ++channel)
        {
            // Input to delay line
            fillDelayLine (buffer, channel, false, lastWetDryValue, *paramWetDry);
            // Add feedback
            fillDelayLine (feedbackBuffer, channel, true);
        }

        float tapGainStart, tapGainEnd, totalGainStart, totalGainEnd, HAngle, m_HyperLeftGain, m_HyperRightGain;

        // Process ouput
        for (int channel = 0; channel < numOutputChannels; ++channel)
        {
            tapGainStart = lastTapMixValue * lastWetDryValue;
            tapGainEnd = *paramTapMix * *paramWetDry;
            totalGainStart = (1.0f - lastTapMixValue) * lastWetDryValue;
            totalGainEnd = (1.0f - *paramTapMix) * *paramWetDry;
            // Add input to output
            buffer.copyFromWithRamp (channel, 0, buffer.getReadPointer (channel), buffer.getNumSamples(), 1.0f - lastWetDryValue, 1.0f - *paramWetDry);


            // Read from delay after tap
            readDelayLine (buffer, channel, getReadSampleIndexFromTime (*paramTime * *paramTaps[tapIndex]), true, tapGainStart, tapGainEnd);

            // Read from delay after total
            readDelayLine (buffer, channel, getReadSampleIndexFromTime (*paramTime), true, totalGainStart, totalGainEnd);

            // Write for feedback
            feedbackBuffer.copyFromWithRamp (channel, 0, buffer.getReadPointer (channel), buffer.getNumSamples(), lastWetDryValue, *paramWetDry);



        }

        
        if (*paramPans[tapIndex] < 0)
            *paramPans[tapIndex] = PanLeft::process (abs (*paramPans[tapIndex]) / 90.0f);
        else if (*paramPans[tapIndex] > 0)
            *paramPans[tapIndex] = PanRight::process (*paramPans[tapIndex] / 90.0f);
        

        HAngle = *paramPans[tapIndex] * MathConstants<float>::halfPi / 2.0f;

	    m_HyperLeftGain = (sqrt(2.0f)/2.0f * (cos (HAngle) - sin (HAngle)));
	    m_HyperRightGain = (sqrt(2.0f)/2.0f * (cos (HAngle) + sin (HAngle)));

        buffer.applyGain (0, 0, buffer.getNumSamples(), m_HyperLeftGain);
        buffer.applyGain (1, 0, buffer.getNumSamples(), m_HyperRightGain);
        


        writeSampleIndex += buffer.getNumSamples();
        writeSampleIndex %= delayLine.getNumSamples();

        lastFeedbackValue = *paramFeedback;
        lastTapMixValue = *paramTapMix;
        lastWetDryValue = *paramWetDry;
    }

    // Fill the delay line with samples from audio block
    void fillDelayLine (AudioBuffer<float>& buffer, int channel, bool add, float startGain, float endGain)
    {
        if (writeSampleIndex + buffer.getNumSamples() <= delayLine.getNumSamples())
        {
            if (add)
                delayLine.addFromWithRamp (channel, writeSampleIndex, buffer.getReadPointer (channel), buffer.getNumSamples(), startGain, endGain);
            else
                delayLine.copyFromWithRamp (channel, writeSampleIndex, buffer.getReadPointer (channel), buffer.getNumSamples(), startGain, endGain);
        }
        else
        {
            const auto midPos = delayLine.getNumSamples() - writeSampleIndex;
            if (add)
            {
                delayLine.addFromWithRamp (channel, writeSampleIndex, buffer.getReadPointer (channel), midPos, startGain, endGain);
                delayLine.addFromWithRamp (channel, 0, buffer.getReadPointer (channel, midPos), buffer.getNumSamples() - midPos, startGain, endGain);
            }
            else
            {
                delayLine.copyFromWithRamp (channel, writeSampleIndex, buffer.getReadPointer (channel), midPos, startGain, endGain);
                delayLine.copyFromWithRamp (channel, 0, buffer.getReadPointer (channel, midPos), buffer.getNumSamples() - midPos, startGain, endGain);
            }
        }
    }

    // Read from the delay line at a given read point
    void readDelayLine (AudioBuffer<float>& buffer, int channel, int readSampleIndex, bool add, float startGain, float endGain)
    {
        auto bufferLength = buffer.getNumSamples();
        auto delayLineLength = delayLine.getNumSamples();

        if (readSampleIndex + bufferLength <= delayLineLength)
        {
            if (add)
                buffer.addFromWithRamp (channel, 0, delayLine.getReadPointer (channel, readSampleIndex), bufferLength, startGain, endGain);
            else
                buffer.copyFromWithRamp (channel, 0, delayLine.getReadPointer (channel, readSampleIndex), bufferLength, startGain, endGain);
        }
        else
        {
            const auto midPos = delayLine.getNumSamples() - readSampleIndex;
            if (add)
            {
                buffer.addFromWithRamp (channel, 0, delayLine.getReadPointer (channel, readSampleIndex), midPos, startGain, endGain);
                buffer.addFromWithRamp (channel, midPos, delayLine.getReadPointer (channel), bufferLength - midPos, startGain, endGain);
            }
            else
            {
                buffer.copyFromWithRamp (channel, 0, delayLine.getReadPointer (channel, readSampleIndex), midPos, startGain, endGain);
                buffer.copyFromWithRamp (channel, midPos, delayLine.getReadPointer (channel), bufferLength - midPos, startGain, endGain);
            }
        }
    }

    void applyFilter (AudioBuffer<float>& buffer, float cutoffParam)
    {
        dsp::AudioBlock <float> block (buffer);
        *lowPassFilter.state = *dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, cutoffParam);
        lowPassFilter.process (dsp::ProcessContextReplacing<float>(block));
    }

    // Calculate read point from current write point and a given delay time
    int getReadSampleIndexFromTime (float timeInMilliseconds) const
    {
        // read delayed signal
        int readSampleIndex = roundToInt (writeSampleIndex - (sampleRate * timeInMilliseconds / 1000.0));
        if (readSampleIndex < 0)
            readSampleIndex += delayLine.getNumSamples();

        return readSampleIndex;
    }


    void fillDelayLine (AudioBuffer<float>& buffer, int channel, bool add)
    {
        if (writeSampleIndex + buffer.getNumSamples() <= delayLine.getNumSamples())
        {
            if (add)
                delayLine.addFrom (channel, writeSampleIndex, buffer.getReadPointer (channel), buffer.getNumSamples());
            else
                delayLine.copyFrom (channel, writeSampleIndex, buffer.getReadPointer (channel), buffer.getNumSamples());
        }
        else
        {
            const auto midPos = delayLine.getNumSamples() - writeSampleIndex;
            if (add)
            {
                delayLine.addFrom (channel, writeSampleIndex, buffer.getReadPointer (channel), midPos);
                delayLine.addFrom (channel, 0, buffer.getReadPointer (channel, midPos), buffer.getNumSamples() - midPos);
            }
            else
            {
                delayLine.copyFrom (channel, writeSampleIndex, buffer.getReadPointer (channel), midPos);
                delayLine.copyFrom (channel, 0, buffer.getReadPointer (channel, midPos), buffer.getNumSamples() - midPos);
            }
        }
    }
};

class InfinitapAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    InfinitapAudioProcessor();
    ~InfinitapAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================

    
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout params{
            std::make_unique<AudioParameterFloat>  ("feedback",
                                                    "Feedback",
                                                    NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                0.5f),
            std::make_unique<AudioParameterFloat>  ("time",
                                                    "Time",
                                                    NormalisableRange<float>(0.0f, maxDelayInSeconds * 1000.0f, 1.0f),
                                                    1000.0f),
            std::make_unique<AudioParameterFloat>  ("cutoff",
                                                    "Cutoff",
                                                    NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 20000.0f),
                                                    2000.0f),
            std::make_unique<AudioParameterFloat>  ("tapMix",
                                                    "TapMix",
                                                    NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                                                    0.5f),
            std::make_unique<AudioParameterFloat>  ("wetDry",
                                                    "WetDry",
                                                    NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                                                    0.5f)
        };

        for (int i = 0; i < numTaps; i++)
        {
            String tapID ("tap" + String(i));
            String capitalTapID ("Tap" + String(i));

            params.add (std::make_unique<AudioParameterFloat>(tapID,
            capitalTapID,
            NormalisableRange<float> (0.0f, 1.0f, 0.01f),
            0.5f));

            String panID ("pan" + String(i));
            String capitalPanID ("Pan" + String(i));

            params.add (std::make_unique<AudioParameterFloat>(panID,
            capitalPanID,
            NormalisableRange<float> (-90.0f, 90.0f, 1.0f),
            0.0f));

            String modID ("mod" + String(i));
            String capitalModID ("Mod" + String(i));

            params.add (std::make_unique<AudioParameterFloat>(modID,
            capitalModID,
            NormalisableRange<float> (0.0f, 1.0f, 0.01f),
            0.0f));

        }
        
        return params;
    }
    


    int numTaps = 2;

    juce::AudioBuffer<float> scratchBuffer;
    juce::AudioBuffer<float> resultBuffer;


private:
    float maxDelayInSeconds { 2.0f };

    //==============================================================================
    AudioProcessorValueTreeState parameters;

    //==============================================================================


    std::unique_ptr<TapDelayProcessor> tapDelay;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InfinitapAudioProcessor)

};
