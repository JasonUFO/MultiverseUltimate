#include "RoutingPanel.h"
#include "../PluginProcessor.h"
#include "../Layers/LayerManager.h"
#include "../Layers/LayerEngine.h"

//==============================================================================
// LayerBlock
//==============================================================================

RoutingPanel::LayerBlock::LayerBlock(int idx, PluginProcessor& p)
    : layerIndex(idx), proc(p)
{
    nameLabel.setText("Layer " + juce::String(idx + 1), juce::dontSendNotification);
    nameLabel.setJustificationType(juce::Justification::centredLeft);
    nameLabel.setColour(juce::Label::textColourId, MultiverseFlatTheme::textPrimary);
    nameLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    addAndMakeVisible(nameLabel);

    engineSelector.addItem("Off",      1);
    engineSelector.addItem("Synth",    2);
    engineSelector.addItem("Granular", 3);
    engineSelector.addItem("Sampler",  4);
    engineSelector.setSelectedId(2, juce::dontSendNotification);
    engineSelector.onChange = [this]
    {
        auto type = static_cast<LayerEngineType>(engineSelector.getSelectedId() - 2);
        proc.getLayerManager().getLayer(layerIndex).setEngineType(type);
    };
    addAndMakeVisible(engineSelector);

    muteButton.setButtonText("M");
    muteButton.setClickingTogglesState(true);
    muteButton.onClick = [this]
    {
        proc.getLayerManager().getLayer(layerIndex).setMute(muteButton.getToggleState());
    };
    addAndMakeVisible(muteButton);

    soloButton.setButtonText("S");
    soloButton.setClickingTogglesState(true);
    soloButton.onClick = [this]
    {
        proc.getLayerManager().getLayer(layerIndex).setSolo(soloButton.getToggleState());
    };
    addAndMakeVisible(soloButton);

    busSelector.addItem("Main", 1);
    for (int b = 1; b <= 8; ++b)
        busSelector.addItem("Bus " + juce::String(b), b + 1);
    busSelector.setSelectedId(1, juce::dontSendNotification);
    busSelector.onChange = [this]
    {
        int id = busSelector.getSelectedId();
        proc.getLayerManager().getLayer(layerIndex).setOutputBusIndex(id <= 1 ? 0 : id - 1);
    };
    addAndMakeVisible(busSelector);
}

void RoutingPanel::LayerBlock::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto& layer = proc.getLayerManager().getLayer(layerIndex);

    bool active = layer.getEngineType() != LayerEngineType::Off;
    bool soloed = layer.isSoloed();
    bool muted  = layer.isMuted();

    juce::Colour borderColor = soloed  ? MultiverseFlatTheme::accentPink
                              : active ? MultiverseFlatTheme::accentCyan
                                       : MultiverseFlatTheme::borderLight;

    MultiverseFlatTheme::drawCard(g, bounds, 6.0f, active);

    if (active)
    {
        // Level bar at bottom
        float barW = static_cast<float>((bounds.getWidth() - 8) * levelValue);
        g.setColour(MultiverseFlatTheme::accentCyan.withAlpha(0.5f));
        g.fillRoundedRectangle(bounds.getX() + 4, bounds.getBottom() - 6, barW, 3.0f, 1.5f);
    }

    // Bus indicator for non-main routing
    int busIdx = layer.getOutputBusIndex();
    if (busIdx > 0)
    {
        g.setColour(MultiverseFlatTheme::accentAmber);
        g.setFont(juce::Font(8.0f));
        g.drawText("B" + juce::String(busIdx),
                   bounds.withTrimmedLeft(bounds.getWidth() - 24).withTrimmedTop(2),
                   juce::Justification::topRight);
    }
}

void RoutingPanel::LayerBlock::resized()
{
    auto area = getLocalBounds().reduced(4);

    // Row 1: Name + Mute + Solo
    auto row1 = area.removeFromTop(16);
    nameLabel.setBounds(row1.removeFromLeft(row1.getWidth() - 52));
    muteButton.setBounds(row1.removeFromLeft(24));
    soloButton.setBounds(row1.removeFromLeft(24));

    // Row 2: Engine type
    auto row2 = area.removeFromTop(20);
    engineSelector.setBounds(row2);

    // Row 3: Bus selector
    auto row3 = area.removeFromTop(18);
    busSelector.setBounds(row3);

    // Bottom: level bar area (painted in paint())
}

void RoutingPanel::LayerBlock::updateFromModel()
{
    auto& layer = proc.getLayerManager().getLayer(layerIndex);

    int engineId = static_cast<int>(layer.getEngineType()) + 2;
    if (engineSelector.getSelectedId() != engineId)
        engineSelector.setSelectedId(engineId, juce::dontSendNotification);

    if (muteButton.getToggleState() != layer.isMuted())
        muteButton.setToggleState(layer.isMuted(), juce::dontSendNotification);

    if (soloButton.getToggleState() != layer.isSoloed())
        soloButton.setToggleState(layer.isSoloed(), juce::dontSendNotification);

    int busId = layer.getOutputBusIndex() + 1;
    if (busSelector.getSelectedId() != busId)
        busSelector.setSelectedId(busId, juce::dontSendNotification);

    levelValue = layer.getLevel();
}

//==============================================================================
// DrumsSummaryBlock
//==============================================================================

RoutingPanel::DrumsSummaryBlock::DrumsSummaryBlock(PluginProcessor& p) : proc(p)
{
    infoLabel.setText("DRUMS \xe2\x80\x94 8 tracks", juce::dontSendNotification);
    infoLabel.setJustificationType(juce::Justification::centredLeft);
    infoLabel.setColour(juce::Label::textColourId, MultiverseFlatTheme::textPrimary);
    infoLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    addAndMakeVisible(infoLabel);

    editButton.setButtonText("Edit");
    editButton.onClick = [this] { if (onEdit) onEdit(); };
    addAndMakeVisible(editButton);
}

void RoutingPanel::DrumsSummaryBlock::paint(juce::Graphics& g)
{
    MultiverseFlatTheme::drawCard(g, getLocalBounds().toFloat(), 6.0f);
}

void RoutingPanel::DrumsSummaryBlock::resized()
{
    auto area = getLocalBounds().reduced(6);
    infoLabel.setBounds(area.removeFromLeft(area.getWidth() - 50));
    editButton.setBounds(area.removeFromRight(44).reduced(2));
}

//==============================================================================
// ChainStrip (drag-reorder, mirrors EffectChainStrip)
//==============================================================================

const char* RoutingPanel::ChainStrip::effectName(int id)
{
    switch (id)
    {
        case 0: return "CHOR";
        case 1: return "DIST";
        case 2: return "EQ";
        case 3: return "COMP";
        case 4: return "DLY";
        case 5: return "REV";
        default: return "?";
    }
}

RoutingPanel::ChainStrip::ChainStrip(PluginProcessor& p) : proc(p)
{
    setMouseCursor(juce::MouseCursor::DraggingHandCursor);
}

juce::Rectangle<int> RoutingPanel::ChainStrip::tileRect(int i) const
{
    const int tileW = getWidth() / 6;
    return { i * tileW + 2, 2, tileW - 4, getHeight() - 4 };
}

int RoutingPanel::ChainStrip::slotAt(int x) const
{
    const int tileW = juce::jmax(1, getWidth() / 6);
    return juce::jlimit(0, 5, x / tileW);
}

void RoutingPanel::ChainStrip::paint(juce::Graphics& g)
{
    for (int i = 0; i < 6; ++i)
    {
        const int id = proc.getChainSlot(i);
        const auto tile = tileRect(i);

        const bool isSource = dragging && (i == dragSource);
        const bool isTarget = dragging && (i == dragOver) && (i != dragSource);

        g.setColour(isTarget  ? MultiverseFlatTheme::accentAmber.withAlpha(0.85f)
                   : isSource ? MultiverseFlatTheme::bgRaised.darker(0.3f)
                              : MultiverseFlatTheme::bgRaised);
        g.fillRoundedRectangle(tile.toFloat(), 4.f);

        g.setColour(juce::Colours::white.withAlpha(isSource ? 0.45f : 0.90f));
        g.setFont(juce::Font(9.f, juce::Font::bold));
        g.drawFittedText(effectName(id), tile, juce::Justification::centred, 1);

        // Arrow between tiles
        if (i < 5)
        {
            g.setColour(MultiverseFlatTheme::textMuted);
            auto arrowX = tile.getRight() + 2;
            auto arrowY = tile.getCentreY();
            g.drawLine(static_cast<float>(arrowX), static_cast<float>(arrowY - 3),
                       static_cast<float>(arrowX + 3), static_cast<float>(arrowY), 1.5f);
            g.drawLine(static_cast<float>(arrowX), static_cast<float>(arrowY + 3),
                       static_cast<float>(arrowX + 3), static_cast<float>(arrowY), 1.5f);
        }
    }
}

void RoutingPanel::ChainStrip::mouseDown(const juce::MouseEvent& e)
{
    dragSource = slotAt(e.x);
    dragOver   = dragSource;
    dragging   = true;
    repaint();
}

void RoutingPanel::ChainStrip::mouseDrag(const juce::MouseEvent& e)
{
    if (!dragging) return;
    const int over = slotAt(e.x);
    if (over != dragOver) { dragOver = over; repaint(); }
}

void RoutingPanel::ChainStrip::mouseUp(const juce::MouseEvent&)
{
    if (dragging && dragSource >= 0 && dragOver >= 0 && dragSource != dragOver)
        proc.swapChainSlots(dragSource, dragOver);
    dragging = false;
    dragSource = -1;
    dragOver = -1;
    repaint();
}

void RoutingPanel::ChainStrip::mouseExit(const juce::MouseEvent&)
{
    if (dragging) { dragging = false; dragSource = -1; dragOver = -1; repaint(); }
}

//==============================================================================
// RoutingPanel
//==============================================================================

RoutingPanel::RoutingPanel(PluginProcessor& p)
    : processorRef(p),
      drumsBlock(p),
      chainStrip(p),
      auxDelayKnob(),
      auxReverbKnob()
{
    for (int i = 0; i < 8; ++i)
    {
        layerBlocks[i] = std::make_unique<LayerBlock>(i, p);
        addAndMakeVisible(*layerBlocks[i]);
    }

    addAndMakeVisible(drumsBlock);
    drumsBlock.onEdit = [this] { if (onSwitchToTab) onSwitchToTab(1); };

    addAndMakeVisible(chainStrip);

    // Aux send knobs
    auxDelayKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    auxDelayKnob.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    auxDelayKnob.init(p, "auxSendDelay");
    addAndMakeVisible(auxDelayKnob);

    auxReverbKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    auxReverbKnob.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    auxReverbKnob.init(p, "auxSendReverb");
    addAndMakeVisible(auxReverbKnob);

    auxDelayLabel.setText("Delay", juce::dontSendNotification);
    auxDelayLabel.setJustificationType(juce::Justification::centred);
    auxDelayLabel.setColour(juce::Label::textColourId, MultiverseFlatTheme::textSecondary);
    auxDelayLabel.setFont(juce::Font(9.0f));
    addAndMakeVisible(auxDelayLabel);

    auxReverbLabel.setText("Reverb", juce::dontSendNotification);
    auxReverbLabel.setJustificationType(juce::Justification::centred);
    auxReverbLabel.setColour(juce::Label::textColourId, MultiverseFlatTheme::textSecondary);
    auxReverbLabel.setFont(juce::Font(9.0f));
    addAndMakeVisible(auxReverbLabel);

    auxSectionLabel.setText("AUX SENDS", juce::dontSendNotification);
    auxSectionLabel.setJustificationType(juce::Justification::centredLeft);
    auxSectionLabel.setColour(juce::Label::textColourId, MultiverseFlatTheme::textSecondary);
    auxSectionLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    addAndMakeVisible(auxSectionLabel);

    startTimerHz(10);
    updateUI();
}

RoutingPanel::~RoutingPanel()
{
    stopTimer();
}

void RoutingPanel::paint(juce::Graphics& g)
{
    g.fillAll(MultiverseFlatTheme::bgBase);

    // Section headers
    g.setColour(MultiverseFlatTheme::textSecondary);
    g.setFont(juce::Font(10.0f, juce::Font::bold));

    g.drawText("GENERATORS", generatorsRect.removeFromTop(16).reduced(4, 0),
               juce::Justification::topLeft);
    generatorsRect = generatorsRect.withTrimmedTop(16); // already consumed

    g.drawText("DRUMS", drumsRect.removeFromTop(0).reduced(4, 0),
               juce::Justification::topLeft);

    // Connection lines
    drawConnectionLines(g);
}

void RoutingPanel::resized()
{
    auto area = getLocalBounds().reduced(8, 8);

    // ─── Generators Section ───
    generatorsRect = area.removeFromTop(16);
    auto genArea = area.removeFromTop(140);

    // 2 rows × 4 columns of layer blocks
    const int cols = 4;
    const int blockW = (genArea.getWidth() - (cols - 1) * 6) / cols;
    const int blockH = 66;

    for (int row = 0; row < 2; ++row)
    {
        for (int col = 0; col < cols; ++col)
        {
            int idx = row * cols + col;
            int x = genArea.getX() + col * (blockW + 6);
            int y = genArea.getY() + row * (blockH + 6);
            layerBlocks[idx]->setBounds(x, y, blockW, blockH);
        }
    }

    area.removeFromTop(8); // gap

    // ─── Drums Section ───
    drumsRect = area.removeFromTop(36);
    drumsBlock.setBounds(drumsRect);

    area.removeFromTop(8);

    // ─── Global Effects Chain ───
    auto chainLabel = area.removeFromTop(16);
    // (label drawn in paint)
    juce::ignoreUnused(chainLabel);

    chainRect = area.removeFromTop(36);
    chainStrip.setBounds(chainRect);

    area.removeFromTop(8);

    // ─── Aux Sends ───
    auxRect = area.removeFromTop(56);
    auxSectionLabel.setBounds(auxRect.removeFromTop(14));

    auto auxArea = auxRect.reduced(4);
    const int knobSz = 32;
    const int knobGap = 20;

    auto delayArea = auxArea.removeFromLeft(knobSz + knobGap);
    auxDelayKnob.setBounds(delayArea.removeFromTop(knobSz));
    auxDelayLabel.setBounds(delayArea.removeFromTop(14));

    auto reverbArea = auxArea.removeFromLeft(knobSz + knobGap);
    auxReverbKnob.setBounds(reverbArea.removeFromTop(knobSz));
    auxReverbLabel.setBounds(reverbArea.removeFromTop(14));

    area.removeFromTop(8);

    // ─── Output terminus ───
    outputRect = area.removeFromTop(28);
}

void RoutingPanel::updateUI()
{
    for (int i = 0; i < 8; ++i)
        layerBlocks[i]->updateFromModel();

    repaint();
}

void RoutingPanel::drawConnectionLines(juce::Graphics& g)
{
    const float lineW = 1.5f;

    // Collect active layer centers
    float flowCenterX = static_cast<float>(getWidth() / 2);
    float genBottom = static_cast<float>(layerBlocks[7]->getBottom() + 4);
    float drumsBottom = static_cast<float>(drumsBlock.getBounds().getBottom() + 4);
    float chainTop = static_cast<float>(chainStrip.getY() - 4);
    float chainBottom = static_cast<float>(chainStrip.getBottom() + 4);
    float auxTop = static_cast<float>(auxDelayKnob.getY() - 4);
    float outTop = static_cast<float>(outputRect.getY() + 14);

    // Main vertical flow line from generators to chain
    g.setColour(MultiverseFlatTheme::accentCyan.withAlpha(0.3f));
    g.drawLine(flowCenterX, genBottom, flowCenterX, chainTop, lineW);

    // Small arrow above chain
    drawArrowDown(g, flowCenterX, chainTop - 8, chainTop, MultiverseFlatTheme::accentCyan.withAlpha(0.5f));

    // From chain to aux sends
    g.drawLine(flowCenterX, chainBottom, flowCenterX, auxTop, lineW);

    // Aux send branches
    float auxDelayCx = static_cast<float>(auxDelayKnob.getBounds().getCentreX());
    float auxReverbCx = static_cast<float>(auxReverbKnob.getBounds().getCentreX());
    float auxY = static_cast<float>(auxDelayKnob.getBounds().getCentreY());

    g.drawLine(flowCenterX, auxTop + 4, auxDelayCx, auxY, lineW);
    g.drawLine(flowCenterX, auxTop + 4, auxReverbCx, auxY, lineW);

    // From aux to output
    float auxBottom = static_cast<float>(auxDelayKnob.getBounds().getBottom() + auxReverbLabel.getBounds().getBottom()) * 0.5f + 8.0f;
    g.drawLine(flowCenterX, auxBottom, flowCenterX, outTop, lineW);

    // Per-layer connection stubs from layer blocks to the flow line
    for (int i = 0; i < 8; ++i)
    {
        auto& layer = processorRef.getLayerManager().getLayer(i);
        if (layer.getEngineType() == LayerEngineType::Off) continue;

        auto blockBounds = layerBlocks[i]->getBounds();
        float cx = static_cast<float>(blockBounds.getCentreX());
        float by = static_cast<float>(blockBounds.getBottom());

        juce::Colour lineColour = layer.isSoloed() ? MultiverseFlatTheme::accentPink
                                  : layer.getOutputBusIndex() > 0 ? MultiverseFlatTheme::accentAmber
                                  : MultiverseFlatTheme::accentCyan;

        float alpha = layer.isMuted() ? 0.15f : 0.5f;
        g.setColour(lineColour.withAlpha(alpha));

        // Short vertical stub from block bottom
        g.drawLine(cx, by, cx, by + 3, lineW);
    }

    // Drums connection stub
    {
        float drumsCx = static_cast<float>(drumsBlock.getBounds().getCentreX());
        float drumsBy = static_cast<float>(drumsBlock.getBounds().getBottom());
        g.setColour(MultiverseFlatTheme::accentCyan.withAlpha(0.3f));
        g.drawLine(drumsCx, drumsBy, drumsCx, drumsBy + 3, lineW);
    }

    // Output label
    g.setColour(MultiverseFlatTheme::textPrimary);
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawText("\xe2\x86\x92 OUTPUT", outputRect.reduced(4, 0),
               juce::Justification::centredLeft);
}

void RoutingPanel::drawArrowDown(juce::Graphics& g, float x, float y1, float y2, juce::Colour colour)
{
    g.setColour(colour);
    g.drawLine(x, y1, x, y2, 1.5f);
    const float sz = 4.0f;
    // Arrow head pointing down
    g.drawLine(x - sz, y2 - sz, x, y2, 1.5f);
    g.drawLine(x + sz, y2 - sz, x, y2, 1.5f);
}