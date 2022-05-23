/*
  ==============================================================================

    RecordComponent.h
    Created: 11 Apr 2022 4:03:59pm
    Author:  Gevyn

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "language.h"



////==============================================================================
/** This component scrolls a continuous waveform showing the audio that's
   coming into whatever audio inputs this object is connected to.
*/
class LiveScrollingAudioDisplay  : public juce::AudioVisualiserComponent,
                                   public juce::AudioIODeviceCallback
{
public:
    LiveScrollingAudioDisplay()  : AudioVisualiserComponent (1)
    {
        setSamplesPerBlock (256);
        setBufferSize (1024);
    }

    //==============================================================================
    void audioDeviceAboutToStart (juce::AudioIODevice*) override
    {
        clear();
    }

    void audioDeviceStopped() override
    {
        clear();
    }

    void audioDeviceIOCallback (const float** inputChannelData, int numInputChannels,
                                float** outputChannelData, int numOutputChannels,
                                int numberOfSamples) override
    {
        for (int i = 0; i < numberOfSamples; ++i)
        {
            float inputSample = 0;

            for (int chan = 0; chan < numInputChannels; ++chan)
                if (const float* inputChannel = inputChannelData[chan])
                    inputSample += inputChannel[i];  // find the sum of all the channels

            inputSample *= 2.0f; // boost the level to make it more easily visible.

            pushSample (&inputSample, 1);
        }

        // We need to clear the output buffers before returning, in case they're full of junk..
        for (int j = 0; j < numOutputChannels; ++j)
            if (float* outputChannel = outputChannelData[j])
                juce::zeromem (outputChannel, (size_t) numberOfSamples * sizeof (float));
    }
};

////==============================================================================
/** A simple class that acts as an AudioIODeviceCallback and writes the
    incoming audio data to a WAV file.
*/
class AudioRecorder  : public juce::AudioIODeviceCallback
{
public:
    AudioRecorder (juce::AudioThumbnail& thumbnailToUpdate)
            : thumbnail (thumbnailToUpdate)
    {
        backgroundThread.startThread();
    }

    ~AudioRecorder() override
    {
        stop();
    }

    //==============================================================================
    void startRecording (const File& file)
    {
        stop();

        if (sampleRate > 0)
        {
            // Create an OutputStream to write to our destination file...
            file.deleteFile();

            if (auto fileStream = std::unique_ptr<FileOutputStream> (file.createOutputStream()))
            {
                // Now create a WAV writer object that writes to our output stream...
                WavAudioFormat wavFormat;

                if (auto writer = wavFormat.createWriterFor (fileStream.get(), sampleRate, 1, 16, {}, 0))
                {
                    fileStream.release(); // (passes responsibility for deleting the stream to the writer object that is now using it)

                    // Now we'll create one of these helper objects which will act as a FIFO buffer, and will
                    // write the data to disk on our background thread.
                    threadedWriter.reset (new AudioFormatWriter::ThreadedWriter (writer, backgroundThread, 32768));

                    // Reset our recording thumbnail
                    thumbnail.reset (writer->getNumChannels(), writer->getSampleRate());
                    nextSampleNum = 0;

                    // And now, swap over our active writer pointer so that the audio callback will start using it..
                    const ScopedLock sl (writerLock);
                    activeWriter = threadedWriter.get();
                }
            }
        }
    }

    void stop()
    {
        // First, clear this pointer to stop the audio callback from using our writer object..
        {
            const juce::ScopedLock sl (writerLock);
            activeWriter = nullptr;
        }
        // Now we can delete the writer object. It's done in this order because the deletion could
        // take a little time while remaining data gets flushed to disk, so it's best to avoid blocking
        // the audio callback while this happens.
        threadedWriter.reset();
    }

    bool isRecording() const
    {
        return activeWriter.load() != nullptr;
    }

    //==============================================================================
    void audioDeviceAboutToStart (juce::AudioIODevice* device) override
    {
        sampleRate = device->getCurrentSampleRate();
    }

    void audioDeviceStopped() override
    {
        sampleRate = 0;
    }

    void audioDeviceIOCallback (const float** inputChannelData, int numInputChannels,
                                float** outputChannelData, int numOutputChannels,
                                int numSamples) override
    {
        const juce::ScopedLock sl (writerLock);

        if (activeWriter.load() != nullptr && numInputChannels >= thumbnail.getNumChannels())
        {
            activeWriter.load()->write (inputChannelData, numSamples);

            // Create an AudioBuffer to wrap our incoming data, note that this does no allocations or copies, it simply references our input data
            juce::AudioBuffer<float> buffer (const_cast<float**> (inputChannelData), thumbnail.getNumChannels(), numSamples);
            thumbnail.addBlock (nextSampleNum, buffer, 0, numSamples);
            nextSampleNum += numSamples;
        }

        // We need to clear the output buffers, in case they're full of junk..
        for (int i = 0; i < numOutputChannels; ++i)
            if (outputChannelData[i] != nullptr)
                juce::FloatVectorOperations::clear (outputChannelData[i], numSamples);
    }

private:
    juce::AudioThumbnail& thumbnail;
    juce::TimeSliceThread backgroundThread { "Audio Recorder Thread" }; // the thread that will write our audio data to disk
    std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> threadedWriter; // the FIFO used to buffer the incoming data
    double sampleRate = 0.0;
    juce::int64 nextSampleNum = 0;

    juce::CriticalSection writerLock;
    std::atomic<juce::AudioFormatWriter::ThreadedWriter*> activeWriter { nullptr };
};


////======================================================================================================================================
class RecordingThumbnail  : public juce::Component,
                            private juce::ChangeListener
{
public:
    float endTime;
    RecordingThumbnail()
    {
        formatManager.registerBasicFormats();
        thumbnail.addChangeListener (this);
    }

    ~RecordingThumbnail() override
    {
        thumbnail.removeChangeListener (this);
    }

    juce::AudioThumbnail& getAudioThumbnail()     { return thumbnail; }

    void setDisplayFullThumbnail (bool displayFull)
    {
        displayFullThumb = displayFull;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::black);
        g.setColour (juce::Colours::white);

        if (thumbnail.getTotalLength() > 0.0)
        {
            endTime = thumbnail.getTotalLength();

            thumbnail.drawChannels (g, getLocalBounds(), 0.0, endTime, 1.0f);
        }
        else
        {
            //g.setFont (14.0f);
            //g.drawFittedText ("(No file recorded)", getLocalBounds(), juce::Justification::centred, 2);
        }
    }

private:
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache  { 10 };
    juce::AudioThumbnail thumbnail            { 512, formatManager, thumbnailCache };

    bool displayFullThumb = false;

    void changeListenerCallback (juce::ChangeBroadcaster* source) override
    {
        if (source == &thumbnail)
            repaint();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecordingThumbnail)
};

////======================================================================================================================================
class AudioRecordingComponent  : public juce::Component
{
public:
    AudioRecordingComponent()
    {
        addAndMakeVisible(TimerText);
        TimerText.setVisible(false);
        TimerText.setJustificationType(juce::Justification::centred);
        TimerText.setFont(juce::Font(60,0));



        setOpaque (true);
        addAndMakeVisible (liveAudioScroller);
        addAndMakeVisible (recordingThumbnail);



        addAndMakeVisible (recordButton);
        recordButton.setImages(false, true, true,
                             recordButtonImage,1.0f,{},
                             recordButtonImage,1.0f,{},
                             recordButtonImage,0.5f,{});

        recordButton.onClick = [this]
        {
            if (recorder.isRecording())
                stopRecording();
            else
                startRecording();
        };

#ifndef JUCE_DEMO_RUNNER
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [this] (bool granted)
                                           {
                                               int numInputChannels = granted ? 2 : 0;
                                               audioDeviceManager.initialise (numInputChannels, 2, nullptr, true, {}, nullptr);
                                           });
#endif

        audioDeviceManager.addAudioCallback (&liveAudioScroller);
        audioDeviceManager.addAudioCallback (&recorder);

    }

    ~AudioRecordingComponent() override
    {
        audioDeviceManager.removeAudioCallback (&recorder);
        audioDeviceManager.removeAudioCallback (&liveAudioScroller);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::black);

        auto currentTimer = roundf(recordingThumbnail.endTime*100)/100;

        TimerText.setText(String(currentTimer)+" S",dontSendNotification);
    }

    void resized() override
    {
        auto area = getLocalBounds();

        liveAudioScroller .setBounds (area.removeFromTop(200));

        TimerText.setBounds(area.removeFromTop(70));

        recordingThumbnail.setBounds (area.removeFromTop(200));

        recordButton      .setBounds (area.removeFromBottom(140).removeFromTop(120)
                                                    .removeFromLeft(getLocalBounds().getWidth()*2/3)
                                                    .removeFromRight(getLocalBounds().getWidth()*1/3));

    }

private:
    // if this PIP is running inside the demo runner, we'll use the shared device manager instead
    juce::AudioDeviceManager audioDeviceManager;
    LiveScrollingAudioDisplay liveAudioScroller;
    RecordingThumbnail recordingThumbnail;
    AudioRecorder recorder  { recordingThumbnail.getAudioThumbnail() };

    juce::Label TimerText;


    juce::ImageButton recordButton;
    juce::Image recordButtonImage=juce::ImageCache::getFromMemory(BinaryData::play_png,BinaryData::play_pngSize);
    juce::Image stopButtonImage=juce::ImageCache::getFromMemory(BinaryData::stop_png,BinaryData::stop_pngSize);

    juce::File lastRecording;





    void startRecording()
    {
        if (! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::writeExternalStorage))
        {
            SafePointer<AudioRecordingComponent> safeThis (this);

            juce::RuntimePermissions::request (juce::RuntimePermissions::writeExternalStorage,
                                               [safeThis] (bool granted) mutable
                                               {
                                                   if (granted)
                                                       safeThis->startRecording();
                                               });
            return;
        }

        juce::File recordingDir = juce::File::getSpecialLocation(juce::File::userMusicDirectory);
        auto time=juce::Time::getCurrentTime();
        lastRecording = recordingDir.getNonexistentChildFile ("Record"+time.toString(true, false,false,
                                                                                         false), ".wav");

        recorder.startRecording (lastRecording);

        recordButton.setImages(false, true, true,
                               stopButtonImage,1.0f,{},
                               stopButtonImage,1.0f,{},
                               stopButtonImage,0.5f,{});

        TimerText.setVisible(true);


        recordingThumbnail.setDisplayFullThumbnail (false);
    }

    void stopRecording()
    {
        recorder.stop();

        lastRecording = juce::File();

        recordButton.setImages(false, true, true,
                               recordButtonImage,1.0f,{},
                               recordButtonImage,1.0f,{},
                               recordButtonImage,0.5f,{});

        TimerText.setVisible(false);

        recordingThumbnail.setDisplayFullThumbnail (true);
    }
};


////==========================================================================================================================================
class RecordComponent  : public juce::Component
{
public:
    RecordComponent();
    ~RecordComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AudioRecordingComponent audioRecordingComponent;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecordComponent)
};




