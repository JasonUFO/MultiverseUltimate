#include "ModulationMatrixPanel.h"
#include "LFOShapeEditor.h"
#include "../MultiverseFlatTheme.h"
#include "../NeuKnob.h"
#include "../PluginProcessor.h"

namespace {
    constexpr int HEADER_H   = 36;
    constexpr int LFO_HDR_H  = 22;
    constexpr int LFO_ROW_H  = 28;
    constexpr int LFO_GAP    = 2;
    constexpr int COL_HDR_H  = 20;
    constexpr int ROW_H      = 36;
    constexpr int ROW_GAP    = 3;
    constexpr int PADDING    = 8;
    constexpr int SRC_W      = 140;
    constexpr int TGT_W      = 140;
    constexpr int DEL_W      = 28;
    constexpr int INNER_GAP  = 4;

    // LFO row column widths
    constexpr int LFO_LABEL_W   = 44;
    constexpr int LFO_RATE_W    = 100;
    constexpr int LFO_SHAPE_W   = 84;
    constexpr int LFO_SYNC_W    = 50;
    constexpr int LFO_DIV_W     = 60;
    constexpr int LFO_DRAW_W    = 42;
}

// ─── LFORow ──────────────────────────────────────────────────────────────────

ModulationMatrixPanel::LFORow::LFORow(int index, juce::AudioProcessorValueTreeState& apvts, ModulationMatrix& matrix)
    : matrixRef(matrix), lfoIdx(index)
{
    const juce::String idx = juce::String(index + 1);

    label.setText("LFO " + idx, juce::dontSendNotification);
    label.setFont(MultiverseFlatTheme::headerFont());
    label.setColour(juce::Label::textColourId, NeuKnob::getModSourceColour(
        static_cast<ModSourceType>(index < 4 ? index : index + 11)));
    label.setJustificationType(juce::Justification::centredLeft);

    rateSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    rateSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 46, LFO_ROW_H - 4);
    rateSlider.setTooltip("LFO " + idx + " rate (Hz) — overridden when SYNC is on");

    shapeCombo.addItem("Sine",     1);
    shapeCombo.addItem("Triangle", 2);
    shapeCombo.addItem("Saw",      3);
    shapeCombo.addItem("Square",   4);
    shapeCombo.addItem("S&H",      5);
    shapeCombo.addItem("Custom",   6);
    shapeCombo.setTooltip("LFO " + idx + " waveform shape");

    syncButton.setButtonText("SYNC");
    syncButton.setTooltip("Sync LFO " + idx + " rate to DAW tempo");

    syncDivCombo.addItem("1/32", 1);
    syncDivCombo.addItem("1/16", 2);
    syncDivCombo.addItem("1/8",  3);
    syncDivCombo.addItem("1/4",  4);
    syncDivCombo.addItem("1/2",  5);
    syncDivCombo.addItem("1/1",  6);
    syncDivCombo.addItem("2/1",  7);
    syncDivCombo.addItem("4/1",  8);
    syncDivCombo.setTooltip("LFO " + idx + " tempo-sync division");

    drawButton.setTooltip("Draw a custom LFO shape");

    addAndMakeVisible(label);
    addAndMakeVisible(rateSlider);
    addAndMakeVisible(shapeCombo);
    addAndMakeVisible(syncButton);
    addAndMakeVisible(syncDivCombo);
    addAndMakeVisible(drawButton);

    if (auto* rateParam = apvts.getParameter("lfo" + idx + "Rate"))
        rateAttachment = std::make_unique<juce::SliderParameterAttachment>(*rateParam, rateSlider, nullptr);
    if (auto* shapeParam = apvts.getParameter("lfo" + idx + "Shape"))
        shapeAttachment = std::make_unique<juce::ComboBoxParameterAttachment>(*shapeParam, shapeCombo, nullptr);
    if (auto* syncParam = apvts.getParameter("lfo" + idx + "Sync"))
        syncAttachment = std::make_unique<juce::ButtonParameterAttachment>(*syncParam, syncButton, nullptr);
    if (auto* divParam = apvts.getParameter("lfo" + idx + "SyncDiv"))
        syncDivAttachment = std::make_unique<juce::ComboBoxParameterAttachment>(*divParam, syncDivCombo, nullptr);

    // Show DRAW button only when Custom is selected
    shapeCombo.onChange = [this] { updateDrawButtonState(); };
    updateDrawButtonState();

    // Open LFOShapeEditor in a CallOutBox when DRAW is clicked
    drawButton.onClick = [this] {
        auto editor = std::make_unique<LFOShapeEditor>();
        editor->setTable(matrixRef.getCustomTable(lfoIdx));
        editor->onTableChanged = [this](const std::array<float, 256>& t) {
            matrixRef.setCustomTable(lfoIdx, t);
        };
        editor->setSize(310, 160);
        juce::CallOutBox::launchAsynchronously(
            std::move(editor), drawButton.getScreenBounds(), nullptr);
    };
}

void ModulationMatrixPanel::LFORow::resized()
{
    auto b = getLocalBounds().reduced(2, 2);
    label.setBounds(b.removeFromLeft(LFO_LABEL_W));
    b.removeFromLeft(INNER_GAP);
    rateSlider.setBounds(b.removeFromLeft(LFO_RATE_W));
    b.removeFromLeft(INNER_GAP);
    shapeCombo.setBounds(b.removeFromLeft(LFO_SHAPE_W));
    b.removeFromLeft(INNER_GAP);
    syncButton.setBounds(b.removeFromLeft(LFO_SYNC_W));
    b.removeFromLeft(INNER_GAP);
    syncDivCombo.setBounds(b.removeFromLeft(LFO_DIV_W));
    b.removeFromLeft(INNER_GAP);
    drawButton.setBounds(b.removeFromLeft(LFO_DRAW_W));
}

void ModulationMatrixPanel::LFORow::mouseDrag(const juce::MouseEvent& e)
{
    // Only start drag if dragging from the label area
    if (!label.getBounds().contains(e.getMouseDownPosition()))
        return;

    if (auto* container = juce::DragAndDropContainer::findParentDragContainerFor(this))
    {
        // Map LFO index to ModSourceType (LFO1-4 = 0-3, LFO5-8 = 15-18)
        static const int lfoSourceMap[] = { 0, 1, 2, 3, 15, 16, 17, 18 };
        int sourceInt = (lfoIdx >= 0 && lfoIdx < 8) ? lfoSourceMap[lfoIdx] : 0;

        auto dragDescription = juce::String("modsrc:") + juce::String(sourceInt);
        container->startDragging(dragDescription, this);
    }
}

void ModulationMatrixPanel::LFORow::updateDrawButtonState()
{
    // selectedId 6 = "Custom" (1-based)
    const bool isCustom = (shapeCombo.getSelectedId() == 6);
    drawButton.setEnabled(isCustom);
    drawButton.setAlpha(isCustom ? 1.0f : 0.35f);
}

// ─── Row ─────────────────────────────────────────────────────────────────────

ModulationMatrixPanel::Row::Row()
{
    for (int i = 0; i < MAX_MOD_SOURCES; ++i)
    {
        const char* name = ModulationMatrix::getSourceName(static_cast<ModSourceType>(i));
        if (name && name[0] != '\0')
            sourceBox.addItem(name, i + 1);
    }
    sourceBox.setSelectedId(1, juce::dontSendNotification);

    for (int i = 0; i < MAX_MOD_TARGETS; ++i)
    {
        const char* name = ModulationMatrix::getTargetName(static_cast<ModTargetType>(i));
        if (name && name[0] != '\0')
            targetBox.addItem(name, i + 1);
    }
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

    sourceBox.setTooltip   ("Modulation source: LFO1-8, Envelopes, Velocity, Random…");
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

// ─── Panel ───────────────────────────────────────────────────────────────────

ModulationMatrixPanel::ModulationMatrixPanel(PluginProcessor& p, ModulationMatrix& m)
    : processorRef(p), matrix(m)
{
    titleLabel.setText("Modulation Matrix", juce::dontSendNotification);
    titleLabel.setFont(MultiverseFlatTheme::titleFont());
    titleLabel.setColour(juce::Label::textColourId, MultiverseFlatTheme::textPrimary);
    addAndMakeVisible(titleLabel);

    addButton.onClick = [this]() {
        matrix.addConnection(ModSourceType::LFO1, ModTargetType::FilterCutoff, 0.5f);
        rebuild();
        resized();
        repaint();
    };
    addButton.setTooltip("Add a new modulation route (source → target with depth)");
    addAndMakeVisible(addButton);

    // Build LFO rows
    for (int i = 0; i < 8; ++i)
    {
        lfoRows[i] = std::make_unique<LFORow>(i, processorRef.apvts, matrix);
        addAndMakeVisible(*lfoRows[i]);
    }

    rebuild();
    startTimer(200);
}

ModulationMatrixPanel::~ModulationMatrixPanel()
{
    stopTimer();
}

void ModulationMatrixPanel::paint(juce::Graphics& g)
{
    g.fillAll(MultiverseFlatTheme::bgBase);

    // LFO section header
    const int lfoSectionTop = HEADER_H;
    g.setColour(MultiverseFlatTheme::textSecondary);
    g.setFont(MultiverseFlatTheme::headerFont());
    g.drawText("LFO BANKS", PADDING, lfoSectionTop, 80, LFO_HDR_H, juce::Justification::centredLeft);

    // Thin separator line under LFO section
    const int lfoSectionBottom = HEADER_H + LFO_HDR_H + 8 * (LFO_ROW_H + LFO_GAP) + 4;
    g.setColour(MultiverseFlatTheme::borderLight.withAlpha(0.6f));
    g.drawHorizontalLine(lfoSectionBottom, PADDING, (float)(getWidth() - PADDING));

    // Draw neumorphic cards for each LFO row
    for (const auto& row : lfoRows)
    {
        if (row)
        {
            auto b = row->getBounds().expanded(1, 0);
            MultiverseFlatTheme::drawCard (g, b.toFloat(), 5.0f);
            g.setColour(MultiverseFlatTheme::bgRaised.withAlpha(0.7f));
            g.fillRoundedRectangle(b.toFloat(), 5.0f);
        }
    }

    // Column headers for connections section
    const int connHeaderY = lfoSectionBottom + 4;
    const int rowLeft = PADDING + 2;
    g.setColour(MultiverseFlatTheme::textSecondary);
    g.setFont(MultiverseFlatTheme::labelFont());
    g.drawText("SOURCE", rowLeft,                                           connHeaderY, SRC_W, COL_HDR_H, juce::Justification::centredLeft);
    g.drawText("TARGET", rowLeft + SRC_W + INNER_GAP,                      connHeaderY, TGT_W, COL_HDR_H, juce::Justification::centredLeft);
    g.drawText("AMOUNT", rowLeft + SRC_W + INNER_GAP + TGT_W + INNER_GAP, connHeaderY, 80,   COL_HDR_H, juce::Justification::centredLeft);

    // Draw neumorphic cards for each connection row
    const float cr = 6.0f;
    for (const auto& b : rowBounds)
    {
        if (b.getHeight() <= 0) continue;
        MultiverseFlatTheme::drawCard(g, b.toFloat().reduced(1.0f), cr);
        g.setColour(MultiverseFlatTheme::bgRaised);
        g.fillRoundedRectangle(b.toFloat().reduced(1.0f), cr);
        g.setColour(MultiverseFlatTheme::borderLight.withAlpha(0.3f));
        g.drawRoundedRectangle(b.toFloat().reduced(1.5f), cr, 1.0f);
    }

    if (rows.empty())
    {
        g.setColour(MultiverseFlatTheme::textMuted);
        g.setFont(MultiverseFlatTheme::valueFont());
        g.drawText("No connections — press + to add one",
                   getLocalBounds().withTop(lfoSectionBottom + COL_HDR_H + 24),
                   juce::Justification::centredTop);
    }
}

void ModulationMatrixPanel::resized()
{
    auto b = getLocalBounds().reduced(PADDING);

    // Header row: title + add button
    auto headerRow = b.removeFromTop(HEADER_H - 8);
    titleLabel.setBounds(headerRow.withTrimmedRight(36));
    addButton.setBounds(headerRow.removeFromRight(28));

    // LFO section header gap
    b.removeFromTop(LFO_HDR_H);

    // 8 LFO rows
    for (int i = 0; i < 8; ++i)
    {
        lfoRows[i]->setBounds(b.removeFromTop(LFO_ROW_H));
        b.removeFromTop(LFO_GAP);
    }
    b.removeFromTop(8); // spacing before separator + col header

    // Connections column header + rows
    b.removeFromTop(COL_HDR_H + 4);

    rowBounds.clear();
    for (auto& row : rows)
    {
        auto rb = b.withHeight(ROW_H);
        rowBounds.push_back(rb);
        row->setBounds(rb);
        b.removeFromTop(ROW_H + ROW_GAP);
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
