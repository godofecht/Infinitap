/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "InfinitapAudioProcessor.h"
#include "newLookAndFeel.h"

//==============================================================================
typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

class TapKnobComponent : public Component
{
public:

    Slider slider { Slider::RotaryHorizontalVerticalDrag, Slider::TextBoxBelow };
    Label label;
    Label display;
    std::unique_ptr<SliderAttachment> attachment;

    TapKnobComponent()
    {
        addAndMakeVisible (slider);
        addAndMakeVisible (label);
    }

    void resized() override
    {
   //     slider.setBounds (getBounds());
    }

};

class InfinitapAudioProcessorEditor : public AudioProcessorEditor
{
public:
    InfinitapAudioProcessorEditor (InfinitapAudioProcessor&, AudioProcessorValueTreeState&);
    ~InfinitapAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    std::vector<std::unique_ptr<TapKnobComponent>> tapSliders;
    std::vector<std::unique_ptr<TapKnobComponent>> panSliders;

private:
    InfinitapAudioProcessor& processor;

    NewLookAndFeel newLookAndFeel;

    AudioProcessorValueTreeState& valueTreeState;

    Slider mFeedbackSlider{ Slider::RotaryHorizontalVerticalDrag, Slider::TextBoxBelow };
    Label mFeedbackLabel;
    Label mFeedbackDisplay;
    std::unique_ptr<SliderAttachment> feedbackAttachment;

    Slider mTimeSlider{ Slider::RotaryHorizontalVerticalDrag, Slider::TextBoxBelow };
    Label mTimeLabel;
    Label mTimeDisplay;
    std::unique_ptr<SliderAttachment> timeAttachment;
    
    Slider mCutoffSlider{ Slider::RotaryHorizontalVerticalDrag, Slider::TextBoxBelow };
    Label mCutoffLabel;
    Label mCutoffDisplay;
    std::unique_ptr<SliderAttachment> cutoffAttachment;

    Slider mTapMixSlider{ Slider::RotaryHorizontalVerticalDrag, Slider::TextBoxBelow };
    Label mTapMixLabel;
    Label mTapMixDisplay;
    std::unique_ptr<SliderAttachment> tapMixAttachment;

    Slider mWetDrySlider{ Slider::RotaryHorizontalVerticalDrag, Slider::TextBoxBelow };
    Label mWetDryLabel;
    Label mWetDryDisplay;
    std::unique_ptr<SliderAttachment> wetDryAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InfinitapAudioProcessorEditor)
};
