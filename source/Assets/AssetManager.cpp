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

    // Specialty
    LOAD_IMG ("eq_graph",  EQ_Graph_png);
    LOAD_IMG ("lphp",      LpHp_png);
    LOAD_IMG ("wheel_sprite", wheel_png);

    #undef LOAD_IMG
}

void AssetManager::generateTintedVariants()
{
    const auto& s = MultiverseFlatTheme::skin();

    // Helper lambda: generate tinted variant only if source image exists and is valid
    auto tint = [this](const char* srcKey, const char* dstKey, juce::Colour colour, float alpha)
    {
        if (images.count(srcKey) && images.at(srcKey).isValid())
            tintedImages[dstKey] = tintImageOverlay(images.at(srcKey), colour, alpha);
    };

    // Knobs
    tint("knob_large",  "knob_large_tinted",  s.bgRaised, 0.55f);
    tint("knob_small",  "knob_small_tinted",  s.bgRaised, 0.55f);

    // Specialty
    tint("eq_graph",    "eq_graph_tinted",    s.bgDeep,      0.60f);
    tint("lphp",        "lphp_tinted",        s.accent1,     0.65f);
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

juce::Image AssetManager::tintImageOverlay (const juce::Image& source, juce::Colour tintColor, float blendAlpha)
{
    if (! source.isValid())
        return {};

    juce::Image result (juce::Image::ARGB, source.getWidth(), source.getHeight(), true);
    juce::Image::BitmapData srcData (source, juce::Image::BitmapData::readOnly);
    juce::Image::BitmapData dstData (result, juce::Image::BitmapData::readWrite);

    const float rTint = tintColor.getFloatRed();
    const float gTint = tintColor.getFloatGreen();
    const float bTint = tintColor.getFloatBlue();

    for (int y = 0; y < source.getHeight(); ++y)
    {
        for (int x = 0; x < source.getWidth(); ++x)
        {
            juce::Colour src = srcData.getPixelColour (x, y);
            const float a = src.getFloatAlpha();
            // Alpha-composite blend: lerp source toward tint color
            const float r = src.getFloatRed()   * (1.0f - blendAlpha) + rTint * blendAlpha;
            const float g = src.getFloatGreen() * (1.0f - blendAlpha) + gTint * blendAlpha;
            const float b = src.getFloatBlue()  * (1.0f - blendAlpha) + bTint * blendAlpha;
            dstData.setPixelColour (x, y, juce::Colour::fromFloatRGBA (r, g, b, a));
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