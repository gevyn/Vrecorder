/*
  ==============================================================================

    RecordComponent.cpp
    Created: 11 Apr 2022 4:03:59pm
    Author:  Gevyn

  ==============================================================================
*/

#include <JuceHeader.h>
#include "RecordComponent.h"

//==============================================================================
RecordComponent::RecordComponent()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    addAndMakeVisible(audioRecordingComponent);
}

RecordComponent::~RecordComponent()
{
}

void RecordComponent::paint (juce::Graphics& g)
{
}

void RecordComponent::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    audioRecordingComponent.setBounds(getLocalBounds());
}
