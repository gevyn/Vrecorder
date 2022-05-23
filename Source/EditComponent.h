#pragma once

#include <JuceHeader.h>
#include "ThumbnailComp.h"



//=================================================================================================
/*

std::string app_id = "25983029";
std::string api_key = "pukpNcimIwuFOBSP3R1n7CGR";
std::string secret_key = "GyXA2ZV5VoPliAXZMOdDBZEGI493bvI5";

aip::Speech client(app_id, api_key, secret_key);

*/

////==============================================================================
class EditComponent  : public juce::Component,
                       private juce::Button::Listener,
                       private juce::ChangeListener
{
public:
    EditComponent()
    {
        /*
        addAndMakeVisible(regout);
        regout.setMultiLine(true,true);
        regout.setColour(juce::TextEditor::backgroundColourId,juce::Colours::black);
        regout.setReadOnly(true);
        regout.setTextToShowWhenEmpty("Speech Recognition outputs will show here",juce::Colours::grey);
        regout.setFont (juce::Font (18.0f, juce::Font::bold));
         */
        ////============================================================================================================
        thumbnail.reset (new ThumbnailComp (formatManager, transportSource));
        addAndMakeVisible (thumbnail.get());
        thumbnail->addChangeListener (this);
        addAndMakeVisible(filenameLabel);
        filenameLabel.setEditable(true);
        addAndMakeVisible(playbacktimeLabel);
        playbacktimeLabel.setJustificationType(Justification::centredRight);
        ////Button============================================================================================================
        {
            addAndMakeVisible(chooseFileButton);
            chooseFileButton.addListener(this);
            juce::Image chooseFileButtonImage=juce::ImageCache::getFromMemory(BinaryData::openfile_png,BinaryData::openfile_pngSize);
            chooseFileButton.setImages(false, true, true,
                                       chooseFileButtonImage,1.0f,{},
                                       chooseFileButtonImage,1.0f,{},
                                       chooseFileButtonImage,0.5f,{});

            addAndMakeVisible(SaveFileButton);
            SaveFileButton.onClick = [this] { Save(); };
            juce::Image SaveFileButtonImage=juce::ImageCache::getFromMemory(BinaryData::save_png,BinaryData::save_pngSize);
            SaveFileButton.setImages(false, true, true,
                                     SaveFileButtonImage,1.0f,{},
                                     SaveFileButtonImage,1.0f,{},
                                     SaveFileButtonImage,0.5f,{});

            addAndMakeVisible(SaveAsNewButton);
            SaveAsNewButton.onClick = [this] { SaveAsNew(); };
            juce::Image SaveAsNewButtonImage=juce::ImageCache::getFromMemory(BinaryData::saveasnew_png,BinaryData::saveasnew_pngSize);
            SaveAsNewButton.setImages(false, true, true,
                                      SaveAsNewButtonImage,1.0f,{},
                                      SaveAsNewButtonImage,1.0f,{},
                                      SaveAsNewButtonImage,0.5f,{});

            addAndMakeVisible(cutstartButton);
            cutstartButton.onClick = [this] { cutstart(); };
            juce::Image cutstartButtonImage=juce::ImageCache::getFromMemory(BinaryData::cutstart_png,BinaryData::cutstart_pngSize);
            cutstartButton.setImages(false, true, true,
                                      cutstartButtonImage,1.0f,{},
                                      cutstartButtonImage,1.0f,{},
                                      cutstartButtonImage,0.5f,{});

            addAndMakeVisible(startStopButton);
            startStopButton.onClick = [this] { startOrStop(); };
            juce::Image startStopButtonImage=juce::ImageCache::getFromMemory(BinaryData::playstop_png,BinaryData::playstop_pngSize);
            startStopButton.setImages(false, true, true,
                                      startStopButtonImage,1.0f,{},
                                      startStopButtonImage,1.0f,{},
                                      startStopButtonImage,0.5f,{});

            addAndMakeVisible(cutendButton);
            cutendButton.onClick = [this] { cutend(); };
            juce::Image cutendButtonImage=juce::ImageCache::getFromMemory(BinaryData::cutend_png,BinaryData::cutend_pngSize);
            cutendButton.setImages(false, true, true,
                                   cutendButtonImage,1.0f,{},
                                   cutendButtonImage,1.0f,{},
                                   cutendButtonImage,0.5f,{});


            addAndMakeVisible(chooseFileButtonLabel);
            chooseFileButtonLabel.setJustificationType(juce::Justification::centred);
            addAndMakeVisible(SaveFileButtonLabel);
            SaveFileButtonLabel.setJustificationType(juce::Justification::centred);
            addAndMakeVisible(SaveAsNewButtonLabel);
            SaveAsNewButtonLabel.setJustificationType(juce::Justification::centred);
            addAndMakeVisible(cutstartButtonLabel);
            cutstartButtonLabel.setJustificationType(juce::Justification::centred);
            addAndMakeVisible(startStopButtonLabel);
            startStopButtonLabel.setJustificationType(juce::Justification::centred);
            addAndMakeVisible(cutendButtonLabel);
            cutendButtonLabel.setJustificationType(juce::Justification::centred);
        }

        ////audio setup===============================================================================================
        formatManager.registerBasicFormats();
        thread.startThread (3);

        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                     [this] (bool granted)
                                     {
                                         int numInputChannels = granted ? 2 : 0;
                                         audioDeviceManager.initialise (numInputChannels, 2, nullptr, true, {}, nullptr);
                                     });

        audioDeviceManager.addAudioCallback (&audioSourcePlayer);
        audioSourcePlayer.setSource (&transportSource);
        ////==============================================================================================================
        tempdir.deleteRecursively();
        tempdir.createDirectory();

    }

    ~EditComponent() override
    {
        transportSource  .setSource (nullptr);
        audioSourcePlayer.setSource (nullptr);

        audioDeviceManager.removeAudioCallback (&audioSourcePlayer);

        chooseFileButton.removeListener (this);

        thumbnail->removeChangeListener (this);

        tempfile.deleteFile();
        tempdir.deleteFile();

    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::black);
        if(transportSource.getLengthInSeconds()>0) {
            float currentSecond=roundf(100*(transportSource.getCurrentPosition() / transportSource.getLengthInSeconds())*transportSource.getLengthInSeconds())/100;
            float fullSecond= roundf(100*transportSource.getLengthInSeconds())/100;
            playbacktimeLabel.setText(String(currentSecond) + " s / " +
                                      String(fullSecond)  + " s",
                                      juce::dontSendNotification);
        }

        if(Vrecorder_language==0){
            chooseFileButtonLabel.setText("Choose File",dontSendNotification);
            SaveFileButtonLabel.setText("Save File",dontSendNotification);
            SaveAsNewButtonLabel.setText("Save As New",dontSendNotification);
            cutstartButtonLabel.setText("Cut Start",dontSendNotification);
            startStopButtonLabel.setText("Play/Stop",dontSendNotification);
            cutendButtonLabel.setText("Cut End",dontSendNotification);
        }
        if(Vrecorder_language==1){
            chooseFileButtonLabel.setText(juce::String::fromUTF8("选择文件"),dontSendNotification);
            SaveFileButtonLabel.setText(juce::String::fromUTF8("保存"),dontSendNotification);
            SaveAsNewButtonLabel.setText(juce::String::fromUTF8("另存为"),dontSendNotification);
            cutstartButtonLabel.setText(juce::String::fromUTF8("断头"),dontSendNotification);
            startStopButtonLabel.setText(juce::String::fromUTF8("播放/暂停"),dontSendNotification);
            cutendButtonLabel.setText(juce::String::fromUTF8("断尾"),dontSendNotification);
        }

    }

    void resized() override
    {
        //regout.setBounds();
        auto filearea = getLocalBounds().removeFromTop(50);
        auto chooseFileButtonarea=filearea.removeFromLeft(filearea.getWidth()/3);
        auto SaveFileButtonarea=filearea.removeFromLeft(filearea.getWidth()/2);
        auto SaveAsNewButtonarea=filearea;
        chooseFileButton.setBounds(chooseFileButtonarea);
        chooseFileButtonLabel.setBounds(chooseFileButtonarea.removeFromBottom(15));
        SaveFileButton.setBounds(SaveFileButtonarea);
        SaveFileButtonLabel.setBounds(SaveFileButtonarea.removeFromBottom(15));
        SaveAsNewButton.setBounds(SaveAsNewButtonarea);
        SaveAsNewButtonLabel.setBounds(SaveAsNewButtonarea.removeFromBottom(15));

        thumbnail->setBounds (getLocalBounds().removeFromBottom(getLocalBounds().getHeight()-50).removeFromTop(140));
        filenameLabel.setBounds(getLocalBounds().removeFromBottom(getLocalBounds().getHeight()-50).removeFromTop(140).removeFromTop(20));
        playbacktimeLabel.setBounds(getLocalBounds().removeFromBottom(getLocalBounds().getHeight()-50).removeFromTop(140).removeFromBottom(20));


        auto cutarea = getLocalBounds().removeFromBottom(getLocalBounds().getHeight()-200).removeFromTop(60);
        auto cutstartButtonarea =cutarea.removeFromLeft(getLocalBounds().getWidth()/3);
        auto startStopButtonarea = cutarea.removeFromLeft(getLocalBounds().getWidth()/3);
        auto cutendButtonarea = cutarea.removeFromRight(getLocalBounds().getWidth()/3);
        cutstartButton.setBounds(cutstartButtonarea);
        cutstartButtonLabel.setBounds(cutstartButtonarea.removeFromBottom(15));
        startStopButton.setBounds(startStopButtonarea);
        startStopButtonLabel.setBounds(startStopButtonarea.removeFromBottom(15));
        cutendButton.setBounds(cutendButtonarea);
        cutendButtonLabel.setBounds(cutendButtonarea.removeFromBottom(15));

    }




private:
////==============================================================================
    std::unique_ptr<ThumbnailComp> thumbnail;
    juce::Label filenameLabel;
    juce::Label playbacktimeLabel;

    std::unique_ptr<juce::FileChooser> fileChooser;

    juce::AudioDeviceManager audioDeviceManager;
    juce::AudioFormatManager formatManager;
    juce::TimeSliceThread thread  { "audio file preview" };

    juce::AudioSourcePlayer audioSourcePlayer;
    juce::AudioTransportSource transportSource;
    std::unique_ptr<juce::AudioFormatReaderSource> currentAudioFileSource;
    ////creat temp file==============================================================================
    juce::File tempdir=juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("tempdir");
    juce::File tempfile;
    String filename;
    String filenamewithoutend;

    ////==============================================================================
    //juce::TextEditor regout{"speech recognition output:",0};

    ////==============================================================================
    juce::ImageButton chooseFileButton;
    juce::ImageButton SaveFileButton;
    juce::ImageButton SaveAsNewButton;
    juce::ImageButton cutstartButton;
    juce::ImageButton startStopButton;
    juce::ImageButton cutendButton;

    juce::Label chooseFileButtonLabel;
    juce::Label SaveFileButtonLabel;
    juce::Label SaveAsNewButtonLabel;
    juce::Label cutstartButtonLabel;
    juce::Label startStopButtonLabel;
    juce::Label cutendButtonLabel;

    ////==============================================================================
    void loadFileIntoTransport (juce::File file)
    {
        // unload the previous file source and delete it..
        transportSource.stop();
        transportSource.setSource (nullptr);
        currentAudioFileSource.reset();

        juce::AudioFormatReader* reader = nullptr;

        reader = formatManager.createReaderFor (file);

        currentAudioFileSource.reset (new juce::AudioFormatReaderSource (reader, true));

        // ..and plug it into our transport source
        transportSource.setSource (currentAudioFileSource.get(),
                                       32768,  // tells it to buffer this many samples ahead
                                       &thread,                 // this is the background thread to use for reading-ahead
                                       reader->sampleRate);     // allows for sample rate correction

    }

    void showAudioResource (juce::File file)
    {
        //play the audio file
        loadFileIntoTransport(file);
        //show the thumbnail of the audio file
        thumbnail->setFile (file);
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
    ////===================================================================================================
    void Save()
    {
        juce::File::getSpecialLocation(juce::File::userMusicDirectory).getChildFile(filenamewithoutend+".wav").deleteFile();

        filenamewithoutend=filenameLabel.getText();
        tempfile.copyFileTo(juce::File::getSpecialLocation(juce::File::userMusicDirectory).getNonexistentChildFile(filenamewithoutend,".wav"));

    }

    void SaveAsNew()
    {
        //record tempfile after
        filenamewithoutend=filenameLabel.getText();
        tempfile.copyFileTo(juce::File::getSpecialLocation(juce::File::userMusicDirectory).getNonexistentChildFile(filenamewithoutend,".wav"));

        //add temp file to Music

    }

    void cutstart()
    {
        //cut the tempfile
        double CurrentPosition = transportSource.getCurrentPosition()/transportSource.getLengthInSeconds();

        auto reader = formatManager.createReaderFor(tempfile);

        juce::AudioBuffer<float> buffer;
        buffer.clear();
        buffer.setSize((int) reader->numChannels, (int)(reader->lengthInSamples*(1-CurrentPosition)));
        reader->read(&buffer, 0,(int)(reader->lengthInSamples*(1-CurrentPosition)),(int64)(reader->lengthInSamples*CurrentPosition), true, true);

        tempfile.deleteFile();
        tempfile=tempdir.getNonexistentChildFile("temp"+String(CurrentPosition),".wav");


        juce::WavAudioFormat format;
        auto writer = format.createWriterFor(new FileOutputStream(tempfile), reader->sampleRate,reader->numChannels, reader->bitsPerSample,{}, 0);

        if(writer!= nullptr) {
            writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
            //writer->writeFromAudioReader(*reader, (int64)(reader->lengthInSamples * CurrentPosition),(int64)(reader->lengthInSamples * (1 - CurrentPosition)));
            delete writer;
        }
        delete reader;

        showAudioResource(tempfile);

    }

    void cutend()
    {
        //cut the tempfile
        double CurrentPosition = transportSource.getCurrentPosition()/transportSource.getLengthInSeconds();

        auto reader = formatManager.createReaderFor(tempfile);

        juce::AudioBuffer<float> buffer;
        buffer.clear();
        buffer.setSize((int) reader->numChannels, (int)(reader->lengthInSamples*CurrentPosition));
        reader->read(&buffer, 0,(int)(reader->lengthInSamples*CurrentPosition),0, true, true);

        tempfile.deleteFile();
        tempfile=tempdir.getNonexistentChildFile("temp"+String(CurrentPosition),".wav");


        juce::WavAudioFormat format;
        auto writer = format.createWriterFor(new FileOutputStream(tempfile), reader->sampleRate,reader->numChannels, reader->bitsPerSample,{}, 0);

        if(writer!= nullptr) {
            writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
            //writer->writeFromAudioReader(*reader, (int64)(reader->lengthInSamples * CurrentPosition),(int64)(reader->lengthInSamples * (1 - CurrentPosition)));
            delete writer;
        }
        delete reader;

        //thumnail.showAudioResource(temp file)
        showAudioResource(tempfile);
    }
    ////===========================================================================================================
    void buttonClicked (juce::Button* btn) override
    {
        if (btn == &chooseFileButton && fileChooser.get() == nullptr)
        {
            tempfile.deleteFile();
            if (! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::readExternalStorage))
            {
                SafePointer<EditComponent> safeThis (this);
                juce::RuntimePermissions::request (juce::RuntimePermissions::readExternalStorage,
                                             [safeThis] (bool granted) mutable
                                             {
                                                 if (safeThis != nullptr && granted)
                                                     safeThis->buttonClicked (&safeThis->chooseFileButton);
                                             });
                return;
            }

            if (juce::FileChooser::isPlatformDialogAvailable())
            {

                fileChooser.reset (new juce::FileChooser ("Select an audio file...",
                                                          juce::File::getSpecialLocation(juce::File::userMusicDirectory), "*.wav;*.mp3;*.aif"));

                fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                          [this] (const juce::FileChooser& fc) mutable
                                          {
                                              if (fc.getURLResults().size() > 0)
                                              {
                                                  filename=fc.getResult().getFileName();
                                                  filenamewithoutend=fc.getResult().getFileNameWithoutExtension();
                                                  filenameLabel.setText(filenamewithoutend,juce::dontSendNotification);


                                                  tempfile=tempdir.getChildFile("temp-"+fc.getResult().getFileName());
                                                  fc.getResult().copyFileTo(tempfile);
                                                  showAudioResource (tempfile);

                                              }
                                              fileChooser = nullptr;
                                          }, nullptr);
            }
        }
    }


    void changeListenerCallback (juce::ChangeBroadcaster* source) override
    {
        if (source == thumbnail.get())
            showAudioResource (tempfile);

    }


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditComponent)
};


