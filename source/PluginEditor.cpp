#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (p), processorRef (p),
      drumSequencerPanel (p.getDrumSequencer()),
      modulationMatrixPanel (p, p.getModulationMatrix()),
      samplerPanel (p, p.getSamplerEngine()),
      proSequencerPanel (p.getProSequencer()),
      arpeggiatorPanel (p.getArpeggiator()),
      synthPanel (p),
      effectsPanel (p),
      macroPanel (p),
      granularPanel (p),
      tabs (juce::TabbedButtonBar::TabsAtTop),
      midiLearnButton ("MIDI Learn"),
      midiLearnLabel ("", ""),
      presetBrowserPanel (p)
{
    setupTabs();
    setupMidiLearnButton();

    presetsButton.addListener (this);
    addAndMakeVisible (presetsButton);
    addChildComponent (presetBrowserPanel);

    helpButton.setClickingTogglesState (true);
    helpButton.setToggleState (true, juce::dontSendNotification);
    helpButton.onClick = [this]
    {
        tooltipWindow.setMillisecondsBeforeTipAppears (
            helpButton.getToggleState() ? 700 : 99999999);
    };
    addAndMakeVisible (helpButton);

    addAndMakeVisible (tabs);
    addAndMakeVisible (midiLearnButton);
    addAndMakeVisible (midiLearnLabel);
    addAndMakeVisible (paramSelector);
    setSize (1200, 800);
}

PluginEditor::~PluginEditor()
{
}

void PluginEditor::setupTabs()
{
    tabs.addTab ("Synth",      juce::Colours::darkgrey, &synthPanel,            false);
    tabs.addTab ("Drums",      juce::Colours::darkgrey, &drumSequencerPanel,    false);
    tabs.addTab ("Modulation", juce::Colours::darkgrey, &modulationMatrixPanel, false);
    tabs.addTab ("Sampler",    juce::Colours::darkgrey, &samplerPanel,          false);
    tabs.addTab ("Sequencer",  juce::Colours::darkgrey, &proSequencerPanel,     false);
    tabs.addTab ("Arp",        juce::Colours::darkgrey, &arpeggiatorPanel,      false);
    tabs.addTab ("Effects",    juce::Colours::darkgrey, &effectsPanel,          false);
    tabs.addTab ("Macros",     juce::Colours::darkgrey, &macroPanel,            false);
    tabs.addTab ("Granular",   juce::Colours::darkgrey, &granularPanel,         false);
}

void PluginEditor::setupMidiLearnButton()
{
    midiLearnButton.setToggleState (false, juce::dontSendNotification);
    midiLearnButton.setColour (juce::ToggleButton::textColourId, juce::Colours::white);
    midiLearnButton.setTooltip ("Enable MIDI Learn — select a parameter, then move a controller");
    midiLearnButton.addListener (this);

    midiLearnLabel.setJustificationType (juce::Justification::centredLeft);
    midiLearnLabel.setFont (juce::Font (11.0f, juce::Font::plain));
    midiLearnLabel.setColour (juce::Label::textColourId, juce::Colours::orange);
    midiLearnLabel.setVisible (false);

    // Populate parameter selector with all APVTS parameters
    paramSelector.addItem ("(select parameter)", 1);
    auto& params = processorRef.getParameters();
    for (int i = 0; i < params.size(); ++i)
    {
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*>(params[i]))
            paramSelector.addItem (rp->getName (64), i + 2); // offset by 2: id 1 = placeholder
    }
    paramSelector.setSelectedId (1, juce::dontSendNotification);
    paramSelector.setVisible (false);
    paramSelector.addListener (this);
}

void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void PluginEditor::resized()
{
    auto area = getLocalBounds();

    // Header strip: 32 px tall
    auto header = area.removeFromTop (32);
    midiLearnButton.setBounds (header.removeFromLeft (110).reduced (4, 4));
    paramSelector.setBounds   (header.removeFromLeft (200).reduced (2, 4));
    presetsButton.setBounds   (header.removeFromRight (72).reduced (4, 4));
    helpButton.setBounds      (header.removeFromRight (28).reduced (2, 4));
    midiLearnLabel.setBounds  (header.reduced (4, 4));

    // Preset browser (collapsible, 160px)
    if (presetsVisible)
    {
        presetBrowserPanel.setBounds (area.removeFromTop (160));
        presetBrowserPanel.setVisible (true);
    }
    else
    {
        presetBrowserPanel.setVisible (false);
    }

    tabs.setBounds (area);
}

void PluginEditor::buttonClicked (juce::Button* button)
{
    if (button == &presetsButton)
    {
        presetsVisible = !presetsVisible;
        if (presetsVisible)
            presetBrowserPanel.refresh();
        resized();
        return;
    }

    if (button != &midiLearnButton)
        return;

    const bool isActive = midiLearnButton.getToggleState();
    processorRef.midiLearnActive = isActive;

    if (!isActive)
    {
        processorRef.stopMidiLearn();
        paramSelector.setSelectedId (1, juce::dontSendNotification);
    }

    updateMidiLearnUI();
}

void PluginEditor::comboBoxChanged (juce::ComboBox* combo)
{
    if (combo != &paramSelector)
        return;

    const int selectedId = paramSelector.getSelectedId();
    if (selectedId <= 1)
    {
        processorRef.learnParameterIndex = -1;
        return;
    }

    // id = paramIndex + 2  →  paramIndex = selectedId - 2
    const int paramIndex = selectedId - 2;
    processorRef.startMidiLearnForParameter (paramIndex);

    midiLearnLabel.setText ("Waiting for CC...", juce::dontSendNotification);
    midiLearnLabel.setVisible (true);
}

void PluginEditor::updateMidiLearnUI()
{
    const bool active = midiLearnButton.getToggleState();
    paramSelector.setVisible (active);

    if (!active)
    {
        midiLearnLabel.setVisible (false);
        midiLearnLabel.setText ("", juce::dontSendNotification);
    }
}

bool PluginEditor::keyPressed (const juce::KeyPress& key)
{
    if (key == juce::KeyPress ('z', juce::ModifierKeys::commandModifier, 0))
    {
        processorRef.undoManager.undo();
        return true;
    }
    if (key == juce::KeyPress ('z', juce::ModifierKeys::commandModifier | juce::ModifierKeys::shiftModifier, 0))
    {
        processorRef.undoManager.redo();
        return true;
    }
    return false;
}
