/*
 * File      : font_freetype.c
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
 * 2010-09-15     Grissom      first version
 */

#include <rtthread.h>

#ifdef RTGUI_USING_TTF
#include <rtgui/rtgui.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/dc.h>
#include <rtgui/blit.h>
#include <rtgui/font.h>
#include <rtgui/font_freetype.h>

#include <ftcache.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#define LOG_ERROR

#ifdef LOG_ERROR
#define PERROR       rt_kprintf
#else
#define PERROR(...)
#endif

//#define LOG_INFO

#ifdef LOG_INFO
#define PINFO       rt_kprintf
#else
#define PINFO(...)
#endif

#define _dc_get_pixel(dc, x, y)	\
    ((dc)->pixel + (y) * (dc)->pitch + (x) * rtgui_color_get_bpp((dc)->pixel_format))

#if !defined(RT_USING_DFS) || !defined(RT_USING_DFS_ELMFAT) || (RT_DFS_ELM_CODE_PAGE != 936)
extern rt_uint32_t gb2312tounicode(const rt_uint16_t key);
#else
extern unsigned short ff_convert(unsigned short wch, int direction);
#define gb2312tounicode(code) ff_convert(code, 1)
#endif

#ifndef UTF8_TO_UNICODE

static void gbk_to_unicode(rt_uint16_t *unicode, const char *text, int len)
{
    int i;
    const unsigned char *tp = (const unsigned char *)text;
    unsigned short wch;

    for (i = 0; i < len;)
    {
        if (*tp < 0x80)
        {
            wch = *tp;
            *unicode = gb2312tounicode(wch);
            tp ++;
            i ++;
        }
        else
        {
            wch = *(tp + 1) | (*tp << 8);
            *unicode = gb2312tounicode(wch);
            tp += 2;
            i += 2;
        }

        unicode ++;
    }

    *unicode = '\0';
}

#else

static rt_uint16_t _get_unicode(const char* str, int n)
{
    int i;
    rt_uint16_t unicode = str[0] & ((1 << (8 - n)) - 1);

    for (i = 1; i<n; i++)
    {
        unicode = unicode << 6 | ((rt_uint8_t)str[i] & 0x3f);
    }

    return unicode;
}

static int utf8_to_unicode_len(const char* utf8, int len)
{
    int index = 0, unicode_len = 0;
    rt_uint8_t c;

    if (!utf8)
    {
        return 0;
    }

    if (*utf8 == 0)
    {
        return 0;
    }

    for (index = 0; index < len;)
    {
        c = *utf8;

        if ((c & 0x80) == 0)
        {
            utf8 += 1;
            index += 1;
            unicode_len += 1;
        }
        else if ((c & 0xe0) == 0xc0)
        {
            utf8 += 2;
            index += 2;
            unicode_len += 1;
        }
        else if ((c & 0xf0) == 0xe0)
        {
            utf8 += 3;
            index += 3;
            unicode_len += 1;
        }
        else if ((c & 0xf8) == 0xf0)
        {
            utf8 += 4;
            index += 4;
            unicode_len += 1;
        }
        else if ((c & 0xfc) == 0xf8)
        {
            utf8 += 5;
            index += 5;
            unicode_len += 1;
        }
        else if ((c & 0xfe) == 0xfc)
        {
            utf8 += 6;
            index += 6;
            unicode_len += 1;
        }
        else
        {
            return unicode_len;
        }
    }

    return unicode_len;
}

static int utf8_to_unicode(rt_uint16_t* unicode, const char* utf8, int len)
{
    int index = 0;
    rt_uint8_t c;

    if (!utf8 || !unicode)
    {
        return 0;
    }

    if (*utf8 == 0)
    {
        return 0;
    }

    for (index = 0; index < len;)
    {
        c = *utf8;

        if ((c & 0x80) == 0)
        {
            *unicode = _get_unicode(utf8, 1);

            utf8 += 1;
            unicode++;
            index += 1;
        }
        else if ((c & 0xe0) == 0xc0)
        {
            *unicode = _get_unicode(utf8, 2);

            utf8 += 2;
            unicode++;
            index += 2;
        }
        else if ((c & 0xf0) == 0xe0)
        {
            *unicode = _get_unicode(utf8, 3);

            utf8 += 3;
            unicode++;
            index += 3;
        }
        else if ((c & 0xf8) == 0xf0)
        {
            *unicode = _get_unicode(utf8, 4);

            utf8 += 4;
            unicode++;
            index += 4;
        }
        else if ((c & 0xfc) == 0xf8)
        {
            *unicode = _get_unicode(utf8, 5);

            utf8 += 5;
            unicode++;
            index += 5;
        }
        else if ((c & 0xfe) == 0xfc)
        {
            *unicode = _get_unicode(utf8, 6);

            utf8 += 6;
            unicode++;
            index += 6;
        }
        else
        {
            return index;
        }
    }

    return index;
}

#endif

static void _rtgui_rect_move_to_align(const rtgui_rect_t *rect, rtgui_rect_t *to, rt_uint16_t height, int align)
{
    int dw, dh;
    dw = 0;
    dh = 0;

    /* get delta width and height */
    dw = rtgui_rect_width(*rect) - rtgui_rect_width(*to);
    if (RTGUI_ALIGN_TTF_SIZE & align)
        dh = rtgui_rect_height(*rect) - height;
    else
        dh = rtgui_rect_height(*rect) - rtgui_rect_height(*to);
    if (dw < 0) dw = 0;
    if (dh < 0) dh = 0;

    PINFO(" rect align =1=>  %d %d %d %d\n", to->x1, to->y1, to->x2, to->y2);

    /* move to insider of rect */
    if (RTGUI_ALIGN_TTF_SIZE & align)
        rtgui_rect_move_to_point(to, rect->x1, rect->y1 + height * 3 / 4 - to->y2);
    else
        rtgui_rect_move_to_point(to, rect->x1, rect->y1);
    PINFO(" rect align =2=>  %d %d %d %d\n", to->x1, to->y1, to->x2, to->y2);

    /* align to right */
    if (align & RTGUI_ALIGN_RIGHT)
    {
        to->x1 += dw;
        to->x2 += dw;
    }
    /* align to center horizontal */
    else if (align & RTGUI_ALIGN_CENTER_HORIZONTAL)
    {
        to->x1 += dw >> 1;
        to->x2 += dw >> 1;
    }

    /* align to bottom */
    if (align & RTGUI_ALIGN_BOTTOM)
    {
        to->y1 += dh;
        to->y2 += dh;
    }
    /* align to center vertical */
    else if (align & RTGUI_ALIGN_CENTER_VERTICAL)
    {
        to->y1 += dh >> 1;
        to->y2 += dh >> 1;
    }

    PINFO(" rect align =3=>  %d %d %d %d\n", to->x1, to->y1, to->x2, to->y2);
}

static void ftc_draw_text(struct rtgui_font *font,
                          struct rtgui_dc *dc,
                          const char *text,
                          rt_ubase_t len,
                          struct rtgui_rect *rect);
static void ftc_get_metrics(struct rtgui_font *font, const char *text, rtgui_rect_t *rect);

static const struct rtgui_font_engine ftc_engine =
{
    RT_NULL,
    RT_NULL,
    ftc_draw_text,
    ftc_get_metrics
};

struct ttf_face_id
{
    const char*  pathname;
    FT_Long      face_index;
};

struct rtgui_ttf
{
    /* the ttf list */
    rtgui_list_t 	list;
    rt_uint32_t		refer_count;

    struct ttf_face_id  face_id;
    FT_Library  		library;
    FTC_Manager 		manager;
    FTC_SBitCache 		sbit_cache;
    FTC_CMapCache 		cmap_cache;
};

struct rtgui_ttf_font
{
    struct rtgui_ttf	*ttf;
    FTC_ImageTypeRec	image_type_rec;
};

static rtgui_list_t _rtgui_ttf_list;

void rtgui_ttf_system_init(void)
{
    rtgui_list_init(&(_rtgui_ttf_list));
}

static void rtgui_ttf_system_add_ttf(struct rtgui_ttf *ttf)
{
    rtgui_list_append(&_rtgui_ttf_list, &(ttf->list));
}

static void rtgui_ttf_system_remove_ttf(struct rtgui_ttf *ttf)
{
    rtgui_list_remove(&_rtgui_ttf_list, &(ttf->list));
}

static struct rtgui_ttf *rtgui_ttf_refer(const char *family)
{
    /* search ttf */
    struct rtgui_list_node *node;
    struct rtgui_ttf *ttf;

    rtgui_list_foreach(node, &_rtgui_ttf_list)
    {
        ttf = (struct rtgui_ttf *)node;
        if (rt_strncmp(ttf->face_id.pathname, family, RT_DFS_ELM_MAX_LFN) == 0)
        {
            ttf->refer_count ++;
            return ttf;
        }
    }

    return RT_NULL;
}

static void rtgui_ttf_derefer(struct rtgui_ttf *ttf)
{
    RT_ASSERT(ttf != RT_NULL);

    ttf->refer_count --;

    /* no refer, remove ttf */
    if (ttf->refer_count == 0)
    {
        rtgui_ttf_system_remove_ttf(ttf);
    }
}


static void _get_metrics(struct rtgui_ttf_font *ttf_font, const rt_uint16_t *text_short, struct rtgui_rect *rect);

static void _draw_bitmap(struct rtgui_dc *dc,
                         FTC_SBit bitmap,
                         rt_int16_t ox, rt_int16_t btm_y,
                         rtgui_color_t fgc, rt_uint16_t right, rt_uint16_t bottom)
{
    rt_int16_t x_start, y_start;
    struct rtgui_blit_info info = { 0 };
    struct rtgui_dc_buffer *dest_buf;


    x_start = ox + bitmap->left;
    y_start = btm_y - bitmap->top;

    PINFO(" draw bitmap (x, y) -> (%d, %d)\n", x_start, y_start);

    info.a = RTGUI_RGB_A(fgc);
    info.r = RTGUI_RGB_R(fgc);
    info.g = RTGUI_RGB_G(fgc);
    info.b = RTGUI_RGB_B(fgc);

    if (dc->type == RTGUI_DC_HW)
    {
        struct rtgui_graphic_driver *hw_driver = rtgui_graphic_driver_get_default();
        dest_buf = (struct rtgui_dc_buffer*)dc;

        if (hw_driver->framebuffer != RT_NULL)
        {
            struct rtgui_widget *owner = ((struct rtgui_dc_hw*)dest_buf)->owner;

            if (x_start + bitmap->width < 0 || y_start + bitmap->height < 0
                    || x_start > owner->extent.x2 - owner->extent.x1 || y_start > owner->extent.y2 - owner->extent.y1)
                return;

            /* blit source */
            info.src = (rt_uint8_t *)bitmap->buffer;
            info.src = info.src + (y_start > 0 ? 0 : bitmap->top - btm_y) * bitmap->width + (x_start > 0 ? 0 : -x_start);
            info.src_fmt = RTGRAPHIC_PIXEL_FORMAT_ALPHA;
            info.src_w = bitmap->width - (x_start > 0 ? 0 : -x_start) - (x_start + bitmap->width < owner->extent.x2 - owner->extent.x1 ? 0 : x_start + bitmap->width - owner->extent.x2 + owner->extent.x1);
            info.src_h = bitmap->height - (y_start > 0 ? 0 : bitmap->top - btm_y) - (y_start + bitmap->height < owner->extent.y2 - owner->extent.y1 ? 0 : y_start + bitmap->height - owner->extent.y2 + owner->extent.y1);
            info.src_pitch = bitmap->width;
            info.src_skip = info.src_pitch - info.src_w;

            /* blit destination */
            info.dst = (rt_uint8_t*)hw_driver->framebuffer;
            info.dst = info.dst + (owner->extent.y1 + (y_start < 0 ? 0 : y_start)) * hw_driver->pitch + (owner->extent.x1 + (x_start < 0 ? 0 : x_start)) * rtgui_color_get_bpp(hw_driver->pixel_format);
            info.dst_fmt = hw_driver->pixel_format;
            info.dst_w = info.src_w;
            info.dst_h = info.src_h;
            info.dst_pitch = hw_driver->pitch;
            info.dst_skip = info.dst_pitch - info.dst_w * rtgui_color_get_bpp(hw_driver->pixel_format);

            rtgui_blit(&info);
        }
    }
    else if (dc->type == RTGUI_DC_BUFFER)
    {
        int max_right, max_bottom;

        dest_buf = (struct rtgui_dc_buffer*)dc;
        max_right = right > dest_buf->width ? dest_buf->width : right;
        max_bottom = bottom > dest_buf->height ? dest_buf->height : bottom;

        if (x_start + bitmap->width < 0 || y_start + bitmap->height < 0
                || x_start > max_right || y_start > max_bottom)
            return;

        /* blit source */
        info.src = (rt_uint8_t *)bitmap->buffer;
        info.src = info.src + (y_start > 0 ? 0 : -y_start) * bitmap->width + (x_start > 0 ? 0 : -x_start);
        info.src_fmt = RTGRAPHIC_PIXEL_FORMAT_ALPHA;
        info.src_w = bitmap->width - (x_start > 0 ? 0 : -x_start) - (x_start + bitmap->width < max_right ? 0 : x_start + bitmap->width - max_right);
        info.src_h = bitmap->height - (y_start > 0 ? 0 : -y_start) - (y_start + bitmap->height < max_bottom ? 0 : y_start + bitmap->height - max_bottom);
        info.src_pitch = bitmap->width;
        info.src_skip = info.src_pitch - info.src_w;

        /* blit destination */
        info.dst = _dc_get_pixel(dest_buf, (x_start < 0 ? 0 : x_start), (y_start < 0 ? 0 : y_start));
        info.dst_fmt = dest_buf->pixel_format;
        info.dst_w = info.src_w;
        info.dst_h = info.src_h;
        info.dst_pitch = dest_buf->pitch;
        info.dst_skip = info.dst_pitch - info.dst_w * rtgui_color_get_bpp(dest_buf->pixel_format);

        rtgui_blit(&info);
    }
    else
    {
        struct rtgui_rect text_rect;
        struct rtgui_point dc_point = { 0, 0 };
        struct rtgui_dc *text_dc;

        if (x_start + bitmap->width < 0 || y_start + bitmap->height < 0
                || x_start > right || y_start > bottom)
            return;

        text_dc = rtgui_dc_buffer_create_pixformat(RTGRAPHIC_PIXEL_FORMAT_ARGB888, bitmap->width, bitmap->height);
        if (text_dc)
        {
            dest_buf = (struct rtgui_dc_buffer*)text_dc;

            /* blit source */
            info.a = 0;
            info.src = (rt_uint8_t *)bitmap->buffer;
            info.src_fmt = RTGRAPHIC_PIXEL_FORMAT_ALPHA;
            info.src_w = bitmap->width;
            info.src_h = bitmap->height;
            info.src_pitch = bitmap->width;
            info.src_skip = 0;

            /* blit destination */
            info.dst = _dc_get_pixel(dest_buf, 0, 0);
            info.dst_fmt = dest_buf->pixel_format;
            info.dst_w = info.src_w;
            info.dst_h = info.src_h;
            info.dst_pitch = dest_buf->pitch;
            info.dst_skip = info.dst_pitch - info.dst_w * rtgui_color_get_bpp(dest_buf->pixel_format);

            rtgui_blit(&info);

            text_rect.x1 = x_start;
            text_rect.x2 = text_rect.x1 + bitmap->width;

            text_rect.y1 = y_start;
            text_rect.y2 = text_rect.y1 + bitmap->height;

            rtgui_dc_buffer_set_alpha(text_dc, RTGUI_RGB_A(fgc));
            rtgui_dc_blit(text_dc, &dc_point, dc, &text_rect);

            rtgui_dc_destory(text_dc);
        }
    }
}

static void _draw_text(struct rtgui_dc *dc,
                       struct rtgui_ttf_font *ttf_font,
                       rt_uint16_t *text_short,
                       rt_int16_t begin_x, rt_int16_t btm_y,
                       rtgui_color_t fgc, rt_uint16_t right, rt_uint16_t bottom)
{
    int glyphIndex;
    FTC_SBit ftcSBit = RT_NULL;
    FT_Error err = 0;

    while (*text_short)
    {
        glyphIndex = FTC_CMapCache_Lookup(ttf_font->ttf->cmap_cache, &ttf_font->ttf->face_id, 0, *text_short);

        err = FTC_SBitCache_Lookup(ttf_font->ttf->sbit_cache, &ttf_font->image_type_rec, glyphIndex, &ftcSBit, 0);
        if (err == 0 && ftcSBit->width != 0)
        {
            /* render font */
            begin_x -= (ftcSBit->left - (abs(ftcSBit->left) + 2) / 2);

            _draw_bitmap(dc, ftcSBit, begin_x, btm_y, fgc, right, bottom);

            begin_x += ftcSBit->width + ftcSBit->left;

            text_short++;
        }
        else if (*text_short == ' ')
        {
            glyphIndex = FTC_CMapCache_Lookup(ttf_font->ttf->cmap_cache, &ttf_font->ttf->face_id, 0, '-');

            err = FTC_SBitCache_Lookup(ttf_font->ttf->sbit_cache, &ttf_font->image_type_rec, glyphIndex, &ftcSBit, 0);
            if (err == 0)
            {
                begin_x += ftcSBit->width;

                text_short++;
            }
        }
    }
}

static void ftc_draw_text(struct rtgui_font *font,
                          struct rtgui_dc *dc,
                          const char *text,
                          rt_ubase_t len,
                          struct rtgui_rect *rect)
{
    rt_uint16_t *text_short;
    struct rtgui_ttf_font *ttf_font;
    rt_int16_t begin_x, btm_y;
    rt_int16_t topy;
    rtgui_color_t fgc;
    struct rtgui_rect text_rect;

    RT_ASSERT(font != RT_NULL);
    ttf_font = (struct rtgui_ttf_font *) font->data;
    RT_ASSERT(ttf_font != RT_NULL);

    /* allocate unicode buffer */
#ifndef UTF8_TO_UNICODE
    text_short = (rt_uint16_t *)rtgui_malloc((len + 1) * 2);
#else
    text_short = (rt_uint16_t *)rtgui_malloc((utf8_to_unicode_len(text, len) + 1) * 2);
#endif
    if (text_short == RT_NULL)
        return; /* out of memory */
#ifndef UTF8_TO_UNICODE
    rt_memset(text_short, 0x00, (len + 1) * 2);
#else
    rt_memset(text_short, 0x00, (utf8_to_unicode_len(text, len) + 1) * 2);
#endif

    RT_ASSERT(rect);

    /* convert gbk to unicode */
#ifndef UTF8_TO_UNICODE
    gbk_to_unicode(text_short, text, len);
#else
    utf8_to_unicode(text_short, text, len);
#endif

    fgc = RTGUI_DC_FC(dc);

    /* text align */
    _get_metrics(ttf_font, text_short, &text_rect);

    topy = text_rect.y1;

    _rtgui_rect_move_to_align(rect, &text_rect, font->height, RTGUI_DC_TEXTALIGN(dc));

    btm_y = topy + text_rect.y2;
    if (btm_y <= 0)
        return;
    begin_x = text_rect.x1;

    if (dc->type == RTGUI_DC_BUFFER)
    {
        struct rtgui_dc_buffer *bdc = (struct rtgui_dc_buffer *) dc;

        if (begin_x >= bdc->width || topy >= bdc->height)
        {
            goto _out;
        }
    }
    else if (dc->type == RTGUI_DC_HW)
    {
        int x, y;
        struct rtgui_dc_hw *hdc = (struct rtgui_dc_hw *) dc;

        x = begin_x + hdc->owner->extent.x1;
        y = topy + hdc->owner->extent.y1;
        if (x >= hdc->owner->extent.x2)
            goto _out;
        if (y >= hdc->owner->extent.y2)
            goto _out;
    }

    _draw_text(dc, ttf_font, text_short, begin_x, btm_y, fgc, rect->x2, rect->y2);

_out:
    /* release unicode buffer */
    rtgui_free(text_short);
}

static void _get_metrics(struct rtgui_ttf_font *ttf_font, const rt_uint16_t *text_short, struct rtgui_rect *rect)
{
    FTC_SBit ftcSBit = RT_NULL;
    rt_int16_t w = 0, top = 0, btm = 0;

    while (*text_short)
    {
        FT_Error err = 0;
        int glyphIndex;

        glyphIndex = FTC_CMapCache_Lookup(ttf_font->ttf->cmap_cache, &ttf_font->ttf->face_id, 0, *text_short);

        err = FTC_SBitCache_Lookup(ttf_font->ttf->sbit_cache, &ttf_font->image_type_rec, glyphIndex, &ftcSBit, 0);
        if (err == 0 && ftcSBit->width != 0)
        {
            w -= (ftcSBit->left - (abs(ftcSBit->left) + 2) / 2);
            w += ftcSBit->width + ftcSBit->left;

            top = top > ftcSBit->top ? top : ftcSBit->top;
            btm = (ftcSBit->top - ftcSBit->height) > btm ? btm : (ftcSBit->top - ftcSBit->height);
        }
        else if (*text_short == ' ')
        {
            glyphIndex = FTC_CMapCache_Lookup(ttf_font->ttf->cmap_cache, &ttf_font->ttf->face_id, 0, '-');

            err = FTC_SBitCache_Lookup(ttf_font->ttf->sbit_cache, &ttf_font->image_type_rec, glyphIndex, &ftcSBit, 0);
            if (err == 0)
            {
                w += ftcSBit->width;
            }
        }
        PINFO(" bitmap:(%d, %d, %d, %d)\n", ftcSBit->left, ftcSBit->top - ftcSBit->height, ftcSBit->width, ftcSBit->height);

        text_short ++;
    }

    w += ftcSBit->left - (ftcSBit->left - (abs(ftcSBit->left) + 2) / 2);

    rect->x1 = 0;
    rect->y1 = btm;
    rect->x2 = w;
    rect->y2 = top;

    PINFO(" _get_metrics: %d %d %d %d\n", rect->x1, rect->y1, rect->x2, rect->y2);
}

static void ftc_get_metrics(struct rtgui_font *font, const char *text, struct rtgui_rect *rect)
{
    int len;
    rt_uint16_t *text_short;
    struct rtgui_ttf_font *ttf_font;

    RT_ASSERT(font != RT_NULL);
    RT_ASSERT(rect != RT_NULL);
    ttf_font = (struct rtgui_ttf_font *) font->data;
    RT_ASSERT(ttf_font != RT_NULL);

    len = strlen(text);
    if (len == 0)
        return;

    memset(rect, 0, sizeof(struct rtgui_rect));

    /* allocate unicode buffer */
#ifndef UTF8_TO_UNICODE
    text_short = (rt_uint16_t *)rtgui_malloc((len + 1) * 2);
#else
    text_short = (rt_uint16_t *)rtgui_malloc((utf8_to_unicode_len(text, len) + 1) * 2);
#endif
    if (text_short == RT_NULL)
        return; /* out of memory */

#ifndef UTF8_TO_UNICODE
    rt_memset(text_short, 0x00, (len + 1) * 2);
#else
    rt_memset(text_short, 0x00, (utf8_to_unicode_len(text, len) + 1) * 2);
#endif

    /* convert gbk to unicode */
#ifndef UTF8_TO_UNICODE
    gbk_to_unicode(text_short, text, len);
#else
    utf8_to_unicode(text_short, text, len);
#endif

    _get_metrics(ttf_font, text_short, rect);
    rtgui_rect_move_to_point(rect, 0, 0);

    PINFO(" ftc_get_metrics_kern: %d %d %d %d\n", rect->x1, rect->y1, rect->x2, rect->y2);
    /* release unicode buffer */
    rtgui_free(text_short);
}

static FT_Error ftc_face_requester(FTC_FaceID faceID, FT_Library lib, FT_Pointer reqData, FT_Face *face)
{
    int ret;

    struct ttf_face_id *face_id = (struct ttf_face_id *)faceID;

    PINFO("ftc_face_requester  %s  %d\n", face_id->pathname, face_id->face_index);
    ret = FT_New_Face(lib, (char *)face_id->pathname, face_id->face_index, face);
    if (ret != 0)
    {
        PERROR("FT_New_Face failed!\n");
        return -1;
    }

    return 0;
}

static struct rtgui_ttf *rtgui_ttf_load(const char *filename)
{
    FT_Error err = 0;
    struct rtgui_ttf *ttf;

    ttf = (struct rtgui_ttf *)rtgui_malloc(sizeof(struct rtgui_ttf));
    if (!ttf)
    {
        PERROR("rtgui_ttf_load rtgui_malloc failed!\n");
        return RT_NULL;
    }

    /* chack ttf file */
    {
        int fd = open(filename, O_RDONLY, 0);
        if (fd > 0)
        {
            close(fd);
        }
        else
        {
            PERROR("open %s failed!\n", filename);
            return RT_NULL;
        }
    }

    /* face_id init */
    ttf->face_id.pathname = rt_strdup(filename);
    ttf->face_id.face_index = 0;

    err = FT_Init_FreeType(&ttf->library);
    if (err)
    {
        PERROR("FT_Init_FreeType failed err: %d\n", err);
        goto _err_done_malloc;
    }

    err = FTC_Manager_New(ttf->library, 0, 0, 0, ftc_face_requester, 0, &ttf->manager);
    if (err != 0)
    {
        PERROR("FTC_Manager_New failed!\n");
        goto _err_done_init;
    }

    err = FTC_CMapCache_New(ttf->manager, &ttf->cmap_cache);
    if (err != 0)
    {
        PERROR("FTC_CMapCache_New failed!\n");
        goto _err_done_manager;
    }

    err = FTC_SBitCache_New(ttf->manager, &ttf->sbit_cache);
    if (err != 0)
    {
        PERROR("FTC_SBitCache_New failed!\n");
        goto _err_done_manager;
    }

    ttf->refer_count = 1;

    rtgui_ttf_system_add_ttf(ttf);

    return ttf;

_err_done_manager:
    FTC_Manager_Done(ttf->manager);
_err_done_init:
    FT_Done_FreeType(ttf->library);
_err_done_malloc:
    rt_free((void *)ttf->face_id.pathname);
    rtgui_free(ttf);

    return RT_NULL;
}
RTM_EXPORT(rtgui_ttf_load);

rtgui_font_t *rtgui_freetype_font_create(const char *filename, rt_size_t size)
{
    struct rtgui_font *font;
    struct rtgui_ttf_font *ttf_font;
    struct rtgui_ttf *ttf;

    ttf = rtgui_ttf_refer(filename);
    if (!ttf)
    {
        ttf = rtgui_ttf_load(filename);
        if (!ttf)
        {
            PERROR("rtgui_ttf_load failed!\n");
            return RT_NULL;
        }
    }
    else
    {
        font = rtgui_font_refer(filename, size);
        if (font)
        {
            if (font->height == size)
                return font;
            else
                rtgui_font_derefer(font);
        }
    }

    font = (struct rtgui_font *)rtgui_malloc(sizeof(struct rtgui_font) + sizeof(struct rtgui_ttf_font));
    if (!font)
    {
        PERROR("rtgui_ttf_load rtgui_malloc failed!\n");
        return RT_NULL;
    }

    ttf_font = (struct rtgui_ttf_font *)(font + 1);
    font->data = ttf_font;

    ttf_font->ttf = ttf;

    /* image_type_rec init */
    ttf_font->image_type_rec.face_id = &ttf_font->ttf->face_id;
    ttf_font->image_type_rec.width 	 = size;
    ttf_font->image_type_rec.height  = size;
    ttf_font->image_type_rec.flags 	 = FT_LOAD_RENDER | FT_LOAD_NO_HINTING;

    /* set user data */
    font->family = (char *)(ttf_font->ttf->face_id.pathname);
    font->height = (rt_uint16_t)ttf_font->image_type_rec.height;
    font->refer_count = 1;
    font->engine = &ftc_engine;

    /* add to system */
    rtgui_font_system_add_font(font);

    return font;
}
RTM_EXPORT(rtgui_freetype_font_create);

void rtgui_freetype_font_destroy(rtgui_font_t *font)
{
    struct rtgui_ttf_font *ttf_font;

    RT_ASSERT(font != RT_NULL);
    ttf_font = (struct rtgui_ttf_font *) font->data;
    RT_ASSERT(ttf_font != RT_NULL);

    rtgui_font_system_remove_font(font);

    rtgui_ttf_derefer(ttf_font->ttf);

    if (ttf_font->ttf->refer_count == 0)
    {
        FTC_Manager_Done(ttf_font->ttf->manager);
        FT_Done_FreeType(ttf_font->ttf->library);
        rt_free(font->family);
    }

    rtgui_free(font);
}
RTM_EXPORT(rtgui_freetype_font_destroy);

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT_ALIAS(rtgui_freetype_font_create, ffc, "create freetype font: name, size")
#endif
#endif
