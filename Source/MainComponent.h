#pragma once

#include <JuceHeader.h>
#include "RecordComponent.h"
#include "EditComponent.h"
#include "FileComponent.h"
#include "BinaryData.h"
#include "language.h"



//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/

class MainComponent  : public juce::Component,
                            juce::Button::Listener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void buttonClicked(juce::Button* button) override
    {
        if(button==&recordbutton){
            recordComponent.setVisible(true);
            editComponent.setVisible(false);
            fileComponent.setVisible(false);
        }
        else if(button==&editButton){
            editComponent.setVisible(true);
            fileComponent.setVisible(false);
            recordComponent.setVisible(false);
        }
        else if(button==&fileButton){
            fileComponent.refresh();
            fileComponent.setVisible(true);
            recordComponent.setVisible(false);
            editComponent.setVisible(false);
        }
        else if(button==&languageButton){
            if(Vrecorder_language==0) {
                Vrecorder_language = 1;
            }
            else if(Vrecorder_language==1) {
                Vrecorder_language = 0;
            }
        }
    }

private:
    //==============================================================================
    // Your private member variables go here...
    juce::LookAndFeel_V4 buttonLookAndFeel;

    juce::Image zhButtonImage=juce::ImageCache::getFromMemory(BinaryData::zh_png,BinaryData::zh_pngSize);
    juce::Image enButtonImage=juce::ImageCache::getFromMemory(BinaryData::en_png,BinaryData::en_pngSize);

    juce::ImageButton recordbutton;
    juce::ImageButton editButton;
    juce::ImageButton fileButton;
    juce::ImageButton languageButton;

    juce::Label recordbuttonLabel;
    juce::Label editbuttonLabel;
    juce::Label filebuttonlabel;
    juce::Label languagelabel;

    RecordComponent recordComponent;
    EditComponent editComponent;
    FileComponent fileComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
