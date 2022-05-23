/*
  ==============================================================================

    FileComponent.h
    Created: 12 Apr 2022 3:50:14pm
    Author:  Gevyn

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ThumbnailComp.h"

////==============================================================================
class FileComponent  : public juce::Component,
                        private juce::Button::Listener,
                        private juce::ChangeListener,
                        private juce::FileBrowserListener
{
public:
    FileComponent();
    ~FileComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void refresh();

private:

    juce::TimeSliceThread filereadingthread{"filereadingthread"};
    juce::WildcardFileFilter wildcardFileFilter{"*.wav;", "*.wav", "recordings"};
    juce::DirectoryContentsList listoshow{&wildcardFileFilter,filereadingthread};

    juce::FileListComponent fileListComponent{listoshow};
//===========================================================================================

#ifndef JUCE_DEMO_RUNNER
    juce::AudioDeviceManager audioDeviceManager;
#else
    AudioDeviceManager& audioDeviceManager { getSharedAudioDeviceManager (0, 2) };
#endif
    juce::AudioFormatManager formatManager;
    juce::TimeSliceThread thread  { "audio file preview" };

    juce::URL currentAudioFile;
    juce::AudioSourcePlayer audioSourcePlayer;
    juce::AudioTransportSource transportSource;
    std::unique_ptr<juce::AudioFormatReaderSource> currentAudioFileSource;

    std::unique_ptr<ThumbnailComp> thumbnail;

    juce::ImageButton startStopButton;
    juce::ImageButton deleteButton;
    juce::ImageButton shareButton;

    juce::Label startStopLabel;
    juce::Label deleteLabel;
    juce::Label shareLabel;

//================================================================================================================
    void showAudioResource (juce::URL resource)
    {
        if (loadURLIntoTransport (resource))
            currentAudioFile = std::move (resource);

        thumbnail->setFile(currentAudioFile.getLocalFile());
    }

    bool loadURLIntoTransport (const juce::URL& audioURL)
    {
        // unload the previous file source and delete it..
        transportSource.stop();
        transportSource.setSource (nullptr);
        currentAudioFileSource.reset();

        juce::AudioFormatReader* reader = nullptr;

#if ! JUCE_IOS
        if (audioURL.isLocalFile())
        {
            reader = formatManager.createReaderFor (audioURL.getLocalFile());
        }
        else
#endif
        {
            if (reader == nullptr)
                reader = formatManager.createReaderFor (audioURL.createInputStream (juce::URL::InputStreamOptions (juce::URL::ParameterHandling::inAddress)));
        }

        if (reader != nullptr)
        {
            currentAudioFileSource.reset (new juce::AudioFormatReaderSource (reader, true));

            // ..and plug it into our transport source
            transportSource.setSource (currentAudioFileSource.get(),
                                       32768,                   // tells it to buffer this many samples ahead
                                       &thread,                 // this is the background thread to use for reading-ahead
                                       reader->sampleRate);     // allows for sample rate correction

            return true;
        }

        return false;
    }

    void startOrStop()
    {
        if (transportSource.isPlaying())
        {
            transportSource.stop();
        }
        else
        {
            transportSource.setPosition (transportSource.getCurrentPosition());
            transportSource.start();
        }
    }

    void deletefile()
    {
        fileListComponent.getSelectedFile().deleteFile();
        refresh();
        thumbnail->setFile(juce::File());
    }

    void sharefile()
    {
        SafePointer<FileComponent> safeThis (this);
        juce::File fileToShare = fileListComponent.getSelectedFile();

        juce::ContentSharer::getInstance()->shareFiles (juce::Array<juce::URL> ({juce::URL (fileToShare)}),
                                                        [safeThis, fileToShare] (bool success, const juce::String& error)
                                                        {
                                                            if (fileToShare.existsAsFile())
                                                                fileToShare.deleteFile();

                                                            if (! success && error.isNotEmpty())
                                                                juce::NativeMessageBox::showAsync (juce::MessageBoxOptions()
                                                                                                           .withIconType (juce::MessageBoxIconType::WarningIcon)
                                                                                                           .withTitle ("Sharing Error")
                                                                                                           .withMessage (error),
                                                                                                   nullptr);
                                                        });
    }

    void changeListenerCallback (juce::ChangeBroadcaster* source) override
    {

    }

    void selectionChanged() override
    {
        showAudioResource (juce::URL (fileListComponent.getSelectedFile()));
    }

    void fileClicked (const File& file, const MouseEvent& e) override{}

    void fileDoubleClicked (const File& file) override
    {

    }

    void browserRootChanged (const File& newRoot) override{
        listoshow.setDirectory(newRoot, false, true);
    }

    void buttonClicked (juce::Button* btn) override
    {
        //
    }


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileComponent)
};

////==============================================================================
