/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

/*
/**
 * Copyright (C) Quilio - All Rights Reserved
 * 
 * This source code is protected under international copyright law.  All rights
 * reserved and protected by the copyright holders.
 * This file is confidential and only available to authorized individuals with the
 * permission of the copyright holders.  If you encounter this file and do not have
 * permission, please contact the copyright holders and delete this file.
 */

/** 
 * Copyright (C) {Company} - All Rights Reserved
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by {Author} <{Email}>, {Date}
 */

#include "InfinitapAudioProcessor.h"
#include "InfinitapAudioProcessorEditor.h"

//==============================================================================
InfinitapAudioProcessor::InfinitapAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", AudioChannelSet::stereo(), true)
#endif
    ),
    parameters (*this, nullptr, Identifier ("Infinitap"), createParameterLayout())
#endif
{

    tapDelay = std::make_unique<TapDelayProcessor> (numTaps, parameters);
}

InfinitapAudioProcessor::~InfinitapAudioProcessor()
{
}

//==============================================================================
const String InfinitapAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool InfinitapAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool InfinitapAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool InfinitapAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double InfinitapAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int InfinitapAudioProcessor::getNumPrograms()
{
    return 1;
}

int InfinitapAudioProcessor::getCurrentProgram()
{
    return 0;
}

void InfinitapAudioProcessor::setCurrentProgram (int index)
{
}

const String InfinitapAudioProcessor::getProgramName (int index)
{
    return {};
}

void InfinitapAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void InfinitapAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    tapDelay->prepare (sampleRate, samplesPerBlock, 2, 2);
}

void InfinitapAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool InfinitapAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void InfinitapAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    resultBuffer.clear();

    resultBuffer.setSize (buffer.getNumChannels(), buffer.getNumSamples());

    for (int i = 0; i < numTaps; ++i)
    {
        scratchBuffer.clear();
        scratchBuffer.setSize (buffer.getNumChannels(), buffer.getNumSamples());
        scratchBuffer.copyFrom (0, 0, buffer.getWritePointer(0), buffer.getNumSamples());
        scratchBuffer.copyFrom (1, 0, buffer.getWritePointer(1), buffer.getNumSamples());

        tapDelay->ProcessData (scratchBuffer, totalNumInputChannels, totalNumOutputChannels, i);
        resultBuffer.addFrom (0, 0, scratchBuffer.getWritePointer(0), scratchBuffer.getNumSamples(), 1.0f / (float) (numTaps));
        resultBuffer.addFrom (1, 0, scratchBuffer.getWritePointer(1), scratchBuffer.getNumSamples(), 1.0f / (float) (numTaps));
    }

    buffer.copyFrom (0, 0, resultBuffer.getWritePointer(0), resultBuffer.getNumSamples());
    buffer.copyFrom (1, 0, resultBuffer.getWritePointer(1), resultBuffer.getNumSamples());

    auto paramCutoff = parameters.getRawParameterValue ("cutoff");
    tapDelay->applyFilter (buffer, *paramCutoff);
}

//==============================================================================
bool InfinitapAudioProcessor::hasEditor() const
{
    return true;
}

AudioProcessorEditor* InfinitapAudioProcessor::createEditor()
{

    return new InfinitapAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void InfinitapAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void InfinitapAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(ValueTree::fromXml(*xmlState));
}

//==============================================================================
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new InfinitapAudioProcessor();
}