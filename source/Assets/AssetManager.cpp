#include "AssetManager.h"
#include "../MultiverseFlatTheme.h"
#include "BinaryData.h"

AssetManager& AssetManager::instance()
{
    static AssetManager inst;
    return inst;
}

void AssetManager::initialise()
{
    loadAllImages();
    generateTintedVariants();
}

void AssetManager::onSkinChanged()
{
    generateTintedVariants();
}

void AssetManager::loadAllImages()
{
    // Helper macro: load from BinaryData and cache
    #define LOAD_IMG(name, binaryDataVar) \
        images[name] = juce::ImageCache::getFromMemory ( \
            BinaryData::binaryDataVar, BinaryData::binaryDataVar##Size)

    // Knobs
    LOAD_IMG ("knob_large",    Knob_png);
    LOAD_IMG ("knob_small",    Knob_Small_png);

    // Buttons
    LOAD_IMG ("button_big_on",   button_big_on_png);
    LOAD_IMG ("button_big_off",  button_big_off_png);
    LOAD_IMG ("button_normal",   Button_png);
    LOAD_IMG ("button_hover",    Button_1_png);
    LOAD_IMG ("button_pressed",  Button_2_png);
    LOAD_IMG ("button_disabled", Button_3_png);

    // Radio / toggle
    LOAD_IMG ("radio_off",  Radio_Button_png);
    LOAD_IMG ("radio_on",   Radio_Button_1_png);

    // Dropdown
    LOAD_IMG ("dropdown",        Dropdown_png);
    LOAD_IMG ("dropdown_arrow",  Dropdown_6_png);

    // Header
    LOAD_IMG ("header_big",    Header_Big_png);
    LOAD_IMG ("header_small",  Header_Small_png);

    // Panel backgrounds
    LOAD_IMG ("content",     Content_png);
    LOAD_IMG ("box",         Box_png);
    LOAD_IMG ("frame_34",    Frame_34_png);
    LOAD_IMG ("frame_38",    Frame_38_png);
    LOAD_IMG ("frame_482",   Frame_482_png);
    LOAD_IMG ("frame_488",   Frame_488_png);

    // Dividers & tabs
    LOAD_IMG ("frame_1",   Frame_1_png);
    LOAD_IMG ("frame_11",  Frame_11_png);
    LOAD_IMG ("frame_37",  Frame_37_png);

    // Specialty
    LOAD_IMG ("eq_graph",  EQ_Graph_png);
    LOAD_IMG ("lphp",      LpHp_png);
    LOAD_IMG ("wheel_sprite", wheel_png);

    #undef LOAD_IMG
}

void AssetManager::generateTintedVariants()
{
    const auto& s = MultiverseFlatTheme::skin();

    // Generate tinted box for active card state
    if (images.count ("box") && images.at ("box").isValid())
        tintedImages["box_active"] = tintImage (images.at ("box"), s.accent1.withAlpha (0.15f));
}

juce::Image AssetManager::tintImage (const juce::Image& source, juce::Colour tint)
{
    if (! source.isValid())
        return {};

    juce::Image result (juce::Image::ARGB, source.getWidth(), source.getHeight(), true);
    juce::Image::BitmapData srcData (source, juce::Image::BitmapData::readOnly);
    juce::Image::BitmapData dstData (result, juce::Image::BitmapData::readWrite);

    const float rTint = tint.getFloatRed();
    const float gTint = tint.getFloatGreen();
    const float bTint = tint.getFloatBlue();

    for (int y = 0; y < source.getHeight(); ++y)
    {
        for (int x = 0; x < source.getWidth(); ++x)
        {
            juce::Colour src = srcData.getPixelColour (x, y);
            dstData.setPixelColour (x, y, juce::Colour::fromFloatRGBA (
                src.getFloatRed()   * rTint,
                src.getFloatGreen() * gTint,
                src.getFloatBlue()  * bTint,
                src.getFloatAlpha()));
        }
    }

    return result;
}

void AssetManager::drawImageFitted (juce::Graphics& g, const juce::Image& source,
                                     juce::Rectangle<float> dest,
                                     juce::RectanglePlacement placement)
{
    if (! source.isValid())
        return;

    auto sourceRect = source.getBounds().toFloat();
    auto placed = placement.appliedTo (sourceRect, dest);
    g.drawImage (source, placed.getX(), placed.getY(), placed.getWidth(), placed.getHeight(),
                 0, 0, source.getWidth(), source.getHeight());
}

void AssetManager::drawImage9Slice (juce::Graphics& g, const juce::Image& source,
                                     juce::Rectangle<float> dest,
                                     int leftBorder, int rightBorder,
                                     int topBorder, int bottomBorder)
{
    if (! source.isValid())
        return;

    const int sw = source.getWidth();
    const int sh = source.getHeight();
    const float dw = dest.getWidth();
    const float dh = dest.getHeight();

    // Clamp borders so they don't overlap
    leftBorder   = juce::jmin (leftBorder,   sw / 2);
    rightBorder  = juce::jmin (rightBorder,  sw / 2);
    topBorder    = juce::jmin (topBorder,     sh / 2);
    bottomBorder = juce::jmin (bottomBorder,  sh / 2);

    const float scaleX = dw / (float)(sw - leftBorder - rightBorder);
    const float scaleY = dh / (float)(sh - topBorder - bottomBorder);

    // Scale factors for border regions
    const float lbW = leftBorder * scaleX;
    const float rbW = rightBorder * scaleX;
    const float tbH = topBorder * scaleY;
    const float bbH = bottomBorder * scaleY;
    const float midW = dw - lbW - rbW;
    const float midH = dh - tbH - bbH;

    const float x0 = dest.getX();
    const float y0 = dest.getY();

    // Helper lambda: draw a region of the source scaled to a destination rect
    auto drawRegion = [&] (int sx, int sy, int sWidth, int sHeight,
                           float dx, float dy, float dWidth, float dHeight)
    {
        g.drawImage (source,
                     dx, dy, dWidth, dHeight,
                     sx, sy, sWidth, sHeight);
    };

    const int midSrcW = sw - leftBorder - rightBorder;
    const int midSrcH = sh - topBorder - bottomBorder;

    // Top-left corner
    drawRegion (0, 0, leftBorder, topBorder,  x0, y0, lbW, tbH);
    // Top-right corner
    drawRegion (sw - rightBorder, 0, rightBorder, topBorder,  x0 + dw - rbW, y0, rbW, tbH);
    // Bottom-left corner
    drawRegion (0, sh - bottomBorder, leftBorder, bottomBorder,  x0, y0 + dh - bbH, lbW, bbH);
    // Bottom-right corner
    drawRegion (sw - rightBorder, sh - bottomBorder, rightBorder, bottomBorder,  x0 + dw - rbW, y0 + dh - bbH, rbW, bbH);

    // Top edge
    drawRegion (leftBorder, 0, midSrcW, topBorder,  x0 + lbW, y0, midW, tbH);
    // Bottom edge
    drawRegion (leftBorder, sh - bottomBorder, midSrcW, bottomBorder,  x0 + lbW, y0 + dh - bbH, midW, bbH);
    // Left edge
    drawRegion (0, topBorder, leftBorder, midSrcH,  x0, y0 + tbH, lbW, midH);
    // Right edge
    drawRegion (sw - rightBorder, topBorder, rightBorder, midSrcH,  x0 + dw - rbW, y0 + tbH, rbW, midH);

    // Centre
    drawRegion (leftBorder, topBorder, midSrcW, midSrcH,  x0 + lbW, y0 + tbH, midW, midH);
}

void AssetManager::drawWheelFrame (juce::Graphics& g, juce::Rectangle<float> dest,
                                    float value, bool isPitchWheel) const
{
    auto& wheelImg = images.at ("wheel_sprite");
    if (! wheelImg.isValid())
        return;

    // value: -1..+1 for pitch, 0..1 for mod
    int frameIndex;
    if (isPitchWheel)
        frameIndex = static_cast<int> (juce::jmap (value, -1.0f, 1.0f, 0.0f, static_cast<float> (WHEEL_NUM_FRAMES - 1)));
    else
        frameIndex = static_cast<int> (juce::jmap (value, 0.0f, 1.0f, 0.0f, static_cast<float> (WHEEL_NUM_FRAMES - 1)));

    frameIndex = juce::jlimit (0, WHEEL_NUM_FRAMES - 1, frameIndex);

    int frameY = frameIndex * WHEEL_FRAME_H;
    auto frame = wheelImg.getClippedImage (juce::Rectangle<int> (0, frameY, WHEEL_FRAME_W, WHEEL_FRAME_H));

    g.drawImage (frame,
                 dest.getX(), dest.getY(), dest.getWidth(), dest.getHeight(),
                 0, 0, WHEEL_FRAME_W, WHEEL_FRAME_H);
}