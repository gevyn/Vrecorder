/*
  ==============================================================================

    ThumbnailComp.h
    Created: 20 Apr 2022 7:53:54pm
    Author:  Gevyn

  ==============================================================================
*/


#pragma once
#include "language.h"

class ThumbnailComp  : public juce::Component,
                       public juce::ChangeListener,
                       public juce::FileDragAndDropTarget,
                       public juce::ChangeBroadcaster,
                       private juce::Timer
{
public:
    ThumbnailComp (juce::AudioFormatManager& formatManager,
                   juce::AudioTransportSource& source)
            : transportSource (source),
              thumbnail (512, formatManager, thumbnailCache)
    {
        thumbnail.addChangeListener (this);
        currentPositionMarker.setFill (juce::Colours::white.withAlpha (0.85f));
        addAndMakeVisible (currentPositionMarker);
    }

    ~ThumbnailComp() override
    {
        thumbnail.removeChangeListener (this);
    }

    void setFile (const juce::File file)
    {

        juce::InputSource* inputSource = nullptr;

        inputSource = new juce::FileInputSource (file);

        if (inputSource != nullptr)
        {
            thumbnail.setSource (inputSource);

            juce::Range<double> newRange (0.0, thumbnail.getTotalLength());
            setRange (newRange);

            startTimerHz (40);
        }
    }

    juce::URL getLastDroppedFile() const noexcept { return lastFileDropped; }

    void setRange (juce::Range<double> newRange)
    {
        visibleRange = newRange;
        updateCursorPosition();
        repaint();
    }

    void setFollowsTransport (bool shouldFollow)
    {
        isFollowingTransport = shouldFollow;
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::black);
        g.setColour (juce::Colours::white);

        if (thumbnail.getTotalLength() > 0.0)
        {
            auto thumbArea = getLocalBounds();

            thumbnail.drawChannels (g, thumbArea.reduced (2),
                                    visibleRange.getStart(), visibleRange.getEnd(), 1.0f);
        }
        else
        {
            g.setFont (15.0f);
            if(Vrecorder_language==0){
                g.drawFittedText ("(No audio file selected)", getLocalBounds(), juce::Justification::centred, 2);

            }
            if(Vrecorder_language==1){
                g.drawFittedText (juce::String::fromUTF8("(无音频文件选中)"), getLocalBounds(), juce::Justification::centred, 2);


            }
        }
    }

    void resized() override
    {

    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        // this method is called by the thumbnail when it has changed, so we should repaint it..
        repaint();
    }

    bool isInterestedInFileDrag (const juce::StringArray& /*files*/) override
    {
        return true;
    }

    void filesDropped (const juce::StringArray& files, int /*x*/, int /*y*/) override
    {
        lastFileDropped = juce::URL (juce::File (files[0]));
        sendChangeMessage();
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        mouseDrag (e);
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (canMoveTransport())
            transportSource.setPosition (juce::jmax (0.0, xToTime ((float) e.x)));
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        //transportSource.start();
    }

    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& wheel) override
    {
        if (thumbnail.getTotalLength() > 0.0)
        {
            auto newStart = visibleRange.getStart() - wheel.deltaX * (visibleRange.getLength()) / 10.0;
            newStart = juce::jlimit (0.0, juce::jmax (0.0, thumbnail.getTotalLength() - (visibleRange.getLength())), newStart);

            if (canMoveTransport())
                setRange ({ newStart, newStart + visibleRange.getLength() });

            repaint();
        }
    }

    juce::AudioThumbnail& getAudioThumbnail()     { return thumbnail; }


private:
    juce::AudioTransportSource& transportSource;

    juce::AudioThumbnailCache thumbnailCache  { 5 };
    juce::AudioThumbnail thumbnail;
    juce::Range<double> visibleRange;
    bool isFollowingTransport = false;
    juce::URL lastFileDropped;

    juce::DrawableRectangle currentPositionMarker;

////================================================================================================


    float timeToX (const double time) const
    {
        if (visibleRange.getLength() <= 0)
            return 0;

        return (float) getWidth() * (float) ((time - visibleRange.getStart()) / visibleRange.getLength());
    }

    double xToTime (const float x) const
    {
        return (x / (float) getWidth()) * (visibleRange.getLength()) + visibleRange.getStart();
    }

    bool canMoveTransport() const noexcept
    {
        return ! (isFollowingTransport && transportSource.isPlaying());
    }

    void timerCallback() override
    {
        if (canMoveTransport())
            updateCursorPosition();
        else
            setRange (visibleRange.movedToStartAt (transportSource.getCurrentPosition() - (visibleRange.getLength() / 2.0)));
    }

    void updateCursorPosition()
    {
        currentPositionMarker.setVisible (true);

        currentPositionMarker.setRectangle (juce::Rectangle<float> (timeToX (transportSource.getCurrentPosition()) - 0.75f, 0,
                                                                    1.5f, (float) (getHeight())));
    }
};

