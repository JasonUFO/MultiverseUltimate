#include "ModulationMatrixPanel.h"

namespace {
    constexpr int HEADER_H  = 36;
    constexpr int COL_HDR_H = 20;
    constexpr int ROW_H     = 36;
    constexpr int ROW_GAP   = 3;
    constexpr int PADDING   = 8;
    constexpr int SRC_W     = 140;
    constexpr int TGT_W     = 140;
    constexpr int DEL_W     = 28;
    constexpr int INNER_GAP = 4;
}

// ─── Row ────────────────────────────────────────────────────────────────────

ModulationMatrixPanel::Row::Row()
{
    for (int i = 0; i <= static_cast<int>(ModSourceType::MPESlide); ++i)
        sourceBox.addItem(ModulationMatrix::getSourceName(static_cast<ModSourceType>(i)), i + 1);
    sourceBox.setSelectedId(1, juce::dontSendNotification);

    for (int i = 0; i <= static_cast<int>(ModTargetType::GranularPitchScatter); ++i)
        targetBox.addItem(ModulationMatrix::getTargetName(static_cast<ModTargetType>(i)), i + 1);
    targetBox.setSelectedId(1, juce::dontSendNotification);

    amountSlider.setRange(-1.0, 1.0, 0.01);
    amountSlider.setValue(0.5, juce::dontSendNotification);
    amountSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    amountSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 44, ROW_H - 6);

    addAndMakeVisible(sourceBox);
    addAndMakeVisible(targetBox);
    addAndMakeVisible(amountSlider);
    addAndMakeVisible(deleteButton);

    sourceBox.onChange = [this]() {
        if (onChange)
            onChange(static_cast<ModSourceType>(sourceBox.getSelectedId() - 1),
                     static_cast<ModTargetType>(targetBox.getSelectedId() - 1),
                     (float)amountSlider.getValue());
    };

    targetBox.onChange = [this]() {
        if (onChange)
            onChange(static_cast<ModSourceType>(sourceBox.getSelectedId() - 1),
                     static_cast<ModTargetType>(targetBox.getSelectedId() - 1),
                     (float)amountSlider.getValue());
    };

    amountSlider.onValueChange = [this]() {
        if (onChange)
            onChange(static_cast<ModSourceType>(sourceBox.getSelectedId() - 1),
                     static_cast<ModTargetType>(targetBox.getSelectedId() - 1),
                     (float)amountSlider.getValue());
    };

    deleteButton.onClick = [this]() { if (onDelete) onDelete(); };

    sourceBox.setTooltip   ("Modulation source: LFO1, LFO2, Envelope, Velocity, Random…");
    targetBox.setTooltip   ("Modulation target: Pitch, Filter Cutoff, Volume, Effect parameter…");
    amountSlider.setTooltip("Modulation depth: how strongly the source affects the target (−1.0 to +1.0)");
    deleteButton.setTooltip("Remove this modulation route");
}

void ModulationMatrixPanel::Row::resized()
{
    auto b = getLocalBounds().reduced(2, 2);
    deleteButton.setBounds(b.removeFromRight(DEL_W));
    b.removeFromRight(INNER_GAP);
    sourceBox.setBounds(b.removeFromLeft(SRC_W));
    b.removeFromLeft(INNER_GAP);
    targetBox.setBounds(b.removeFromLeft(TGT_W));
    b.removeFromLeft(INNER_GAP);
    amountSlider.setBounds(b);
}

// ─── Panel ──────────────────────────────────────────────────────────────────

ModulationMatrixPanel::ModulationMatrixPanel(PluginProcessor& p, ModulationMatrix& m)
    : processorRef(p), matrix(m)
{
    titleLabel.setText("Modulation Matrix", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    addButton.onClick = [this]() {
        matrix.addConnection(ModSourceType::LFO1, ModTargetType::FilterCutoff, 0.5f);
        rebuild();
        resized();
        repaint();
    };
    addButton.setTooltip("Add a new modulation route (source → target with depth)");
    addAndMakeVisible(addButton);

    rebuild();
    startTimer(200);
}

ModulationMatrixPanel::~ModulationMatrixPanel()
{
    stopTimer();
}

void ModulationMatrixPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xFF1A1A1A));

    // Column headers — x positions mirror Row::resized() with PADDING offset
    const int rowLeft = PADDING + 2;
    g.setColour(juce::Colours::grey);
    g.setFont(juce::Font(11.0f));
    g.drawText("SOURCE", rowLeft,                                              HEADER_H, SRC_W,  COL_HDR_H, juce::Justification::centredLeft);
    g.drawText("TARGET", rowLeft + SRC_W + INNER_GAP,                         HEADER_H, TGT_W,  COL_HDR_H, juce::Justification::centredLeft);
    g.drawText("AMOUNT", rowLeft + SRC_W + INNER_GAP + TGT_W + INNER_GAP,    HEADER_H, 80,     COL_HDR_H, juce::Justification::centredLeft);

    if (rows.empty())
    {
        g.setColour(juce::Colours::darkgrey);
        g.setFont(13.0f);
        g.drawText("No connections — press + to add one",
                   getLocalBounds().withTrimmedTop(HEADER_H + COL_HDR_H + 16),
                   juce::Justification::centredTop);
    }
}

void ModulationMatrixPanel::resized()
{
    auto b = getLocalBounds().reduced(PADDING);

    auto headerRow = b.removeFromTop(HEADER_H - 8);
    titleLabel.setBounds(headerRow.withTrimmedRight(36));
    addButton.setBounds(headerRow.removeFromRight(28));

    b.removeFromTop(COL_HDR_H + 4);

    for (auto& row : rows)
    {
        row->setBounds(b.removeFromTop(ROW_H));
        b.removeFromTop(ROW_GAP);
    }
}

void ModulationMatrixPanel::timerCallback()
{
    int count = static_cast<int>(matrix.getConnections().size());
    if (count != lastConnectionCount)
    {
        rebuild();
        resized();
        repaint();
    }
}

void ModulationMatrixPanel::rebuild()
{
    for (auto& row : rows)
        removeChildComponent(row.get());
    rows.clear();

    auto connections = matrix.getConnections();
    for (int i = 0; i < static_cast<int>(connections.size()); ++i)
    {
        const auto& conn = connections[i];
        auto row = std::make_unique<Row>();

        row->sourceBox.setSelectedId(static_cast<int>(conn.source) + 1, juce::dontSendNotification);
        row->targetBox.setSelectedId(static_cast<int>(conn.target) + 1, juce::dontSendNotification);
        row->amountSlider.setValue(conn.amount, juce::dontSendNotification);

        const int idx = i;

        row->onDelete = [this, idx]() {
            matrix.removeConnection(idx);
            rebuild();
            resized();
            repaint();
        };

        row->onChange = [this, idx](ModSourceType src, ModTargetType tgt, float amount) {
            matrix.updateConnectionSource(idx, src, 0);
            matrix.updateConnectionTarget(idx, tgt, 0);
            matrix.updateConnection(idx, amount);
        };

        addAndMakeVisible(*row);
        rows.push_back(std::move(row));
    }

    lastConnectionCount = static_cast<int>(connections.size());
}
