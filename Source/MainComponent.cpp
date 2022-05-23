#include "MainComponent.h"


//==============================================================================
MainComponent::MainComponent()
{
    buttonLookAndFeel.setColour(juce::TextButton::buttonColourId,juce::Colours::black);
    buttonLookAndFeel.setColour(juce::TextButton::buttonOnColourId,juce::Colours::darkgrey);

    juce::Image recordButtonImage=juce::ImageCache::getFromMemory(BinaryData::speaker_png,BinaryData::speaker_pngSize);
    recordbutton.setImages(false, true, true,
                           recordButtonImage,1.0f,{},
                           recordButtonImage,1.0f,{},
                           recordButtonImage,0.5f,{});

    juce::Image editButtonImage=juce::ImageCache::getFromMemory(BinaryData::edit_png,BinaryData::edit_pngSize);
    editButton.setImages(false, true, true,
                           editButtonImage,1.0f,{},
                           editButtonImage,1.0f,{},
                           editButtonImage,0.5f,{});

    juce::Image fileButtonImage=juce::ImageCache::getFromMemory(BinaryData::folder_png,BinaryData::folder_pngSize);
    fileButton.setImages(false, true, true,
                           fileButtonImage,1.0f,{},
                           fileButtonImage,1.0f,{},
                           fileButtonImage,0.5f,{});



    addAndMakeVisible(recordbutton);
    addAndMakeVisible(editButton);
    addAndMakeVisible(fileButton);
    addAndMakeVisible(languageButton);

    addAndMakeVisible(recordbuttonLabel);
    recordbuttonLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(editbuttonLabel);
    editbuttonLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(filebuttonlabel);
    filebuttonlabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(languagelabel);
    languagelabel.setJustificationType(juce::Justification::centred);

    addChildComponent(editComponent);
    addChildComponent(fileComponent);
    addAndMakeVisible(recordComponent);

    recordbutton.setLookAndFeel(&buttonLookAndFeel);
    editButton.setLookAndFeel(&buttonLookAndFeel);
    fileButton.setLookAndFeel(&buttonLookAndFeel);

    recordbutton.addListener(this);
    editButton.addListener(this);
    fileButton.addListener(this);
    languageButton.addListener(this);


}

MainComponent::~MainComponent()
{
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    if(Vrecorder_language==0){
        languageButton.setImages(false, true, true,
                                 zhButtonImage,1.0f,{},
                                 zhButtonImage,1.0f,{},
                                 zhButtonImage,0.5f,{});

        recordbuttonLabel.setText("Record",dontSendNotification);
        editbuttonLabel.setText("Editing",dontSendNotification);
        filebuttonlabel.setText("Audio File",dontSendNotification);
        languagelabel.setText("Language",dontSendNotification);
    }
    if(Vrecorder_language==1){
        languageButton.setImages(false, true, true,
                                 enButtonImage,1.0f,{},
                                 enButtonImage,1.0f,{},
                                 enButtonImage,0.5f,{});

        recordbuttonLabel.setText(juce::String::fromUTF8("录音"),dontSendNotification);
        editbuttonLabel.setText(juce::String::fromUTF8("编辑"),dontSendNotification);
        filebuttonlabel.setText(juce::String::fromUTF8("文件"),dontSendNotification);
        languagelabel.setText(juce::String::fromUTF8("语言"),dontSendNotification);
    }
}

void MainComponent::resized()
{
    auto area = getLocalBounds().removeFromBottom(getLocalBounds().getHeight()/12);

    juce::Rectangle<int> recordbuttonarea=area.removeFromLeft(area.getWidth()/4);
    juce::Rectangle<int> editbuttonarea=area.removeFromLeft(area.getWidth()/3);
    juce::Rectangle<int> filebuttonarea=area.removeFromLeft(area.getWidth()/2);
    auto languagearea=area;

    auto workarea=getLocalBounds().removeFromTop(getLocalBounds().getHeight()*11/12);

    recordbutton.setBounds(recordbuttonarea);
    recordbuttonLabel.setBounds(recordbuttonarea.removeFromBottom(15));
    editButton.setBounds(editbuttonarea);
    editbuttonLabel.setBounds(editbuttonarea.removeFromBottom(15));
    fileButton.setBounds(filebuttonarea);
    filebuttonlabel.setBounds(filebuttonarea.removeFromBottom(15));
    languageButton.setBounds(languagearea);
    languagelabel.setBounds(languagearea.removeFromBottom(15));

    recordComponent.setBounds(workarea);
    editComponent.setBounds(workarea);
    fileComponent.setBounds(workarea);


    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
