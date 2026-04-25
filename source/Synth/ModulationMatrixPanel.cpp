#include "ModulationMatrixPanel.h"

namespace
{
    constexpr int SOURCE_COL_WIDTH = 140;
    constexpr int TARGET_COL_WIDTH = 140;
    constexpr int ITEM_HEIGHT = 32;
    constexpr int ITEM_PADDING = 4;
    constexpr int HEADER_HEIGHT = 40;
    constexpr int PADDING = 8;
}

ModulationMatrixPanel::SourceComponent::SourceComponent (ModulationMatrix& m)
    : matrix (m)
{
    setInterceptsMouseClicks (true, true);
}

void ModulationMatrixPanel::SourceComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    g.setColour (juce::Colours::cyan.withAlpha (0.3f));
    g.fillRoundedRectangle (bounds.toFloat(), 4.0f);

    g.setColour (juce::Colours::white);
    g.setFont (14.0f);
    auto text = matrix.getSourceName (sourceType);
    g.drawText (text, bounds.reduced (8, 0), juce::Justification::centredLeft);
}

void ModulationMatrixPanel::SourceComponent::mouseDown (const juce::MouseEvent& e)
{
    if (e.mods.isLeftButtonDown())
    {
        if (onDragStarted)
            onDragStarted();
    }
}

void ModulationMatrixPanel::SourceComponent::mouseDrag (const juce::MouseEvent&)
{
}

void ModulationMatrixPanel::SourceComponent::mouseUp (const juce::MouseEvent&)
{
    if (onDragEnded)
        onDragEnded (sourceType, 0.0f);
}

ModulationMatrixPanel::TargetComponent::TargetComponent (ModulationMatrix& m)
    : matrix (m)
{
    setWantsKeyboardFocus (false);
}

void ModulationMatrixPanel::TargetComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto color = isDragOver ? juce::Colours::lime : juce::Colours::orange;
    g.setColour (color.withAlpha (isDragOver ? 0.5f : 0.3f));
    g.fillRoundedRectangle (bounds.toFloat(), 4.0f);

    g.setColour (juce::Colours::white);
    g.setFont (14.0f);
    auto text = matrix.getTargetName (targetType);
    g.drawText (text, bounds.reduced (8, 0), juce::Justification::centredLeft);
}

void ModulationMatrixPanel::TargetComponent::mouseDown (const juce::MouseEvent& e)
{
    if (onClicked)
        onClicked();
}

void ModulationMatrixPanel::TargetComponent::mouseEnter (const juce::MouseEvent&)
{
    isDragOver = true;
    repaint();
}

void ModulationMatrixPanel::TargetComponent::mouseExit (const juce::MouseEvent&)
{
    isDragOver = false;
    repaint();
}

ModulationMatrixPanel::ConnectionItem::ConnectionItem (ModulationMatrix& m)
    : matrix (m)
{
    addAndMakeVisible (amountSlider);
    amountSlider.setRange (-1.0f, 1.0f, 0.01f);
    amountSlider.setValue (currentAmount);
    amountSlider.addListener (this);

    addAndMakeVisible (sourceLabel);
    addAndMakeVisible (targetLabel);
    addAndMakeVisible (deleteButton);

    deleteButton.onClick = [this]() { if (onDelete) onDelete(); };
}

void ModulationMatrixPanel::ConnectionItem::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    g.setColour (juce::Colours::darkgrey.withAlpha (0.5f));
    g.fillRoundedRectangle (bounds.toFloat(), 4.0f);

    g.setColour (juce::Colours::lightgrey);
    g.setFont (12.0f);
    g.drawText (sourceLabel.getText(), bounds.removeFromLeft (SOURCE_COL_WIDTH).reduced (4, 0), juce::Justification::centredLeft);
    g.drawText (targetLabel.getText(), bounds.removeFromLeft (TARGET_COL_WIDTH).reduced (4, 0), juce::Justification::centredLeft);
}

void ModulationMatrixPanel::ConnectionItem::mouseDown (const juce::MouseEvent& e)
{
    if (e.mods.isRightButtonDown())
    {
        if (onDelete)
            onDelete();
    }
}

void ModulationMatrixPanel::ConnectionItem::resized()
{
    auto bounds = getLocalBounds();
    bounds.reduce (4, 4);

    auto sliderWidth = 80;
    deleteButton.setBounds (bounds.removeFromRight (20));
    amountSlider.setBounds (bounds.removeFromRight (sliderWidth));
}

void ModulationMatrixPanel::ConnectionItem::sliderValueChanged (juce::Slider* slider)
{
    currentAmount = slider->getValue();
    if (onAmountChanged)
        onAmountChanged (currentAmount);
}

ModulationMatrixPanel::ModulationMatrixPanel (ModulationMatrix& m)
    : matrix (m)
{
    addAndMakeVisible (titleLabel);
    titleLabel.setText ("Modulation Matrix", juce::dontSendNotification);
    titleLabel.setFont (18.0f);
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::white);

    addAndMakeVisible (addConnectionButton);
    addConnectionButton.onClick = [this]() { createConnection (0, 0); };

    startTimer (100);
}

ModulationMatrixPanel::~ModulationMatrixPanel()
{
    stopTimer();
}

void ModulationMatrixPanel::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    g.setColour (juce::Colour (0xFF1A1A1A));
    g.fillRect (bounds);

    g.setColour (juce::Colours::grey);
    g.setFont (14.0f);
    g.drawText ("SOURCES", PADDING, PADDING, SOURCE_COL_WIDTH, 20, juce::Justification::centredLeft);
    g.drawText ("TARGETS", getWidth() - PADDING - TARGET_COL_WIDTH, PADDING, TARGET_COL_WIDTH, 20, juce::Justification::centredLeft);

    auto& connections = matrix.getConnections();

    for (int i = 0; i < connections.size(); ++i)
    {
        auto& conn = connections[i];

        auto sourceY = HEADER_HEIGHT + conn.sourceIndex * (ITEM_HEIGHT + ITEM_PADDING) + ITEM_PADDING;
        auto targetY = HEADER_HEIGHT + conn.targetIndex * (ITEM_HEIGHT + ITEM_PADDING) + ITEM_PADDING;

        auto sourceX = PADDING;
        auto targetX = getWidth() - PADDING - TARGET_COL_WIDTH;

        auto colour = conn.enabled ? juce::Colours::cyan : juce::Colours::grey;
        g.setColour (colour.withAlpha (0.6f));

        juce::Path path;
        path.startNewSubPath (sourceX + SOURCE_COL_WIDTH, (float)sourceY + ITEM_HEIGHT / 2);

        auto cp1x = sourceX + SOURCE_COL_WIDTH + 30;
        auto cp2x = targetX - 30;
        path.cubicTo (cp1x, (float)sourceY + ITEM_HEIGHT / 2,
                     cp2x, (float)targetY + ITEM_HEIGHT / 2,
                     (float)targetX, (float)targetY + ITEM_HEIGHT / 2);

        g.strokePath (path, juce::PathStrokeType (2.0f));
    }
}

void ModulationMatrixPanel::resized()
{
    updateLayout();
}

void ModulationMatrixPanel::timerCallback()
{
    refreshConnections();
}

void ModulationMatrixPanel::updateLayout()
{
    auto bounds = getLocalBounds();
    bounds.reduce (PADDING, PADDING);

    titleLabel.setBounds (bounds.removeFromTop (30));
    addConnectionButton.setBounds (bounds.removeFromTop (30).removeFromRight (30));

    sourceArea = bounds.removeFromLeft (SOURCE_COL_WIDTH);
    targetArea = bounds.removeFromRight (TARGET_COL_WIDTH);
    connectionArea = bounds;

    sourceArea.removeFromTop (HEADER_HEIGHT - PADDING);
    targetArea.removeFromTop (HEADER_HEIGHT - PADDING);
}

void ModulationMatrixPanel::createConnection(int sourceIndex, int targetIndex)
{
    auto sourceType = matrix.getSourceType (sourceIndex);
    auto targetType = matrix.getTargetType (targetIndex);

    matrix.addConnection (sourceType, targetType, 0.5f);
    refreshConnections();
}

void ModulationMatrixPanel::refreshConnections()
{
    auto& connections = matrix.getConnections();

    if (static_cast<int>(connectionItems.size()) != connections.size())
    {
        connectionItems.clear();

        for (int i = 0; i < connections.size(); ++i)
        {
            auto item = std::make_unique<ConnectionItem> (matrix);
            item->connectionId = i;

            auto& conn = connections[i];
            item->sourceLabel.setText (matrix.getSourceName (conn.source), juce::dontSendNotification);
            item->targetLabel.setText (matrix.getTargetName (conn.target), juce::dontSendNotification);
            item->currentAmount = conn.amount;
            item->amountSlider.setValue (conn.amount);

            item->onDelete = [this, i]() {
                matrix.removeConnection (i);
                refreshConnections();
            };

            item->onAmountChanged = [this, i](float amount) {
                matrix.updateConnection (i, amount);
            };

            addAndMakeVisible (*item);
            connectionItems.push_back (std::move (item));
        }
    }

    auto y = HEADER_HEIGHT + PADDING;
    for (auto& item : connectionItems)
    {
        item->setBounds (PADDING, y, getWidth() - PADDING * 2, ITEM_HEIGHT);
        y += ITEM_HEIGHT + ITEM_PADDING;
    }
}

int ModulationMatrixPanel::countConnectionsForSource (ModSourceType source) const
{
    int count = 0;
    for (auto& conn : matrix.getConnections())
    {
        if (conn.source == source && conn.enabled)
            ++count;
    }
    return count;
}

int ModulationMatrixPanel::countConnectionsForTarget (ModTargetType target) const
{
    int count = 0;
    for (auto& conn : matrix.getConnections())
    {
        if (conn.target == target && conn.enabled)
            ++count;
    }
    return count;
}