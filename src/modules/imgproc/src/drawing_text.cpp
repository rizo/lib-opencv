// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html

#include "precomp.hpp"

#include <ctype.h>
#include <algorithm>

namespace cv
{
class FontFaceImpl
{
public:
    FontFaceImpl();

    int glyph_count;
    const unsigned int* glyph_unicode;
    const unsigned char* glyph_bitmaps;

    const unsigned char* get_glyph_bitmap(unsigned int ch) const;
};
}

#include "draw_text.h"

namespace cv
{

FontFaceImpl::FontFaceImpl()
{
    glyph_count = 0;
    glyph_unicode = 0;
    glyph_bitmaps = 0;
}

const unsigned char* FontFaceImpl::get_glyph_bitmap(unsigned int ch) const
{
    if (!glyph_count || !glyph_unicode || !glyph_bitmaps)
        return 0;

    for (int i = 0; i < glyph_count; i++)
    {
        if (glyph_unicode[i] == ch)
            return glyph_bitmaps + i * 40 * 20;
    }

    return 0;
}

FontFace::FontFace() : d(new FontFaceImpl)
{
}

FontFace::FontFace(const String& fontPath) : d(new FontFaceImpl)
{
    CV_UNUSED(fontPath);
}

FontFace::~FontFace()
{
    delete d;
}

void FontFace::set_glyph(int count, const unsigned int* unicode, const unsigned char* bitmaps)
{
    d->glyph_count = count;
    d->glyph_unicode = unicode;
    d->glyph_bitmaps = bitmaps;
}

static int hersheyFontPixelSize(int fontFace, double fontScale)
{
    return cvRound((fontFace == FONT_HERSHEY_PLAIN ? 8 : (fontFace == FONT_HERSHEY_COMPLEX_SMALL ? 12 : 20)) * fontScale);
}

void putText(InputOutputArray _img, const String& text, Point org,
             int fontFace, double fontScale, Scalar color,
             int thickness, int lineType, bool bottomLeftOrigin)
{
    CV_INSTRUMENT_REGION();

    CV_UNUSED(thickness);
    CV_UNUSED(lineType);

    if (text.empty())
        return;

    Mat img = _img.getMat();
    const int fontpixelsize = hersheyFontPixelSize(fontFace, fontScale);
    const int yoffset = bottomLeftOrigin ? img.rows - org.y - fontpixelsize * 2 : org.y - fontpixelsize * 2;

    unsigned int _color = 0;
    unsigned char* border_color = (unsigned char*)&_color;

    if (img.channels() == 1)
    {
        border_color[0] = saturate_cast<uchar>(color[0]);
        draw_text_c1(img.data, img.cols, img.rows, text.c_str(), org.x, yoffset, fontpixelsize, _color);
    }
    else if (img.channels() == 3)
    {
        border_color[0] = saturate_cast<uchar>(color[0]);
        border_color[1] = saturate_cast<uchar>(color[1]);
        border_color[2] = saturate_cast<uchar>(color[2]);
        draw_text_c3(img.data, img.cols, img.rows, text.c_str(), org.x, yoffset, fontpixelsize, _color);
    }
    else if (img.channels() == 4)
    {
        border_color[0] = saturate_cast<uchar>(color[0]);
        border_color[1] = saturate_cast<uchar>(color[1]);
        border_color[2] = saturate_cast<uchar>(color[2]);
        border_color[3] = saturate_cast<uchar>(color[3]);
        draw_text_c4(img.data, img.cols, img.rows, text.c_str(), org.x, yoffset, fontpixelsize, _color);
    }
}

Size getTextSize(const String& text, int fontFace, double fontScale, int thickness, int* _base_line)
{
    CV_UNUSED(thickness);

    const int fontpixelsize = hersheyFontPixelSize(fontFace, fontScale);

    int w = 0;
    int h = 0;
    get_text_drawing_size(text.c_str(), fontpixelsize, &w, &h);

    if (_base_line)
        *_base_line = 0;

    return Size(w, h);
}

double getFontScaleFromHeight(const int fontFace, const int pixelHeight, const int thickness)
{
    CV_UNUSED(thickness);
    return pixelHeight / (fontFace == FONT_HERSHEY_PLAIN ? 16.0 : (fontFace == FONT_HERSHEY_COMPLEX_SMALL ? 24.0 : 40.0));
}

Point putText(InputOutputArray _img, const String& text, Point org, Scalar color,
              const FontFace& fontface, int size, int weight, PutTextFlags flags, Range wrap)
{
    CV_INSTRUMENT_REGION();

    CV_UNUSED(weight);
    CV_UNUSED(flags);
    CV_UNUSED(wrap);

    if (text.empty())
        return org;

    Mat img = _img.getMat();
    const int yoffset = org.y - size * 2;

    unsigned int _color = 0;
    unsigned char* border_color = (unsigned char*)&_color;

    if (img.channels() == 1)
    {
        border_color[0] = saturate_cast<uchar>(color[0]);
        draw_text_c1(img.data, img.cols, img.rows, text.c_str(), org.x, yoffset, size, _color, fontface);
    }
    else if (img.channels() == 3)
    {
        border_color[0] = saturate_cast<uchar>(color[0]);
        border_color[1] = saturate_cast<uchar>(color[1]);
        border_color[2] = saturate_cast<uchar>(color[2]);
        draw_text_c3(img.data, img.cols, img.rows, text.c_str(), org.x, yoffset, size, _color, fontface);
    }
    else if (img.channels() == 4)
    {
        border_color[0] = saturate_cast<uchar>(color[0]);
        border_color[1] = saturate_cast<uchar>(color[1]);
        border_color[2] = saturate_cast<uchar>(color[2]);
        border_color[3] = saturate_cast<uchar>(color[3]);
        draw_text_c4(img.data, img.cols, img.rows, text.c_str(), org.x, yoffset, size, _color, fontface);
    }

    int w = 0;
    int h = 0;
    get_text_drawing_size(text.c_str(), size, &w, &h, fontface);

    return Point(org.x + w, org.y);
}

Rect getTextSize(Size imgsize, const String& text, Point org,
                 const FontFace& fontface, int size,
                 int weight, PutTextFlags flags, Range wrap)
{
    CV_UNUSED(imgsize);
    CV_UNUSED(weight);
    CV_UNUSED(flags);
    CV_UNUSED(wrap);

    int w = 0;
    int h = 0;
    get_text_drawing_size(text.c_str(), size, &w, &h, fontface);

    return Rect(org.x, org.y - size * 2, w, h);
}

}

