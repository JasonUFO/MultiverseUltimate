#include "GranularPanel.h"
#include "../MultiverseFlatTheme.h"
#include "../PluginProcessor.h"

GranularPanel::GranularPanel (PluginProcessor& p) : proc(p)
{
    auto& apvts = p.apvts;

    // Load button
    loadButton.setColour (juce::TextButton::buttonColourId,   juce::Colour(0xff2a2a2a));
    loadButton.setColour (juce::TextButton::textColourOffId,  juce::Colours::cyan);
    loadButton.setTooltip ("Load an audio file as the granular source");
    loadButton.onClick = [this] { loadSourceFile(); };
    addAndMakeVisible (loadButton);

    fileLabel.setText (proc.granularEngine.getSourceFilePath().isEmpty()
                       ? "(built-in default)" : proc.granularEngine.getSourceFilePath(),
                       juce::dontSendNotification);
    fileLabel.setFont (MultiverseFlatTheme::labelFont());
    fileLabel.setColour (juce::Label::textColourId, juce::Colours::lightgrey);
    fileLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (fileLabel);

    // Grain controls
    setupSlider (positionSlider,     "granularPosition",    "Playback position in the source buffer");
    setupSlider (grainSizeSlider,    "granularGrainSize",   "Duration of each grain in seconds");
    setupSlider (spraySlider,        "granularSpray",       "Randomise grain start position");
    setupSlider (densitySlider,      "granularDensity",     "Grains spawned per second");
    setupSlider (pitchScatterSlider, "granularPitchScatter","Random pitch variation per grain (semitones)");
    setupSlider (stereoSpreadSlider, "granularStereoSpread","Stereo panning spread of grains");

    positionAttach    = std::make_unique<SliderAttach>(apvts, "granularPosition",     positionSlider);
    grainSizeAttach   = std::make_unique<SliderAttach>(apvts, "granularGrainSize",    grainSizeSlider);
    sprayAttach       = std::make_unique<SliderAttach>(apvts, "granularSpray",        spraySlider);
    densityAttach     = std::make_unique<SliderAttach>(apvts, "granularDensity",      densitySlider);
    pitchScatAttach   = std::make_unique<SliderAttach>(apvts, "granularPitchScatter", pitchScatterSlider);
    stereoSpreadAttach= std::make_unique<SliderAttach>(apvts, "granularStereoSpread", stereoSpreadSlider);

    positionSlider.init    (p, "granularPosition");
    grainSizeSlider.init   (p, "granularGrainSize");
    spraySlider.init       (p, "granularSpray");
    densitySlider.init     (p, "granularDensity");
    pitchScatterSlider.init(p, "granularPitchScatter");
    stereoSpreadSlider.init(p, "granularStereoSpread");

    // Envelope shape combo
    envShapeCombo.addItem ("Gaussian",  1);
    envShapeCombo.addItem ("Hann",      2);
    envShapeCombo.addItem ("Trapezoid", 3);
    envShapeCombo.addItem ("Triangle",  4);
    envShapeCombo.setTooltip ("Grain amplitude envelope shape");
    envShapeAttach = std::make_unique<ComboAttach>(apvts, "granularEnvShape", envShapeCombo);
    addAndMakeVisible (envShapeCombo);

    // Reverse toggle
    reverseButton.setColour (juce::ToggleButton::textColourId, juce::Colours::white);
    reverseButton.setTooltip ("Play grains in reverse");
    reverseAttach = std::make_unique<BoolAttach>(apvts, "granularReverse", reverseButton);
    addAndMakeVisible (reverseButton);

    // Voice envelope
    setupSlider (attackSlider,  "granularAttack",  "Voice envelope attack time");
    setupSlider (decaySlider,   "granularDecay",   "Voice envelope decay time");
    setupSlider (sustainSlider, "granularSustain", "Voice envelope sustain level");
    setupSlider (releaseSlider, "granularRelease", "Voice envelope release time");

    attackAttach  = std::make_unique<SliderAttach>(apvts, "granularAttack",  attackSlider);
    decayAttach   = std::make_unique<SliderAttach>(apvts, "granularDecay",   decaySlider);
    sustainAttach = std::make_unique<SliderAttach>(apvts, "granularSustain", sustainSlider);
    releaseAttach = std::make_unique<SliderAttach>(apvts, "granularRelease", releaseSlider);

    attackSlider.init (p, "granularAttack");
    decaySlider.init  (p, "granularDecay");
    sustainSlider.init(p, "granularSustain");
    releaseSlider.init(p, "granularRelease");

    // Labels
    for (auto* l : { &positionLabel, &grainSizeLabel, &sprayLabel, &densityLabel,
                     &pitchScatLabel, &stereoSpreadLabel, &envShapeLabel,
                     &attackLabel, &decayLabel, &sustainLabel, &releaseLabel })
        setupLabel (*l);

    envelopeHeader.setFont (MultiverseFlatTheme::headerFont());
    envelopeHeader.setColour (juce::Label::textColourId, juce::Colours::lightgrey);
    envelopeHeader.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (envelopeHeader);
}

void GranularPanel::setupSlider (NeuKnob& s, const juce::String& paramID,
                                  const juce::String& tooltip)
{
    s.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 16);
    s.setTooltip (tooltip);
    juce::ignoreUnused (paramID);
    addAndMakeVisible (s);
}

void GranularPanel::setupLabel (juce::Label& l)
{
    l.setFont (MultiverseFlatTheme::labelFont());
    l.setColour (juce::Label::textColourId, juce::Colours::lightgrey);
    l.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (l);
}

void GranularPanel::paint (juce::Graphics& g)
{
    MultiverseFlatTheme::drawContentBackground(g, getLocalBounds().toFloat());

    const float cr = 8.0f;

    if (sourceSectionRect.getHeight() > 0)
    {
        MultiverseFlatTheme::drawCard(g, sourceSectionRect.toFloat(), cr);
        // Section title
        g.setColour(MultiverseFlatTheme::textLabel());
        g.setFont(MultiverseFlatTheme::headerFont());
        g.drawText("SOURCE", sourceSectionRect.getX() + 8, sourceSectionRect.getY() + 5, 100, 14, juce::Justification::centredLeft);
        MultiverseFlatTheme::drawDivider(g, static_cast<float>(sourceSectionRect.getY() + 18), static_cast<float>(sourceSectionRect.getX() + 8), static_cast<float>(sourceSectionRect.getRight() - 8));
    }

    if (grainSectionRect.getHeight() > 0)
    {
        MultiverseFlatTheme::drawCard(g, grainSectionRect.toFloat(), cr);
        // Section title
        g.setColour(MultiverseFlatTheme::textLabel());
        g.setFont(MultiverseFlatTheme::headerFont());
        g.drawText("GRAIN", grainSectionRect.getX() + 8, grainSectionRect.getY() + 5, 100, 14, juce::Justification::centredLeft);
        MultiverseFlatTheme::drawDivider(g, static_cast<float>(grainSectionRect.getY() + 18), static_cast<float>(grainSectionRect.getX() + 8), static_cast<float>(grainSectionRect.getRight() - 8));
    }

    if (envelopeSectionRect.getHeight() > 0)
    {
        MultiverseFlatTheme::drawCard(g, envelopeSectionRect.toFloat(), cr);
        // Section title
        g.setColour(MultiverseFlatTheme::textLabel());
        g.setFont(MultiverseFlatTheme::headerFont());
        g.drawText("VOICE ENVELOPE", envelopeSectionRect.getX() + 8, envelopeSectionRect.getY() + 5, 140, 14, juce::Justification::centredLeft);
        MultiverseFlatTheme::drawDivider(g, static_cast<float>(envelopeSectionRect.getY() + 18), static_cast<float>(envelopeSectionRect.getX() + 8), static_cast<float>(envelopeSectionRect.getRight() - 8));
    }
}

void GranularPanel::resized()
{
    auto area = getLocalBounds().reduced (8);

    // SOURCE section: file load row only
    sourceSectionRect = area.removeFromTop(28 + 6).reduced(0, 0);
    auto sourceArea = sourceSectionRect.reduced(8);

    auto fileRow = sourceArea.removeFromTop (28);
    loadButton.setBounds (fileRow.removeFromLeft (80).reduced (2));
    fileLabel.setBounds  (fileRow.reduced (4, 2));

    area.removeFromTop(6);

    // GRAIN CONTROLS section: rows 1 and 2
    grainSectionRect = area.removeFromTop(80 + 4 + 80 + 4).reduced(0, 0);
    auto grainArea = grainSectionRect.reduced(8);

    // Row 1: Position, Grain Size, Spray, Density
    {
        auto row = grainArea.removeFromTop (80);
        auto labelRow = grainArea.removeFromTop (16);
        const int w = row.getWidth() / 4;

        positionSlider .setBounds (row.removeFromLeft(w).reduced(4));
        grainSizeSlider.setBounds (row.removeFromLeft(w).reduced(4));
        spraySlider    .setBounds (row.removeFromLeft(w).reduced(4));
        densitySlider  .setBounds (row.reduced(4));

        positionLabel .setBounds (labelRow.removeFromLeft(w));
        grainSizeLabel.setBounds (labelRow.removeFromLeft(w));
        sprayLabel    .setBounds (labelRow.removeFromLeft(w));
        densityLabel  .setBounds (labelRow);
    }

    grainArea.removeFromTop (4);

    // Row 2: Pitch Scatter, Stereo Spread, Env Shape, Reverse
    {
        auto row      = grainArea.removeFromTop (80);
        auto labelRow = grainArea.removeFromTop (16);
        const int w   = row.getWidth() / 4;

        pitchScatterSlider.setBounds (row.removeFromLeft(w).reduced(4));
        stereoSpreadSlider.setBounds (row.removeFromLeft(w).reduced(4));

        auto envShapeArea  = row.removeFromLeft(w).reduced(4);
        auto reverseArea   = row.reduced(4);

        envShapeCombo.setBounds (envShapeArea.removeFromTop (28));
        reverseButton.setBounds (reverseArea.removeFromTop  (28));

        pitchScatLabel   .setBounds (labelRow.removeFromLeft(w));
        stereoSpreadLabel.setBounds (labelRow.removeFromLeft(w));
        envShapeLabel    .setBounds (labelRow.removeFromLeft(w));
        // no label for reverse — button text is sufficient

        juce::ignoreUnused (labelRow);
    }

    area.removeFromTop(8);

    // VOICE ENVELOPE section
    envelopeSectionRect = area.withHeight(18 + 4 + 80);
    auto envArea = envelopeSectionRect.reduced(8);

    // Envelope header
    envelopeHeader.setBounds (envArea.removeFromTop (18));

    envArea.removeFromTop (4);

    // Row 3: ADSR
    {
        auto row      = envArea.removeFromTop (80);
        auto labelRow = envArea.removeFromTop (16);
        const int w   = row.getWidth() / 4;

        attackSlider .setBounds (row.removeFromLeft(w).reduced(4));
        decaySlider  .setBounds (row.removeFromLeft(w).reduced(4));
        sustainSlider.setBounds (row.removeFromLeft(w).reduced(4));
        releaseSlider.setBounds (row.reduced(4));

        attackLabel .setBounds (labelRow.removeFromLeft(w));
        decayLabel  .setBounds (labelRow.removeFromLeft(w));
        sustainLabel.setBounds (labelRow.removeFromLeft(w));
        releaseLabel.setBounds (labelRow);
    }
}

void GranularPanel::loadSourceFile()
{
    auto chooser = std::make_shared<juce::FileChooser>(
        "Load Granular Source",
        juce::File::getSpecialLocation (juce::File::userHomeDirectory),
        "*.wav;*.aif;*.aiff");

    chooser->launchAsync (juce::FileBrowserComponent::openMode |
                          juce::FileBrowserComponent::canSelectFiles,
        [this, chooser] (const juce::FileChooser& fc)
        {
            auto result = fc.getResult();
            if (!result.existsAsFile()) return;

            if (proc.granularEngine.loadSourceFile (result))
            {
                fileLabel.setText (result.getFileName(), juce::dontSendNotification);
                fileLabel.setColour (juce::Label::textColourId, juce::Colours::cyan);
            }
            else
            {
                fileLabel.setText ("(load failed)", juce::dontSendNotification);
                fileLabel.setColour (juce::Label::textColourId, juce::Colours::red);
            }
        });
}
