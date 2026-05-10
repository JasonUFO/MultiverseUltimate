#include "GlobalPanel.h"
#include "../PluginProcessor.h"

GlobalPanel::GlobalPanel (PluginProcessor& p)
    : processorRef (p),
      voiceModeSelector(),
      portamentoSlider(),
      portaAlwaysButton ("Always"),
      mpeButton ("MPE"),
      unisonVoicesBox(),
      unisonDetuneSlider(),
      unisonWidthSlider(),
      unisonSpreadSelector(),
      chordEnableButton ("Chord"),
      chordShapeSelector(),
      chordStrumSlider(),
      fxModeButton ("FX Mode"),
      qualitySelector()
{
    auto& apvts = p.apvts;

    // Voice Mode
    voiceModeLabel.setText ("VOICE", juce::dontSendNotification);
    voiceModeLabel.setFont (MultiverseFlatTheme::headerFont());
    voiceModeLabel.setColour (juce::Label::textColourId, MultiverseFlatTheme::accentCyan());
    addAndMakeVisible (voiceModeLabel);

    voiceModeSelector.addItem ("Poly", 1);
    voiceModeSelector.addItem ("Mono", 2);
    voiceModeSelector.addItem ("Legato", 3);
    voiceModeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, "voiceMode", voiceModeSelector);
    addAndMakeVisible (voiceModeSelector);

    // Portamento
    portamentoSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    portamentoSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    portamentoSlider.init (p, "portamento");
    portamentoLabel.setText ("Porta", juce::dontSendNotification);
    portamentoLabel.setFont (MultiverseFlatTheme::labelFont());
    portamentoLabel.setColour (juce::Label::textColourId, MultiverseFlatTheme::textSecondary());
    portamentoAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, "portamento", portamentoSlider);
    addAndMakeVisible (portamentoSlider);
    addAndMakeVisible (portamentoLabel);

    // Porta Always
    portaAlwaysButton.setColour (juce::ToggleButton::textColourId, MultiverseFlatTheme::textSecondary());
    portaAlwaysButton.setColour (juce::ToggleButton::tickColourId, MultiverseFlatTheme::accentCyan());
    portaAlwaysAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, "portaAlways", portaAlwaysButton);
    addAndMakeVisible (portaAlwaysButton);

    // MPE
    mpeButton.setColour (juce::ToggleButton::textColourId, MultiverseFlatTheme::textSecondary());
    mpeButton.setColour (juce::ToggleButton::tickColourId, MultiverseFlatTheme::accentCyan());
    mpeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, "mpeEnabled", mpeButton);
    addAndMakeVisible (mpeButton);

    // Unison Voices
    unisonVoicesLabel.setText ("UNISON", juce::dontSendNotification);
    unisonVoicesLabel.setFont (MultiverseFlatTheme::headerFont());
    unisonVoicesLabel.setColour (juce::Label::textColourId, MultiverseFlatTheme::accentCyan());
    addAndMakeVisible (unisonVoicesLabel);

    for (int i = 1; i <= 16; ++i)
        unisonVoicesBox.addItem (juce::String (i), i);
    unisonVoicesAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, "unisonVoices", unisonVoicesBox);
    addAndMakeVisible (unisonVoicesBox);

    // Unison Detune
    unisonDetuneSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    unisonDetuneSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    unisonDetuneSlider.init (p, "unisonDetune");
    unisonDetuneLabel.setText ("Detune", juce::dontSendNotification);
    unisonDetuneLabel.setFont (MultiverseFlatTheme::labelFont());
    unisonDetuneLabel.setColour (juce::Label::textColourId, MultiverseFlatTheme::textSecondary());
    unisonDetuneAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, "unisonDetune", unisonDetuneSlider);
    addAndMakeVisible (unisonDetuneSlider);
    addAndMakeVisible (unisonDetuneLabel);

    // Unison Width
    unisonWidthSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    unisonWidthSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    unisonWidthSlider.init (p, "unisonWidth");
    unisonWidthLabel.setText ("Width", juce::dontSendNotification);
    unisonWidthLabel.setFont (MultiverseFlatTheme::labelFont());
    unisonWidthLabel.setColour (juce::Label::textColourId, MultiverseFlatTheme::textSecondary());
    unisonWidthAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, "unisonWidth", unisonWidthSlider);
    addAndMakeVisible (unisonWidthSlider);
    addAndMakeVisible (unisonWidthLabel);

    // Unison Spread Mode
    unisonSpreadSelector.addItem ("Stacked", 1);
    unisonSpreadSelector.addItem ("Chord", 2);
    unisonSpreadSelector.addItem ("Random", 3);
    unisonSpreadAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, "unisonSpreadMode", unisonSpreadSelector);
    unisonSpreadLabel.setText ("Spread", juce::dontSendNotification);
    unisonSpreadLabel.setFont (MultiverseFlatTheme::labelFont());
    unisonSpreadLabel.setColour (juce::Label::textColourId, MultiverseFlatTheme::textSecondary());
    addAndMakeVisible (unisonSpreadSelector);
    addAndMakeVisible (unisonSpreadLabel);

    // Chord/Strum
    chordEnableButton.setColour (juce::ToggleButton::textColourId, MultiverseFlatTheme::textSecondary());
    chordEnableButton.setColour (juce::ToggleButton::tickColourId, MultiverseFlatTheme::accentCyan());
    chordEnableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, "chordModeEnabled", chordEnableButton);
    addAndMakeVisible (chordEnableButton);

    chordShapeSelector.addItem ("Root Only", 1);
    chordShapeSelector.addItem ("Major", 2);
    chordShapeSelector.addItem ("Minor", 3);
    chordShapeSelector.addItem ("Maj7", 4);
    chordShapeSelector.addItem ("Min7", 5);
    chordShapeSelector.addItem ("Dom7", 6);
    chordShapeSelector.addItem ("Dim", 7);
    chordShapeSelector.addItem ("Aug", 8);
    chordShapeSelector.addItem ("Sus2", 9);
    chordShapeSelector.addItem ("Sus4", 10);
    chordShapeSelector.addItem ("Power", 11);
    chordShapeSelector.addItem ("Octave", 12);
    chordShapeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, "chordShape", chordShapeSelector);
    addAndMakeVisible (chordShapeSelector);

    chordShapeLabel.setText ("Shape", juce::dontSendNotification);
    chordShapeLabel.setFont (MultiverseFlatTheme::labelFont());
    chordShapeLabel.setColour (juce::Label::textColourId, MultiverseFlatTheme::textSecondary());
    addAndMakeVisible (chordShapeLabel);

    chordStrumSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    chordStrumSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    chordStrumSlider.init (p, "chordStrumDelay");
    chordStrumLabel.setText ("Strum", juce::dontSendNotification);
    chordStrumLabel.setFont (MultiverseFlatTheme::labelFont());
    chordStrumLabel.setColour (juce::Label::textColourId, MultiverseFlatTheme::textSecondary());
    chordStrumAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, "chordStrumDelay", chordStrumSlider);
    addAndMakeVisible (chordStrumSlider);
    addAndMakeVisible (chordStrumLabel);

    // Scale Key
    scaleKeyLabel.setText ("SCALE", juce::dontSendNotification);
    scaleKeyLabel.setFont (MultiverseFlatTheme::headerFont());
    scaleKeyLabel.setColour (juce::Label::textColourId, MultiverseFlatTheme::accentCyan());
    addAndMakeVisible (scaleKeyLabel);

    const char* keys[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
    for (int i = 0; i < 12; ++i)
        scaleKeySelector.addItem (keys[i], i + 1);
    scaleKeySelector.setSelectedId (1, juce::dontSendNotification);
    addAndMakeVisible (scaleKeySelector);

    scaleNameLabel.setText ("Type", juce::dontSendNotification);
    scaleNameLabel.setFont (MultiverseFlatTheme::labelFont());
    scaleNameLabel.setColour (juce::Label::textColourId, MultiverseFlatTheme::textSecondary());
    addAndMakeVisible (scaleNameLabel);

    const char* scales[] = {"Major","Minor","Dorian","Phrygian","Lydian","Mixolydian","Pentatonic","Blues","Harmonic Minor","Melodic Minor"};
    for (int i = 0; i < 10; ++i)
        scaleNameSelector.addItem (scales[i], i + 1);
    scaleNameSelector.setSelectedId (1, juce::dontSendNotification);
    addAndMakeVisible (scaleNameSelector);

    // Quality
    qualityLabel.setText ("QUALITY", juce::dontSendNotification);
    qualityLabel.setFont (MultiverseFlatTheme::headerFont());
    qualityLabel.setColour (juce::Label::textColourId, MultiverseFlatTheme::accentCyan());
    addAndMakeVisible (qualityLabel);

    qualitySelector.addItem ("Off", 1);
    qualitySelector.addItem ("2x", 2);
    qualitySelector.addItem ("4x", 3);
    qualityAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, "globalQuality", qualitySelector);
    addAndMakeVisible (qualitySelector);

    // FX Mode
    fxModeButton.setColour (juce::ToggleButton::textColourId, MultiverseFlatTheme::textSecondary());
    fxModeButton.setColour (juce::ToggleButton::tickColourId, MultiverseFlatTheme::accent1());
    fxModeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, "fxModeEnabled", fxModeButton);
    addAndMakeVisible (fxModeButton);

    // Skin selector
    skinLabel.setText ("SKIN", juce::dontSendNotification);
    skinLabel.setFont (MultiverseFlatTheme::headerFont());
    skinLabel.setColour (juce::Label::textColourId, MultiverseFlatTheme::accent1());
    addAndMakeVisible (skinLabel);

    auto& sm = SkinManager::instance();
    for (int i = 0; i < sm.numSkins(); ++i)
        skinSelector.addItem (sm.skinName(i), i + 1);
    skinSelector.setSelectedId (sm.getSkinIndex() + 1, juce::dontSendNotification);
    skinSelector.setColour (juce::ComboBox::backgroundColourId, MultiverseFlatTheme::bgDeep());
    skinSelector.setColour (juce::ComboBox::textColourId, MultiverseFlatTheme::textPrimary());
    skinSelector.setColour (juce::ComboBox::outlineColourId, MultiverseFlatTheme::borderLight());
    skinSelector.setColour (juce::ComboBox::arrowColourId, MultiverseFlatTheme::textMuted());
    skinSelector.setColour (juce::ComboBox::focusedOutlineColourId, MultiverseFlatTheme::accent1());
    skinSelector.onChange = [this] { skinChanged(); };
    addAndMakeVisible (skinSelector);

    // Sync combo box when skin changes externally (e.g. from menu)
    SkinManager::instance().addSkinChangeCallback([this]()
    {
        skinSelector.setSelectedId (SkinManager::instance().getSkinIndex() + 1,
                                     juce::dontSendNotification);
    });
}

void GlobalPanel::skinChanged()
{
    int id = skinSelector.getSelectedId();
    if (id > 0)
    {
        SkinManager::instance().setSkin(id - 1);
        // Callback in constructor handles combo sync;
        // SkinManager listener + callback handles repaint
    }
}

void GlobalPanel::paint (juce::Graphics& g)
{
    MultiverseFlatTheme::drawContentBackground(g, getLocalBounds().toFloat());
    auto b = getLocalBounds().toFloat();

    using M = MultiverseFlatTheme::Metrics;

    // Section cards
    auto voiceRect = juce::Rectangle<int> (M::outerMargin, M::outerMargin, getWidth() / 2 - M::outerMargin - M::smallGap / 2, 100);
    MultiverseFlatTheme::drawCard (g, voiceRect.toFloat(), 6.0f);

    auto unisonRect = juce::Rectangle<int> (getWidth() / 2 + M::smallGap / 2, M::outerMargin, getWidth() / 2 - M::outerMargin - M::smallGap / 2, 100);
    MultiverseFlatTheme::drawCard (g, unisonRect.toFloat(), 6.0f);

    auto chordRect = juce::Rectangle<int> (M::outerMargin, 110 + M::sectionGap, getWidth() / 2 - M::outerMargin - M::smallGap / 2, 80);
    MultiverseFlatTheme::drawCard (g, chordRect.toFloat(), 6.0f);

    auto scaleRect = juce::Rectangle<int> (getWidth() / 2 + M::smallGap / 2, 110 + M::sectionGap, getWidth() / 2 - M::outerMargin - M::smallGap / 2, 80);
    MultiverseFlatTheme::drawCard (g, scaleRect.toFloat(), 6.0f);

    auto qualRect = juce::Rectangle<int> (M::outerMargin, 200 + 2 * M::sectionGap, getWidth() - 2 * M::outerMargin, 60);
    MultiverseFlatTheme::drawCard (g, qualRect.toFloat(), 6.0f);

    // Skin selector card
    auto skinRect = juce::Rectangle<int> (M::outerMargin, 270 + 3 * M::sectionGap, getWidth() - 2 * M::outerMargin, 60);
    MultiverseFlatTheme::drawCard (g, skinRect.toFloat(), 6.0f);
}

void GlobalPanel::resized()
{
    using M = MultiverseFlatTheme::Metrics;
    auto area = getLocalBounds().reduced (M::outerMargin);
    int halfW = (area.getWidth() - M::smallGap) / 2;

    // === Voice Mode row (left) ===
    auto voiceArea = area.removeFromTop (100);
    auto voiceLeft = voiceArea.removeFromLeft (halfW);
    auto card = voiceLeft.reduced (M::sectionPadding);

    voiceModeLabel.setBounds (card.removeFromTop (M::sectionHeaderH));
    auto voiceRow = card.removeFromTop (28);
    voiceModeSelector.setBounds (voiceRow.removeFromLeft (80));
    portaAlwaysButton.setBounds (voiceRow.removeFromLeft (60));
    mpeButton.setBounds (voiceRow.removeFromLeft (50));
    portamentoSlider.setBounds (card.removeFromLeft (60).reduced (4));
    portamentoLabel.setBounds (card.removeFromLeft (50).reduced (0, 24));

    // === Unison row (right) ===
    auto unisonArea = voiceArea;
    card = unisonArea.reduced (M::sectionPadding);

    unisonVoicesLabel.setBounds (card.removeFromTop (M::sectionHeaderH));
    auto uniRow1 = card.removeFromTop (28);
    unisonVoicesBox.setBounds (uniRow1.removeFromLeft (60));
    unisonSpreadSelector.setBounds (uniRow1.removeFromLeft (80));
    unisonSpreadLabel.setBounds (uniRow1.removeFromLeft (50).reduced (0, 4));
    auto uniRow2 = card.removeFromTop (60);
    unisonDetuneSlider.setBounds (uniRow2.removeFromLeft (60).reduced (4));
    unisonWidthSlider.setBounds (uniRow2.removeFromLeft (60).reduced (4));
    unisonDetuneLabel.setBounds (unisonDetuneSlider.getBounds().withY (unisonDetuneSlider.getBottom() + 2).withHeight (14));
    unisonWidthLabel.setBounds (unisonWidthSlider.getBounds().withY (unisonWidthSlider.getBottom() + 2).withHeight (14));

    // === Chord/Strum (left) ===
    area.removeFromTop (M::sectionGap);
    auto chordArea = area.removeFromTop (80);
    auto chordLeft = chordArea.removeFromLeft (halfW);
    card = chordLeft.reduced (M::sectionPadding);

    chordEnableButton.setBounds (card.removeFromTop (20));
    auto chordRow = card.removeFromTop (28);
    chordShapeSelector.setBounds (chordRow.removeFromLeft (90));
    chordShapeLabel.setBounds (chordRow.removeFromLeft (50).reduced (0, 4));
    chordStrumSlider.setBounds (card.removeFromLeft (60).reduced (4));
    chordStrumLabel.setBounds (card.removeFromLeft (50).reduced (0, 24));

    // === Scale (right) ===
    auto scaleArea = chordArea;
    card = scaleArea.reduced (M::sectionPadding);

    scaleKeyLabel.setBounds (card.removeFromTop (M::sectionHeaderH));
    auto scaleRow = card.removeFromTop (28);
    scaleKeySelector.setBounds (scaleRow.removeFromLeft (60));
    scaleNameSelector.setBounds (scaleRow.removeFromLeft (120));
    scaleNameLabel.setBounds (scaleRow.removeFromLeft (40).reduced (0, 4));

    // === Quality + FX Mode ===
    area.removeFromTop (M::sectionGap);
    auto qualArea = area.removeFromTop (60);
    card = qualArea.reduced (M::sectionPadding);

    qualityLabel.setBounds (card.removeFromTop (M::sectionHeaderH));
    auto qualRow = card.removeFromTop (28);
    qualitySelector.setBounds (qualRow.removeFromLeft (80));
    fxModeButton.setBounds (qualRow.removeFromLeft (80));

    // === Skin selector ===
    area.removeFromTop (M::sectionGap);
    auto skinArea = area.removeFromTop (60);
    card = skinArea.reduced (M::sectionPadding);

    skinLabel.setBounds (card.removeFromTop (M::sectionHeaderH));
    auto skinRow = card.removeFromTop (28);
    skinSelector.setBounds (skinRow.removeFromLeft (200));
}