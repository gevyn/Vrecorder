/*
  ==============================================================================

    FileComponent.cpp
    Created: 12 Apr 2022 3:50:14pm
    Author:  Gevyn

  ==============================================================================
*/

#include <JuceHeader.h>
#include "FileComponent.h"

//==============================================================================
FileComponent::FileComponent()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    filereadingthread.startThread();

    listoshow.setDirectory(juce::File::getSpecialLocation(juce::File::userMusicDirectory ), false, true);

    fileListComponent.setColour(juce::FileListComponent::backgroundColourId,juce::Colours::black);
    fileListComponent.setColour(juce::ListBox::textColourId,juce::Colours::white);
    fileListComponent.setRowHeight(40);
    fileListComponent.setMultipleSelectionEnabled(false);

    listoshow.addChangeListener(this);
    fileListComponent.addListener(this);


    addAndMakeVisible(fileListComponent);

    //========================================================================================

    thumbnail.reset (new ThumbnailComp (formatManager, transportSource));

    addAndMakeVisible (thumbnail.get());
    thumbnail->addChangeListener (this);

    addAndMakeVisible (startStopButton);
    juce::Image startStopButtonImage=juce::ImageCache::getFromMemory(BinaryData::playstop_png,BinaryData::playstop_pngSize);
    startStopButton.setImages(false, true, true,
                              startStopButtonImage,1.0f,{},
                              startStopButtonImage,1.0f,{},
                              startStopButtonImage,0.5f,{});

    startStopButton.onClick = [this] { startOrStop(); };

    addAndMakeVisible (deleteButton);
    juce::Image deleteButtonImage=juce::ImageCache::getFromMemory(BinaryData::delete_png,BinaryData::delete_pngSize);
    deleteButton.setImages(false, true, true,
                           deleteButtonImage,1.0f,{},
                           deleteButtonImage,1.0f,{},
                           deleteButtonImage,0.5f,{});
    deleteButton.onClick = [this] { deletefile(); };

    addAndMakeVisible (shareButton);
    juce::Image shareButtonImage=juce::ImageCache::getFromMemory(BinaryData::share_png,BinaryData::share_pngSize);
    shareButton.setImages(false, true, true,
                          shareButtonImage,1.0f,{},
                          shareButtonImage,1.0f,{},
                          shareButtonImage,0.5f,{});
    shareButton.onClick = [this] { sharefile(); };


    addAndMakeVisible(startStopLabel);
    startStopLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(deleteLabel);
    deleteLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(shareLabel);
    shareLabel.setJustificationType(juce::Justification::centred);

    formatManager.registerBasicFormats();
    thread.startThread (3);
#ifndef JUCE_DEMO_RUNNER
    juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                       [this] (bool granted)
                                       {
                                           int numInputChannels = granted ? 2 : 0;
                                           audioDeviceManager.initialise (numInputChannels, 2, nullptr, true, {}, nullptr);
                                       });
#endif

    audioDeviceManager.addAudioCallback (&audioSourcePlayer);
    audioSourcePlayer.setSource (&transportSource);

}

FileComponent::~FileComponent()
{
    filereadingthread.stopThread(1);
    transportSource  .setSource (nullptr);
    audioSourcePlayer.setSource (nullptr);

    audioDeviceManager.removeAudioCallback (&audioSourcePlayer);

    thumbnail->removeChangeListener (this);


}

void FileComponent::paint (juce::Graphics& g)
{
    if(Vrecorder_language==0){
        startStopLabel.setText("Play/Stop",dontSendNotification);
        deleteLabel.setText("Delete",dontSendNotification);
        shareLabel.setText("Share",dontSendNotification);
    }
    if(Vrecorder_language==1){

        startStopLabel.setText(juce::String::fromUTF8("播放/停止"),dontSendNotification);
        deleteLabel.setText(juce::String::fromUTF8("删除"),dontSendNotification);
        shareLabel.setText(juce::String::fromUTF8("分享"),dontSendNotification);
    }

}

void FileComponent::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    auto area=getLocalBounds();
    fileListComponent.setBounds(area.removeFromTop(area.getHeight()*9/10));

    auto startStopButtonarea=area.removeFromLeft(getLocalBounds().getWidth()/5);
    auto deleteButtonarea=area.removeFromRight(getLocalBounds().getWidth()/7);
    auto shareButtonarea=area.removeFromRight(getLocalBounds().getWidth()/7);

    startStopButton.setBounds(startStopButtonarea);
    startStopLabel.setBounds(startStopButtonarea.removeFromBottom(15));
    deleteButton.setBounds(deleteButtonarea);
    deleteLabel.setBounds(deleteButtonarea.removeFromBottom(15));
    shareButton.setBounds(shareButtonarea);
    shareLabel.setBounds(shareButtonarea.removeFromBottom(15));

    thumbnail->setBounds(area);

}

void FileComponent::refresh() {
    listoshow.refresh();
    fileListComponent.updateContent();
}


