/*
  ==============================================================================

    newLookAndFeel.h
    Created: 24 May 2020 8:31:16am
    Author:  tom-c

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class NewLookAndFeel : public LookAndFeel_V4
{
public:

    Colour textColour = Colours::white;
    Colour backgroundColour = Colours::darkgrey;
    Colour dialColour = Colours::slategrey;
    Colour dotColour = Colours::white;

    NewLookAndFeel()
    {
        setColour(Slider::textBoxOutlineColourId, Colours::transparentWhite);
        setColour(Slider::textBoxTextColourId, textColour);
        setColour(Label::textColourId, textColour);
        setColour(ResizableWindow::backgroundColourId, backgroundColour);
    }

    void drawRotarySlider(Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider) override
    {
        // Dial dims
        auto radius = jmin(width / 2, height / 2) - 4.0f;
        auto centreX = x + width * 0.5f;
        auto centreY = y + height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // Dot dims
        auto dotRadius = radius * 0.05f;
        auto dotW = dotRadius * 2.0f;
        Rectangle<float> area = Rectangle<float>(-1.0f, -radius * 0.8f, dotW, dotW);

        // dial
        g.setColour(dialColour);
        g.fillEllipse(rx, ry, rw, rw);

        // dot
        Path dot;
        dot.addEllipse(area);

        dot.applyTransform(AffineTransform::rotation(angle).translated(centreX, centreY));

        g.setColour(dotColour);
        g.fillPath(dot);

        // border
        g.drawRoundedRectangle(Rectangle<float>(x, y, width, height), 10.0f, 2.0f);
    }
};