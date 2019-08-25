/*
 * File      : font.c
 * This file is part of RT-Thread GUI Engine
 * COPYRIGHT (C) 2006 - 2017, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2010-09-15     Bernard      first version
 */
#include <rtgui/font.h>
#include <rtgui/dc.h>

/* bitmap font private data */

static void rtgui_bitmap_font_draw_text(struct rtgui_font *font, struct rtgui_dc *dc, const char *text, rt_ubase_t len, struct rtgui_rect *rect);
static void rtgui_bitmap_font_get_metrics(struct rtgui_font *font, const char *text, rtgui_rect_t *rect);
const struct rtgui_font_engine bmp_font_engine =
{
    RT_NULL,
    RT_NULL,
    rtgui_bitmap_font_draw_text,
    rtgui_bitmap_font_get_metrics
};
RTM_EXPORT(bmp_font_engine);

void rtgui_bitmap_font_draw_char(struct rtgui_font_bitmap *font, struct rtgui_dc *dc, const char ch,
                                 rtgui_rect_t *rect)
{
    rtgui_color_t bc;
    const rt_uint8_t *font_ptr;
    int x, y, w, h, style;
    register rt_base_t i, j, /*k,*/ word_bytes;
    struct rtgui_rect dc_rect;

    /* check first and last char */
    if (ch < font->first_char || ch > font->last_char) return;

    /* get text style */
    style = rtgui_dc_get_gc(dc)->textstyle;
    bc = rtgui_dc_get_gc(dc)->background;
    rtgui_dc_get_rect(dc, &dc_rect);

    x = rect->x1;
    y = rect->y1;
    /* get width */
    if (font->char_width == RT_NULL)
    {
        word_bytes = (((font->width - 1) / 8) + 1);
        font_ptr = font->bmp + (ch - font->first_char) * word_bytes * font->height;
    }
    else
    {
        word_bytes = ((font->char_width[ch - font->first_char] - 1) / 8) + 1;
        font_ptr = font->bmp + font->offset[ch - font->first_char];
    }
    w = (font->width  + x > rect->x2) ? rect->x2 - rect->x1 : font->width;
    h = (font->height + y > rect->y2) ? rect->y2 - rect->y1 : font->height;

    for (i = 0; i < h; i++)
    {
        rt_uint8_t chr = 0;
        const rt_uint8_t *ptr = font_ptr + i * word_bytes;

        if ((i + y) >= dc_rect.y2) continue;
        if ((i + y) < 0) continue;

        for (j = 0; j < w; j++)
        {
            if (j % 8 == 0)
                chr = *ptr++;
            if (chr & 0x80)
                rtgui_dc_draw_point(dc, j + x, i + y);
            else if (style & RTGUI_TEXTSTYLE_DRAW_BACKGROUND)
                rtgui_dc_draw_color_point(dc, j + x, i + y, bc);
            chr <<= 1;
        }
    }
}

static void rtgui_bitmap_font_draw_text(struct rtgui_font *font, struct rtgui_dc *dc,
                                        const char *text, rt_ubase_t len, struct rtgui_rect *rect)
{
    rt_uint32_t length;
    struct rtgui_rect text_rect;
    struct rtgui_font_bitmap *bmp_font = (struct rtgui_font_bitmap *)(font->data);
#ifdef RTGUI_USING_FONTHZ
    struct rtgui_font *hz_font;
#endif

    RT_ASSERT(bmp_font != RT_NULL);

    rtgui_font_get_metrics(font, text, &text_rect);
    rtgui_rect_move_to_align(rect, &text_rect, RTGUI_DC_TEXTALIGN(dc));

    /* parameter check */
    if (text_rect.y1 > text_rect.y2) return;

#ifdef RTGUI_USING_FONTHZ
    hz_font = rtgui_font_refer("hz", font->height);
    while ((text_rect.x1 < text_rect.x2) && len)
    {
        length = 0;
        while ((rt_uint8_t) * (text + length) >= 0x80) length ++; /* it's not a ascii character */
        if (length > 0)
        {
            if (hz_font != RT_NULL) rtgui_font_draw(hz_font, dc, text, length, &text_rect);
            text += length;
            len -= length;
        }

        length = 0;
        while (((rt_uint8_t) * (text + length) < 0x80) && *(text + length)) length ++;
        if (length > 0)
        {
            len -= length;
            while (length-- && text_rect.x1 < text_rect.x2)
            {
                rtgui_bitmap_font_draw_char(bmp_font, dc, *text, &text_rect);

                /* move x to next character */
                if (bmp_font->char_width == RT_NULL)
                    text_rect.x1 += bmp_font->width;
                else
                    text_rect.x1 += bmp_font->char_width[*text - bmp_font->first_char];
                text ++;
            }
        }
    }

    if (hz_font != RT_NULL) rtgui_font_derefer(hz_font);

#else
    while ((text_rect.x1 < text_rect.x2) && len)
    {
        length = 0;
        while (((rt_uint8_t) * (text + length) < 0x80) && *(text + length)) length ++;
        if (length > 0)
        {
            len -= length;
            while (length-- && text_rect.x1 < text_rect.x2)
            {
                rtgui_bitmap_font_draw_char(bmp_font, dc, *text, &text_rect);

                /* move x to next character */
                if (bmp_font->char_width == RT_NULL)
                    text_rect.x1 += bmp_font->width;
                else
                    text_rect.x1 += bmp_font->char_width[*text - bmp_font->first_char];
                text ++;
            }
        }
    }
#endif
}

static void rtgui_bitmap_font_get_metrics(struct rtgui_font *font, const char *text, rtgui_rect_t *rect)
{
    rt_uint32_t length;
    struct rtgui_font_bitmap *bmp_font = (struct rtgui_font_bitmap *)(font->data);

    RT_ASSERT(bmp_font != RT_NULL);

    /* set init metrics rect */
    rect->x1 = rect->y1 = 0;
    rect->x2 = 0;
    rect->y2 = bmp_font->height;

    while (*text)
    {
        length = 0;
        while ((rt_uint8_t) * (text + length) >= 0x80) length ++; /* it's not a ascii character */
        rect->x2 += (font->height / 2) * length;
        text += length;

        length = 0;
        while (((rt_uint8_t) * (text + length) < 0x80) && *(text + length)) length ++;
        if (bmp_font->char_width != NULL)
        {
            /* get width for each character */
            while (*text && ((rt_uint8_t)*text < 0x80))
            {
                rect->x2 += bmp_font->char_width[*text - bmp_font->first_char];
                text ++;
            }
        }
        else
        {
            /* set metrics rect */
            rect->x2 += bmp_font->width * length;
            text += length;
        }
    }
}
