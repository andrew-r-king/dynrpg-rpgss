/*
    The MIT License (MIT)

    Copyright (c) 2014 Anatoli Steinmark

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#include <cassert>
#include <cstring>

#include <emmintrin.h>

#define NOT_MAIN_MODULE
#include <DynRPG/DynRPG.h>

#include "../../common/cpuinfo.hpp"
#include "../../graphics/primitives.hpp"
#include "../../graphics/Font.hpp"
#include "../../graphics/WindowSkin.hpp"
#include "Screen.hpp"


namespace rpgss {
    namespace script {
        namespace game_module {

            namespace {

                //---------------------------------------------------------
                struct rgb565_set
                {
                    __attribute__((__always_inline__))
                    void operator()(u16* dst, const graphics::RGBA* src)
                    {
                        asm (
                            "movzbl       (%1),   %%eax   \n\t"
                            "andl         $248,   %%eax   \n\t"
                            "shll           $8,   %%eax   \n\t"
                            //---------------------------------
                            "movzbl      1(%1),   %%edx   \n\t"
                            "andl         $252,   %%edx   \n\t"
                            "shll           $3,   %%edx   \n\t"
                            //---------------------------------
                            "movzbl      2(%1),   %%ecx   \n\t"
                            "shrl           $3,   %%ecx   \n\t"
                            //---------------------------------
                            "orl         %%edx,   %%eax   \n\t"
                            "orl         %%ecx,   %%eax   \n\t"
                            //---------------------------------
                            "movw         %%ax,    (%0)"
                            :
                            : "D"(dst), "S"(src)
                            : "eax", "edx", "ecx"
                        );

                        // C++ version:
                        /*
                        *dst = ((src->red & 0xF8) << 8) | ((src->green & 0xFC) << 3) | (src->blue >> 3);
                        */
                    }

                    __attribute__((__always_inline__))
                    void operator()(u16* dst, u8 red, u8 green, u8 blue, u8 alpha)
                    {
                        *dst = ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
                    }
                };

                //---------------------------------------------------------
                struct rgb565_mix
                {
                    __attribute__((__always_inline__))
                    void operator()(u16* dst, const graphics::RGBA* src)
                    {
                        u32 temp;
                        asm (
                            "movzbl      3(%1),   %%ecx   \n\t"
                            "movl         $256,   %%ebx   \n\t"
                            "subl        %%ecx,   %%ebx   \n\t"
                            "addl           $1,   %%ecx   \n\t"
                            //---------------------------------
                            "movzwl       (%0),   %%edx   \n\t"
                            "shrl          $11,   %%edx   \n\t"
                            "imull       %%ebx,   %%edx   \n\t"
                            "shrl           $8,   %%edx   \n\t"
                            //-----------------
                            "movzbl       (%1),   %%eax   \n\t"
                            "imull       %%ecx,   %%eax   \n\t"
                            "shrl          $11,   %%eax   \n\t"
                            //-----------------
                            "addl        %%edx,   %%eax   \n\t"
                            "shll          $11,   %%eax   \n\t"
                            "movl        %%eax,      %2   \n\t"
                            //---------------------------------
                            "movzwl       (%0),   %%edx   \n\t"
                            "shrl           $5,   %%edx   \n\t"
                            "andl          $63,   %%edx   \n\t"
                            "imull       %%ebx,   %%edx   \n\t"
                            "shrl           $8,   %%edx   \n\t"
                            //-----------------
                            "movzbl      1(%1),   %%eax   \n\t"
                            "imull       %%ecx,   %%eax   \n\t"
                            "shrl          $10,   %%eax   \n\t"
                            //-----------------
                            "addl        %%edx,   %%eax   \n\t"
                            "shll           $5,   %%eax   \n\t"
                            "orl         %%eax,      %2   \n\t"
                            //---------------------------------
                            "movzwl       (%0),   %%edx   \n\t"
                            "andl          $31,   %%edx   \n\t"
                            "imull       %%ebx,   %%edx   \n\t"
                            "shrl           $8,   %%edx   \n\t"
                            //-----------------
                            "movzbl      2(%1),   %%eax   \n\t"
                            "imull       %%ecx,   %%eax   \n\t"
                            "shrl          $11,   %%eax   \n\t"
                            //-----------------
                            "addl        %%edx,   %%eax   \n\t"
                            "orl            %2,   %%eax   \n\t"
                            //---------------------------------
                            "movw         %%ax,    (%0)"
                            :
                            : "D"(dst), "S"(src), "m"(temp)
                            : "eax", "ecx", "edx", "ebx"
                        );

                        // C++ version:
                        /*
                        graphics::RGBA temp;
                        int sa = src->alpha + 1;
                        int da = 256 - src->alpha;
                        temp.red   = (   (*dst >>  11)         * da + (src->red   >> 3) * sa ) >> 8;
                        temp.green = ( ( (*dst >>   5) & 0x3F) * da + (src->green >> 2) * sa ) >> 8;
                        temp.blue  = ( (  *dst         & 0x1F) * da + (src->blue  >> 3) * sa ) >> 8;
                        *dst = (temp.red << 11) | (temp.green << 5) | temp.blue;
                        */
                    }

                    __attribute__((__always_inline__))
                    void operator()(u16* dst, u8 red, u8 green, u8 blue, u8 alpha)
                    {
                        graphics::RGBA temp;
                        int sa = alpha + 1;
                        int da = 256 - alpha;
                        temp.red   = (   (*dst >> 11)         * da + (red   >> 3) * sa ) >> 8;
                        temp.green = ( ( (*dst >>  5) & 0x3F) * da + (green >> 2) * sa ) >> 8;
                        temp.blue  = ( (  *dst        & 0x1F) * da + (blue  >> 3) * sa ) >> 8;
                        *dst = (temp.red << 11) | (temp.green << 5) | temp.blue;
                    }
                };

                //---------------------------------------------------------
                struct rgb565_add
                {
                    __attribute__((__always_inline__))
                    void operator()(u16* dst, const graphics::RGBA* src)
                    {
                        asm (
                            "movzwl       (%0),   %%eax   \n\t"
                            "shrl          $11,   %%eax   \n\t"
                            "movzbl       (%1),   %%ecx   \n\t"
                            "shrl           $3,   %%ecx   \n\t"
                            "addl        %%ecx,   %%eax   \n\t"
                            "test          $32,   %%eax   \n\t"
                            "jz                      1f   \n\t"
                            "movl          $31,   %%eax   \n\t"
                            "1:                           \n\t"
                            "shll          $11,   %%eax   \n\t"
                            "movl        %%eax,   %%edx   \n\t"
                            //---------------------------------
                            "movzwl       (%0),   %%eax   \n\t"
                            "shrl           $5,   %%eax   \n\t"
                            "andl          $63,   %%eax   \n\t"
                            "movzbl      1(%1),   %%ecx   \n\t"
                            "shrl           $2,   %%ecx   \n\t"
                            "addl        %%ecx,   %%eax   \n\t"
                            "test          $64,   %%eax   \n\t"
                            "jz                      2f   \n\t"
                            "movl          $63,   %%eax   \n\t"
                            "2:                           \n\t"
                            "shll           $5,   %%eax   \n\t"
                            "orl         %%eax,   %%edx   \n\t"
                            //---------------------------------
                            "movzwl       (%0),   %%eax   \n\t"
                            "andl          $31,   %%eax   \n\t"
                            "movzbl      2(%1),   %%ecx   \n\t"
                            "shrl           $3,   %%ecx   \n\t"
                            "addl        %%ecx,   %%eax   \n\t"
                            "test          $32,   %%eax   \n\t"
                            "jz                      3f   \n\t"
                            "movl          $31,   %%eax   \n\t"
                            "3:                           \n\t"
                            "orl         %%eax,   %%edx   \n\t"
                            //---------------------------------
                            "movw         %%dx,    (%0)"
                            :
                            : "D"(dst), "S"(src)
                            : "eax", "edx", "ecx"
                        );

                        // C++ version:
                        /*
                        u8 r = std::min(  (*dst >> 11)         + (src->red   >> 3), 31);
                        u8 g = std::min(( (*dst >>  5) & 0x3F) + (src->green >> 2), 63);
                        u8 b = std::min((  *dst        & 0x1F) + (src->blue  >> 3), 31);
                        *dst = (r << 11) | (g << 5) | b;
                        */
                    }

                    __attribute__((__always_inline__))
                    void operator()(u16* dst, u8 red, u8 green, u8 blue, u8 alpha)
                    {
                        u8 r = std::min(  (*dst >> 11)         + (red   >> 3), 31);
                        u8 g = std::min(( (*dst >>  5) & 0x3F) + (green >> 2), 63);
                        u8 b = std::min((  *dst        & 0x1F) + (blue  >> 3), 31);
                        *dst = (r << 11) | (g << 5) | b;
                    }
                };

                //---------------------------------------------------------
                struct rgb565_sub
                {
                    __attribute__((__always_inline__))
                    void operator()(u16* dst, const graphics::RGBA* src)
                    {
                        asm (
                            "movzwl       (%0),   %%eax   \n\t"
                            "shrl          $11,   %%eax   \n\t"
                            "movzbl       (%1),   %%ecx   \n\t"
                            "shrl           $3,   %%ecx   \n\t"
                            "subl        %%ecx,   %%eax   \n\t"
                            "jns                     1f   \n\t"
                            "xorl        %%eax,   %%eax   \n\t"
                            "1:                           \n\t"
                            "shll          $11,   %%eax   \n\t"
                            "movl        %%eax,   %%edx   \n\t"
                            //---------------------------------
                            "movzwl       (%0),   %%eax   \n\t"
                            "shrl           $5,   %%eax   \n\t"
                            "andl          $63,   %%eax   \n\t"
                            "movzbl      1(%1),   %%ecx   \n\t"
                            "shrl           $2,   %%ecx   \n\t"
                            "subl        %%ecx,   %%eax   \n\t"
                            "jns                     2f   \n\t"
                            "xorl        %%eax,   %%eax   \n\t"
                            "2:                           \n\t"
                            "shll           $5,   %%eax   \n\t"
                            "orl         %%eax,   %%edx   \n\t"
                            //---------------------------------
                            "movzwl       (%0),   %%eax   \n\t"
                            "andl          $31,   %%eax   \n\t"
                            "movzbl      2(%1),   %%ecx   \n\t"
                            "shrl           $3,   %%ecx   \n\t"
                            "subl        %%ecx,   %%eax   \n\t"
                            "jns                     3f   \n\t"
                            "xorl        %%eax,   %%eax   \n\t"
                            "3:                           \n\t"
                            "orl         %%eax,   %%edx   \n\t"
                            //---------------------------------
                            "movw         %%dx,    (%0)"
                            :
                            : "D"(dst), "S"(src)
                            : "eax", "edx", "ecx"
                        );

                        // C++ version:
                        /*
                        u8 r = std::max(  (*dst >> 11)         - (src->red   >> 3), 0);
                        u8 g = std::max(( (*dst >>  5) & 0x3F) - (src->green >> 2), 0);
                        u8 b = std::max((  *dst        & 0x1F) - (src->blue  >> 3), 0);
                        *dst = (r << 11) | (g << 5) | b;
                        */
                    }

                    __attribute__((__always_inline__))
                    void operator()(u16* dst, u8 red, u8 green, u8 blue, u8 alpha)
                    {
                        u8 r = std::max(  (*dst >> 11)         - (red   >> 3), 0);
                        u8 g = std::max(( (*dst >>  5) & 0x3F) - (green >> 2), 0);
                        u8 b = std::max((  *dst        & 0x1F) - (blue  >> 3), 0);
                        *dst = (r << 11) | (g << 5) | b;
                    }
                };

                //---------------------------------------------------------
                struct rgb565_mul
                {
                    __attribute__((__always_inline__))
                    void operator()(u16* dst, const graphics::RGBA* src)
                    {
                        asm (
                            "movzwl       (%0),   %%eax   \n\t"
                            "andl       $63488,   %%eax   \n\t"
                            "movzbl       (%1),   %%ecx   \n\t"
                            "addl           $1,   %%ecx   \n\t"
                            "imull       %%ecx,   %%eax   \n\t"
                            "shrl           $8,   %%eax   \n\t"
                            "andl       $63488,   %%eax   \n\t"
                            "movl        %%eax,   %%edx   \n\t"
                            //---------------------------------
                            "movzwl       (%0),   %%eax   \n\t"
                            "andl        $2016,   %%eax   \n\t"
                            "movzbl      1(%1),   %%ecx   \n\t"
                            "addl           $1,   %%ecx   \n\t"
                            "imull       %%ecx,   %%eax   \n\t"
                            "shrl           $8,   %%eax   \n\t"
                            "andl        $2016,   %%eax   \n\t"
                            "orl         %%eax,   %%edx   \n\t"
                            //---------------------------------
                            "movzwl       (%0),   %%eax   \n\t"
                            "andl          $31,   %%eax   \n\t"
                            "movzbl      2(%1),   %%ecx   \n\t"
                            "addl           $1,   %%ecx   \n\t"
                            "imull       %%ecx,   %%eax   \n\t"
                            "shrl           $8,   %%eax   \n\t"
                            "orl         %%edx,   %%eax   \n\t"
                            //---------------------------------
                            "movw         %%ax,    (%0)"
                            :
                            : "D"(dst), "S"(src)
                            : "eax", "edx", "ecx"
                        );

                        // C++ version:
                        /*
                        u8 r =   (*dst >> 11)         * ((src->red   >> 3) + 1) >> 5;
                        u8 g = ( (*dst >>  5) & 0x3F) * ((src->green >> 2) + 1) >> 6;
                        u8 b = (  *dst        & 0x1F) * ((src->blue  >> 3) + 1) >> 5;
                        *dst = (r << 11) | (g << 5) | b;
                        */
                    }

                    __attribute__((__always_inline__))
                    void operator()(u16* dst, u8 red, u8 green, u8 blue, u8 alpha)
                    {
                        u8 r =   (*dst >> 11)         * ((red   >> 3) + 1) >> 5;
                        u8 g = ( (*dst >>  5) & 0x3F) * ((green >> 2) + 1) >> 6;
                        u8 b = (  *dst        & 0x1F) * ((blue  >> 3) + 1) >> 5;
                        *dst = (r << 11) | (g << 5) | b;
                    }
                };

                //---------------------------------------------------------
                struct rgb565_set_col
                {
                    u32 cr;
                    u32 cg;
                    u32 cb;

                    explicit rgb565_set_col(graphics::RGBA color)
                    {
                        cr = color.red   + 1;
                        cg = color.green + 1;
                        cb = color.blue  + 1;
                    }

                    __attribute__((__always_inline__))
                    void operator()(u16* dst, const graphics::RGBA* src)
                    {
                        asm (
                            "movzbl       (%1),   %%eax   \n\t"
                            "imull          %2,   %%eax   \n\t"
                            "andl       $63488,   %%eax   \n\t"
                            //---------------------------------
                            "movzbl      1(%1),   %%edx   \n\t"
                            "imull          %3,   %%edx   \n\t"
                            "shrl          $10,   %%edx   \n\t"
                            "shll           $5,   %%edx   \n\t"
                            //---------------------------------
                            "movzbl      2(%1),   %%ecx   \n\t"
                            "imull          %4,   %%ecx   \n\t"
                            "shrl          $11,   %%ecx   \n\t"
                            //---------------------------------
                            "orl         %%edx,   %%eax   \n\t"
                            "orl         %%ecx,   %%eax   \n\t"
                            //---------------------------------
                            "movw         %%ax,    (%0)"
                            :
                            : "D"(dst), "S"(src), "g"(cr), "g"(cg), "g"(cb)
                            : "eax", "edx", "ecx"
                        );

                        // C++ version:
                        /*
                        u8 r = ((src->red   >> 3) * cr) >> 5;
                        u8 g = ((src->green >> 2) * cg) >> 6;
                        u8 b = ((src->blue  >> 3) * cb) >> 5;
                        *dst = (r << 11) | (g << 5) | b;
                        */
                    }
                };

                //---------------------------------------------------------
                struct rgb565_mix_col
                {
                    u32 cr;
                    u32 cg;
                    u32 cb;
                    u32 ca;

                    explicit rgb565_mix_col(graphics::RGBA color)
                    {
                        cr = color.red   + 1;
                        cg = color.green + 1;
                        cb = color.blue  + 1;
                        ca = color.alpha + 1;
                    }

                    __attribute__((__always_inline__))
                    void operator()(u16* dst, const graphics::RGBA* src)
                    {
                        u32 temp = 0;
                        asm (
                            "movzbl      3(%1),   %%ecx   \n\t"
                            "imull          %6,   %%ecx   \n\t"
                            "shrl           $8,   %%ecx   \n\t"
                            "movl         $256,   %%ebx   \n\t"
                            "subl        %%ecx,   %%ebx   \n\t"
                            "addl           $1,   %%ecx   \n\t"
                            //---------------------------------
                            "movzwl       (%0),   %%edx   \n\t"
                            "shrl          $11,   %%edx   \n\t"
                            "imull       %%ebx,   %%edx   \n\t"
                            "shrl           $8,   %%edx   \n\t"
                            //-----------------
                            "movzbl       (%1),   %%eax   \n\t"
                            "imull          %3,   %%eax   \n\t"
                            "imull       %%ecx,   %%eax   \n\t"
                            "shrl          $19,   %%eax   \n\t"
                            //-----------------
                            "addl        %%edx,   %%eax   \n\t"
                            "shll          $11,   %%eax   \n\t"
                            "orl         %%eax,      %2   \n\t"
                            //---------------------------------
                            "movzwl       (%0),   %%edx   \n\t"
                            "shrl           $5,   %%edx   \n\t"
                            "andl          $63,   %%edx   \n\t"
                            "imull       %%ebx,   %%edx   \n\t"
                            "shrl           $8,   %%edx   \n\t"
                            //-----------------
                            "movzbl      1(%1),   %%eax   \n\t"
                            "imull          %4,   %%eax   \n\t"
                            "imull       %%ecx,   %%eax   \n\t"
                            "shrl          $18,   %%eax   \n\t"
                            //-----------------
                            "addl        %%edx,   %%eax   \n\t"
                            "shll           $5,   %%eax   \n\t"
                            "orl         %%eax,      %2   \n\t"
                            //---------------------------------
                            "movzwl       (%0),   %%edx   \n\t"
                            "andl          $31,   %%edx   \n\t"
                            "imull       %%ebx,   %%edx   \n\t"
                            "shrl           $8,   %%edx   \n\t"
                            //-----------------
                            "movzbl      2(%1),   %%eax   \n\t"
                            "imull          %5,   %%eax   \n\t"
                            "imull       %%ecx,   %%eax   \n\t"
                            "shrl          $19,   %%eax   \n\t"
                            //-----------------
                            "addl        %%edx,   %%eax   \n\t"
                            "orl            %2,   %%eax   \n\t"
                            //---------------------------------
                            "movw         %%ax,    (%0)"
                            :
                            : "D"(dst), "S"(src), "m"(temp), "g"(cr), "g"(cg), "g"(cb), "g"(ca)
                            : "eax", "ecx", "edx", "ebx"
                        );

                        // C++ version:
                        /*
                        int sa = (src->alpha * (c.alpha + 1) >> 8) + 1;
                        int da = 256 - (sa - 1);
                        u8 r = (   (*dst >>  11)         * da + (((src->red   >> 3) * c.red  ) >> 5) * sa ) >> 8;
                        u8 g = ( ( (*dst >>   5) & 0x3F) * da + (((src->green >> 2) * c.green) >> 6) * sa ) >> 8;
                        u8 b = ( (  *dst         & 0x1F) * da + (((src->blue  >> 3) * c.blue ) >> 5) * sa ) >> 8;
                        *dst = (r << 11) | (g << 5) | b;
                        */
                    }
                };

                //---------------------------------------------------------
                struct rgb565_add_col
                {
                    u32 cr;
                    u32 cg;
                    u32 cb;

                    explicit rgb565_add_col(graphics::RGBA color)
                    {
                        cr = color.red   + 1;
                        cg = color.green + 1;
                        cb = color.blue  + 1;
                    }

                    __attribute__((__always_inline__))
                    void operator()(u16* dst, const graphics::RGBA* src)
                    {
                        asm (
                            "movzwl       (%0),   %%eax   \n\t"
                            "shrl          $11,   %%eax   \n\t"
                            "movzbl       (%1),   %%ecx   \n\t"
                            "imull          %2,   %%ecx   \n\t"
                            "shrl          $11,   %%ecx   \n\t"
                            "addl        %%ecx,   %%eax   \n\t"
                            "test          $32,   %%eax   \n\t"
                            "jz                      1f   \n\t"
                            "movl          $31,   %%eax   \n\t"
                            "1:                           \n\t"
                            "shll          $11,   %%eax   \n\t"
                            "movl        %%eax,   %%edx   \n\t"
                            //---------------------------------
                            "movzwl       (%0),   %%eax   \n\t"
                            "shrl           $5,   %%eax   \n\t"
                            "andl          $63,   %%eax   \n\t"
                            "movzbl      1(%1),   %%ecx   \n\t"
                            "imull          %3,   %%ecx   \n\t"
                            "shrl          $10,   %%ecx   \n\t"
                            "addl        %%ecx,   %%eax   \n\t"
                            "test          $64,   %%eax   \n\t"
                            "jz                      2f   \n\t"
                            "movl          $63,   %%eax   \n\t"
                            "2:                           \n\t"
                            "shll           $5,   %%eax   \n\t"
                            "orl         %%eax,   %%edx   \n\t"
                            //---------------------------------
                            "movzwl       (%0),   %%eax   \n\t"
                            "andl          $31,   %%eax   \n\t"
                            "movzbl      2(%1),   %%ecx   \n\t"
                            "imull          %4,   %%ecx   \n\t"
                            "shrl          $11,   %%ecx   \n\t"
                            "addl        %%ecx,   %%eax   \n\t"
                            "test          $32,   %%eax   \n\t"
                            "jz                      3f   \n\t"
                            "movl          $31,   %%eax   \n\t"
                            "3:                           \n\t"
                            "orl         %%eax,   %%edx   \n\t"
                            //---------------------------------
                            "movw         %%dx,    (%0)"
                            :
                            : "D"(dst), "S"(src), "g"(cr), "g"(cg), "g"(cb)
                            : "eax", "edx", "ecx"
                        );

                        // C++ version:
                        /*
                        u8 r = std::min(   (*dst >> 11)         + (((src->red   >> 3) * c.red  ) >> 5), 31 );
                        u8 g = std::min( ( (*dst >>  5) & 0x3F) + (((src->green >> 2) * c.green) >> 6), 63 );
                        u8 b = std::min( (  *dst        & 0x1F) + (((src->blue  >> 3) * c.blue ) >> 5), 31 );
                        *dst = (r << 11) | (g << 5) | b;
                        */
                    }
                };

                //---------------------------------------------------------
                struct rgb565_sub_col
                {
                    u32 cr;
                    u32 cg;
                    u32 cb;

                    explicit rgb565_sub_col(graphics::RGBA color)
                    {
                        cr = color.red   + 1;
                        cg = color.green + 1;
                        cb = color.blue  + 1;
                    }

                    __attribute__((__always_inline__))
                    void operator()(u16* dst, const graphics::RGBA* src)
                    {
                        asm (
                            "movzwl       (%0),   %%eax   \n\t"
                            "shrl          $11,   %%eax   \n\t"
                            "movzbl       (%1),   %%ecx   \n\t"
                            "imull          %2,   %%ecx   \n\t"
                            "shrl          $11,   %%ecx   \n\t"
                            "subl        %%ecx,   %%eax   \n\t"
                            "jns                     1f   \n\t"
                            "xorl        %%eax,   %%eax   \n\t"
                            "1:                           \n\t"
                            "shll          $11,   %%eax   \n\t"
                            "movl        %%eax,   %%edx   \n\t"
                            //---------------------------------
                            "movzwl       (%0),   %%eax   \n\t"
                            "shrl           $5,   %%eax   \n\t"
                            "andl          $63,   %%eax   \n\t"
                            "movzbl      1(%1),   %%ecx   \n\t"
                            "imull          %3,   %%ecx   \n\t"
                            "shrl          $10,   %%ecx   \n\t"
                            "subl        %%ecx,   %%eax   \n\t"
                            "jns                     2f   \n\t"
                            "xorl        %%eax,   %%eax   \n\t"
                            "2:                           \n\t"
                            "shll           $5,   %%eax   \n\t"
                            "orl         %%eax,   %%edx   \n\t"
                            //---------------------------------
                            "movzwl       (%0),   %%eax   \n\t"
                            "andl          $31,   %%eax   \n\t"
                            "movzbl      2(%1),   %%ecx   \n\t"
                            "imull          %4,   %%ecx   \n\t"
                            "shrl          $11,   %%ecx   \n\t"
                            "subl        %%ecx,   %%eax   \n\t"
                            "jns                     3f   \n\t"
                            "xorl        %%eax,   %%eax   \n\t"
                            "3:                           \n\t"
                            "orl         %%eax,   %%edx   \n\t"
                            //---------------------------------
                            "movw         %%dx,    (%0)"
                            :
                            : "D"(dst), "S"(src), "g"(cr), "g"(cg), "g"(cb)
                            : "eax", "edx", "ecx"
                        );

                        // C++ version:
                        /*
                        u8 r = std::max(   (*dst >> 11)         - (((src->red   >> 3) * c.red  ) >> 5), 0 );
                        u8 g = std::max( ( (*dst >>  5) & 0x3F) - (((src->green >> 2) * c.green) >> 6), 0 );
                        u8 b = std::max( (  *dst        & 0x1F) - (((src->blue  >> 3) * c.blue ) >> 5), 0 );
                        *dst = (r << 11) | (g << 5) | b;
                        */
                    }
                };

                //---------------------------------------------------------
                struct rgb565_mul_col
                {
                    u32 cr;
                    u32 cg;
                    u32 cb;

                    explicit rgb565_mul_col(graphics::RGBA color)
                    {
                        // "+3" is an optimization, so we can
                        // do two subsequent multiplications
                        cr = color.red   + 3;
                        cg = color.green + 3;
                        cb = color.blue  + 3;
                    }

                    __attribute__((__always_inline__))
                    void operator()(u16* dst, const graphics::RGBA* src)
                    {
                        asm (
                            "movzwl       (%0),   %%eax   \n\t"
                            "shrl          $11,   %%eax   \n\t"
                            "movzbl       (%1),   %%ecx   \n\t"
                            "imull          %2,   %%ecx   \n\t"
                            "imull       %%ecx,   %%eax   \n\t"
                            "shrl          $16,   %%eax   \n\t"
                            "shll          $11,   %%eax   \n\t"
                            "movl        %%eax,   %%edx   \n\t"
                            //---------------------------------
                            "movzwl       (%0),   %%eax   \n\t"
                            "shrl           $5,   %%eax   \n\t"
                            "andl          $63,   %%eax   \n\t"
                            "movzbl      1(%1),   %%ecx   \n\t"
                            "imull          %3,   %%ecx   \n\t"
                            "imull       %%ecx,   %%eax   \n\t"
                            "shrl          $16,   %%eax   \n\t"
                            "shll           $5,   %%eax   \n\t"
                            "orl         %%eax,   %%edx   \n\t"
                            //---------------------------------
                            "movzwl       (%0),   %%eax   \n\t"
                            "andl          $31,   %%eax   \n\t"
                            "movzbl      2(%1),   %%ecx   \n\t"
                            "imull          %4,   %%ecx   \n\t"
                            "imull       %%ecx,   %%eax   \n\t"
                            "shrl          $16,   %%eax   \n\t"
                            "orl         %%edx,   %%eax   \n\t"
                            //---------------------------------
                            "movw         %%ax,    (%0)"
                            :
                            : "D"(dst), "S"(src), "g"(cr), "g"(cg), "g"(cb)
                            : "eax", "edx", "ecx"
                        );

                        // C++ version:
                        /*
                        u8 r =   (*dst >> 11)         * ((((src->red   >> 3) * c.red  ) >> 5) + 1) >> 5;
                        u8 g = ( (*dst >>  5) & 0x3F) * ((((src->green >> 2) * c.green) >> 6) + 1) >> 6;
                        u8 b = (  *dst        & 0x1F) * ((((src->blue  >> 3) * c.blue ) >> 5) + 1) >> 5;
                        *dst = (r << 11) | (g << 5) | b;
                        */
                    }
                };

            }

            //---------------------------------------------------------
            core::Recti Screen::_clipRect = core::Recti(0, 0, 320, 240);

            //---------------------------------------------------------
            int
            Screen::GetWidth()
            {
                return RPG::screen->canvas->width();
            }

            //---------------------------------------------------------
            int
            Screen::GetHeight()
            {
                return RPG::screen->canvas->height();
            }

            //---------------------------------------------------------
            int
            Screen::GetPitch()
            {
                return RPG::screen->canvas->lineSize / 2;
            }

            //---------------------------------------------------------
            u16*
            Screen::GetPixels()
            {
                return RPG::screen->canvas->getScanline(0);
            }

            //---------------------------------------------------------
            const core::Recti&
            Screen::GetClipRect()
            {
                return _clipRect;
            }

            //---------------------------------------------------------
            void
            Screen::SetClipRect(const core::Recti& clipRect)
            {
                core::Recti screenBounds(0, 0, 320, 240);
                if (clipRect.isInside(screenBounds)) {
                    _clipRect = clipRect;
                }
            }

            //---------------------------------------------------------
            graphics::RGBA
            Screen::ApplyBrightness(graphics::RGBA color)
            {
                int brightness = RPG::screen->canvas->brightness;
                int r = std::min(255, (color.red   * brightness) / 100);
                int g = std::min(255, (color.green * brightness) / 100);
                int b = std::min(255, (color.blue  * brightness) / 100);
                return graphics::RGBA(r, g, b, color.alpha);
            }

            //-----------------------------------------------------------------
            u16
            Screen::GetPixel(int x, int y)
            {
                return *(GetPixels() + GetPitch() * y + x);
            }

            //-----------------------------------------------------------------
            void
            Screen::SetPixel(int x, int y, u16 color)
            {
                *(GetPixels() + GetPitch() * y + x) = color;
            }

            //-------------------- ---------------------------------------------
            graphics::Image::Ptr
            Screen::CopyRect(const core::Recti& rect, graphics::Image* destination)
            {
                core::Recti screen_bounds(GetWidth(), GetHeight());
                if (!rect.isValid() || !rect.isInside(screen_bounds)) {
                    return 0;
                }

                int x = rect.getX();
                int y = rect.getY();
                int w = rect.getWidth();
                int h = rect.getHeight();

                graphics::Image::Ptr result;

                if (destination) {
                    result = destination;
                    result->resize(w, h);
                } else {
                    result = graphics::Image::New(w, h);
                }

                graphics::RGBA* dst = result->getPixels();

                int  src_pitch = GetPitch();
                u16* src       = GetPixels() + src_pitch * y + x;

                for (int iy = 0; iy < h; iy++) {
                    for (int ix = 0; ix < w; ix++) {
                        *dst = graphics::RGB565ToRGBA(*src);
                        src++;
                        dst++;
                    }
                    src += src_pitch - w;
                }

                return result;
            }

            //-----------------------------------------------------------------
            void
            Screen::Clear_generic(graphics::RGBA color)
            {
                u16 c = graphics::RGBAToRGB565(color);

                u16* dp = GetPixels();
                int  di = GetPitch() - GetWidth();

                for (int y = GetHeight(); y > 0; y--) {
                    for (int x = GetWidth(); x > 0; x--) {
                        *dp = c;
                        dp++;
                    }
                    dp += di;
                }
            }

            //-----------------------------------------------------------------
            void
            Screen::Clear_sse2(graphics::RGBA color)
            {
                u16 c = graphics::RGBAToRGB565(color);

                u16* dp = GetPixels();
                int  di = GetPitch() - GetWidth();

                int burst1 = GetWidth() / 8;
                int burst2 = GetWidth() % 8;

                __m128i mc = _mm_set_epi16(c, c, c, c, c, c, c, c);

                for (int y = GetHeight(); y > 0; y--) {
                    for (int x = burst1; x > 0; x--) {
                        _mm_storeu_si128((__m128i*)dp, mc);
                        dp += 8;
                    }
                    for (int x = burst2; x > 0; x--) {
                        *dp = c;
                        dp++;
                    }
                    dp += di;
                }
            }

            //-----------------------------------------------------------------
            void
            Screen::Clear(graphics::RGBA color)
            {
                if (CpuSupportsSse2()) {
                    Clear_sse2(color);
                } else {
                    Clear_generic(color);
                }
            }

            //-----------------------------------------------------------------
            void
            Screen::Grey()
            {
                int  dst_pitch  = GetPitch();
                u16* dst_pixels = GetPixels();

                int w = GetWidth();
                int h = GetHeight();

                for (int y = 0; y < h; y++) {
                    u16* dst = dst_pixels + dst_pitch * y;
                    for (int x = 0; x < w; x++) {
                        unsigned int c = *dst;
                        unsigned int g = ((c >> 11) + ((c >> 6) & 0x001F) + (c & 0x001F)) / 3;
                        *dst = (g << 11) | ((g << 6) | 0x01)  | g;
                        dst++;
                    }
                }
            }

            //-----------------------------------------------------------------
            void
            Screen::DrawPoint(const core::Vec2i& pos, graphics::RGBA color, int blendMode)
            {
                color = ApplyBrightness(color);

                switch (blendMode) {
                case graphics::BlendMode::Set:      graphics::primitives::Point(GetPixels(), GetPitch(), _clipRect, pos, color, rgb565_set()); break;
                case graphics::BlendMode::Mix:      graphics::primitives::Point(GetPixels(), GetPitch(), _clipRect, pos, color, rgb565_mix()); break;
                case graphics::BlendMode::Add:      graphics::primitives::Point(GetPixels(), GetPitch(), _clipRect, pos, color, rgb565_add()); break;
                case graphics::BlendMode::Subtract: graphics::primitives::Point(GetPixels(), GetPitch(), _clipRect, pos, color, rgb565_sub()); break;
                case graphics::BlendMode::Multiply: graphics::primitives::Point(GetPixels(), GetPitch(), _clipRect, pos, color, rgb565_mul()); break;
                }
            }

            //-----------------------------------------------------------------
            void
            Screen::DrawLine(const core::Vec2i& p1, const core::Vec2i& p2, graphics::RGBA c1, graphics::RGBA c2, int blendMode)
            {
                c1 = ApplyBrightness(c1);
                c2 = ApplyBrightness(c2);

                switch (blendMode) {
                case graphics::BlendMode::Set:      graphics::primitives::Line(GetPixels(), GetPitch(), _clipRect, p1, p2, c1, c2, rgb565_set()); break;
                case graphics::BlendMode::Mix:      graphics::primitives::Line(GetPixels(), GetPitch(), _clipRect, p1, p2, c1, c2, rgb565_mix()); break;
                case graphics::BlendMode::Add:      graphics::primitives::Line(GetPixels(), GetPitch(), _clipRect, p1, p2, c1, c2, rgb565_add()); break;
                case graphics::BlendMode::Subtract: graphics::primitives::Line(GetPixels(), GetPitch(), _clipRect, p1, p2, c1, c2, rgb565_sub()); break;
                case graphics::BlendMode::Multiply: graphics::primitives::Line(GetPixels(), GetPitch(), _clipRect, p1, p2, c1, c2, rgb565_mul()); break;
                }
            }

            //-----------------------------------------------------------------
            void
            Screen::DrawRectangle(bool fill, const core::Recti& rect, graphics::RGBA c1, graphics::RGBA c2, graphics::RGBA c3, graphics::RGBA c4, int blendMode)
            {
                c1 = ApplyBrightness(c1);
                c2 = ApplyBrightness(c2);
                c3 = ApplyBrightness(c3);
                c4 = ApplyBrightness(c4);

                switch (blendMode) {
                case graphics::BlendMode::Set:      graphics::primitives::Rectangle(GetPixels(), GetPitch(), _clipRect, fill, rect, c1, c2, c3, c4, rgb565_set()); break;
                case graphics::BlendMode::Mix:      graphics::primitives::Rectangle(GetPixels(), GetPitch(), _clipRect, fill, rect, c1, c2, c3, c4, rgb565_mix()); break;
                case graphics::BlendMode::Add:      graphics::primitives::Rectangle(GetPixels(), GetPitch(), _clipRect, fill, rect, c1, c2, c3, c4, rgb565_add()); break;
                case graphics::BlendMode::Subtract: graphics::primitives::Rectangle(GetPixels(), GetPitch(), _clipRect, fill, rect, c1, c2, c3, c4, rgb565_sub()); break;
                case graphics::BlendMode::Multiply: graphics::primitives::Rectangle(GetPixels(), GetPitch(), _clipRect, fill, rect, c1, c2, c3, c4, rgb565_mul()); break;
                }
            }

            //-----------------------------------------------------------------
            void
            Screen::DrawCircle(bool fill, const core::Vec2i& center, int radius, graphics::RGBA c1, graphics::RGBA c2, int blendMode)
            {
                c1 = ApplyBrightness(c1);
                c2 = ApplyBrightness(c2);

                switch (blendMode) {
                case graphics::BlendMode::Set:      graphics::primitives::Circle(GetPixels(), GetPitch(), _clipRect, fill, center, radius, c1, c2, rgb565_set()); break;
                case graphics::BlendMode::Mix:      graphics::primitives::Circle(GetPixels(), GetPitch(), _clipRect, fill, center, radius, c1, c2, rgb565_mix()); break;
                case graphics::BlendMode::Add:      graphics::primitives::Circle(GetPixels(), GetPitch(), _clipRect, fill, center, radius, c1, c2, rgb565_add()); break;
                case graphics::BlendMode::Subtract: graphics::primitives::Circle(GetPixels(), GetPitch(), _clipRect, fill, center, radius, c1, c2, rgb565_sub()); break;
                case graphics::BlendMode::Multiply: graphics::primitives::Circle(GetPixels(), GetPitch(), _clipRect, fill, center, radius, c1, c2, rgb565_mul()); break;
                }
            }

            //-----------------------------------------------------------------
            void
            Screen::DrawTriangle(bool fill, const core::Vec2i& p1, const core::Vec2i& p2, const core::Vec2i& p3, graphics::RGBA c1, graphics::RGBA c2, graphics::RGBA c3, int blendMode)
            {
                c1 = ApplyBrightness(c1);
                c2 = ApplyBrightness(c2);
                c3 = ApplyBrightness(c3);

                // TODO
            }

            //-----------------------------------------------------------------
            void
            Screen::Draw(const graphics::Image* image, const core::Recti& image_rect, const core::Vec2i& pos, float angle, float scale, graphics::RGBA color, int blendMode)
            {
                color = ApplyBrightness(color);

                if (angle == 0.0)
                {
                    core::Recti rect = core::Recti(pos, image_rect.getDimensions()).scale(scale);

                    if (color == graphics::RGBA(255, 255, 255, 255))
                    {
                        switch (blendMode) {
                        case graphics::BlendMode::Set:      graphics::primitives::TexturedRectangle(GetPixels(), GetPitch(), _clipRect, rect, image->getPixels(), image->getWidth(), image_rect, rgb565_set()); break;
                        case graphics::BlendMode::Mix:      graphics::primitives::TexturedRectangle(GetPixels(), GetPitch(), _clipRect, rect, image->getPixels(), image->getWidth(), image_rect, rgb565_mix()); break;
                        case graphics::BlendMode::Add:      graphics::primitives::TexturedRectangle(GetPixels(), GetPitch(), _clipRect, rect, image->getPixels(), image->getWidth(), image_rect, rgb565_add()); break;
                        case graphics::BlendMode::Subtract: graphics::primitives::TexturedRectangle(GetPixels(), GetPitch(), _clipRect, rect, image->getPixels(), image->getWidth(), image_rect, rgb565_sub()); break;
                        case graphics::BlendMode::Multiply: graphics::primitives::TexturedRectangle(GetPixels(), GetPitch(), _clipRect, rect, image->getPixels(), image->getWidth(), image_rect, rgb565_mul()); break;
                        }
                    }
                    else
                    {
                        switch (blendMode) {
                        case graphics::BlendMode::Set:      graphics::primitives::TexturedRectangle(GetPixels(), GetPitch(), _clipRect, rect, image->getPixels(), image->getWidth(), image_rect, rgb565_set_col(color)); break;
                        case graphics::BlendMode::Mix:      graphics::primitives::TexturedRectangle(GetPixels(), GetPitch(), _clipRect, rect, image->getPixels(), image->getWidth(), image_rect, rgb565_mix_col(color)); break;
                        case graphics::BlendMode::Add:      graphics::primitives::TexturedRectangle(GetPixels(), GetPitch(), _clipRect, rect, image->getPixels(), image->getWidth(), image_rect, rgb565_add_col(color)); break;
                        case graphics::BlendMode::Subtract: graphics::primitives::TexturedRectangle(GetPixels(), GetPitch(), _clipRect, rect, image->getPixels(), image->getWidth(), image_rect, rgb565_sub_col(color)); break;
                        case graphics::BlendMode::Multiply: graphics::primitives::TexturedRectangle(GetPixels(), GetPitch(), _clipRect, rect, image->getPixels(), image->getWidth(), image_rect, rgb565_mul_col(color)); break;
                        }
                    }
                }
                else
                {
                    core::Recti rect = core::Recti(pos, image_rect.getDimensions()).scale(scale);
                    core::Vec2i center = rect.getCenter();

                    core::Vec2i ul = rect.getUpperLeft().rotateBy(angle, center);
                    core::Vec2i ur = rect.getUpperRight().rotateBy(angle, center);
                    core::Vec2i lr = rect.getLowerRight().rotateBy(angle, center);
                    core::Vec2i ll = rect.getLowerLeft().rotateBy(angle, center);

                    Drawq(image, image_rect, ul, ur, lr, ll, color, blendMode);
                }
            }

            //-----------------------------------------------------------------
            void
            Screen::Drawq(const graphics::Image* image, const core::Recti& image_rect, const core::Vec2i& ul, const core::Vec2i& ur, const core::Vec2i& lr, const core::Vec2i& ll, graphics::RGBA color, int blendMode)
            {
                color = ApplyBrightness(color);
                core::Vec2i pos[4] = { ul, ur, lr, ll };

                // graphics::primitives::TexturedQuad doesn't support yet arbitrary
                // source rects directly, so we need this fix here for the time being
                const graphics::RGBA* image_pixels = image->getPixels() + image_rect.getY() * image->getWidth() + image_rect.getX();

                if (color == graphics::RGBA(255, 255, 255, 255))
                {
                    switch (blendMode) {
                    case graphics::BlendMode::Set:      graphics::primitives::TexturedQuad(GetPixels(), GetPitch(), _clipRect, pos, image_pixels, image->getWidth(), image_rect, rgb565_set()); break;
                    case graphics::BlendMode::Mix:      graphics::primitives::TexturedQuad(GetPixels(), GetPitch(), _clipRect, pos, image_pixels, image->getWidth(), image_rect, rgb565_mix()); break;
                    case graphics::BlendMode::Add:      graphics::primitives::TexturedQuad(GetPixels(), GetPitch(), _clipRect, pos, image_pixels, image->getWidth(), image_rect, rgb565_add()); break;
                    case graphics::BlendMode::Subtract: graphics::primitives::TexturedQuad(GetPixels(), GetPitch(), _clipRect, pos, image_pixels, image->getWidth(), image_rect, rgb565_sub()); break;
                    case graphics::BlendMode::Multiply: graphics::primitives::TexturedQuad(GetPixels(), GetPitch(), _clipRect, pos, image_pixels, image->getWidth(), image_rect, rgb565_mul()); break;
                    }
                }
                else
                {
                    switch (blendMode) {
                    case graphics::BlendMode::Set:      graphics::primitives::TexturedQuad(GetPixels(), GetPitch(), _clipRect, pos, image_pixels, image->getWidth(), image_rect, rgb565_set_col(color)); break;
                    case graphics::BlendMode::Mix:      graphics::primitives::TexturedQuad(GetPixels(), GetPitch(), _clipRect, pos, image_pixels, image->getWidth(), image_rect, rgb565_mix_col(color)); break;
                    case graphics::BlendMode::Add:      graphics::primitives::TexturedQuad(GetPixels(), GetPitch(), _clipRect, pos, image_pixels, image->getWidth(), image_rect, rgb565_add_col(color)); break;
                    case graphics::BlendMode::Subtract: graphics::primitives::TexturedQuad(GetPixels(), GetPitch(), _clipRect, pos, image_pixels, image->getWidth(), image_rect, rgb565_sub_col(color)); break;
                    case graphics::BlendMode::Multiply: graphics::primitives::TexturedQuad(GetPixels(), GetPitch(), _clipRect, pos, image_pixels, image->getWidth(), image_rect, rgb565_mul_col(color)); break;
                    }
                }
            }

            //-----------------------------------------------------------------
            void
            Screen::DrawText(const graphics::Font* font, core::Vec2i pos, const char* text, int len, float scale, graphics::RGBA color)
            {
                assert(font);
                assert(text);

                color = ApplyBrightness(color);

                if (len < 0) {
                    len = std::strlen(text);
                }

                int cur_x = pos.x;
                int cur_y = pos.y;

                for (int i = 0; i < len; i++)
                {
                    switch (text[i])
                    {
                        case ' ':
                        {
                            const graphics::Image* space_char_image = font->getCharImage(' ');
                            int space_w = (int)(space_char_image ? space_char_image->getWidth() * scale : 0);
                            cur_x += space_w;
                            break;
                        }
                        case '\t':
                        {
                            const graphics::Image* space_char_image = font->getCharImage(' ');
                            int tab_w = (int)(space_char_image ? space_char_image->getWidth() * font->getTabWidth() * scale : 0);
                            if (tab_w > 0) {
                                tab_w = tab_w - ((cur_x - pos.x) % tab_w);
                            }
                            cur_x += tab_w;
                            break;
                        }
                        case '\n':
                        {
                            cur_x = pos.x;
                            cur_y += (int)(font->getMaxCharHeight() * scale);
                            break;
                        }
                        default:
                        {
                            const graphics::Image* char_image = font->getCharImage(text[i]);
                            if (char_image)
                            {
                                if (color == graphics::RGBA(255, 255, 255, 255))
                                {
                                    graphics::primitives::TexturedRectangle(
                                        GetPixels(),
                                        GetPitch(),
                                        _clipRect,
                                        core::Recti(cur_x, cur_y, char_image->getWidth(), char_image->getHeight()).scale(scale),
                                        char_image->getPixels(),
                                        char_image->getWidth(),
                                        core::Recti(0, 0, char_image->getWidth(), char_image->getHeight()),
                                        rgb565_mix()
                                    );
                                }
                                else
                                {
                                    graphics::primitives::TexturedRectangle(
                                        GetPixels(),
                                        GetPitch(),
                                        _clipRect,
                                        core::Recti(cur_x, cur_y, char_image->getWidth(), char_image->getHeight()).scale(scale),
                                        char_image->getPixels(),
                                        char_image->getWidth(),
                                        core::Recti(0, 0, char_image->getWidth(), char_image->getHeight()),
                                        rgb565_mix_col(color)
                                    );
                                }
                                cur_x += (int)(char_image->getWidth() * scale);
                            }
                            break;
                        }
                    }
                }
            }

            //-----------------------------------------------------------------
            void
            Screen::DrawWindow(const graphics::WindowSkin* windowSkin, core::Recti windowRect)
            {
                assert(windowSkin);

                if (!windowRect.isValid()) {
                    return;
                }

                graphics::RGBA color = ApplyBrightness(graphics::RGBA(255, 255, 255, 255));

                // for brevity
                int x1 = windowRect.ul.x;
                int y1 = windowRect.ul.y;
                int x2 = windowRect.lr.x;
                int y2 = windowRect.lr.y;

                const graphics::Image* tlBorder = windowSkin->getBorderImage(graphics::WindowSkin::TopLeftBorder);
                const graphics::Image* trBorder = windowSkin->getBorderImage(graphics::WindowSkin::TopRightBorder);
                const graphics::Image* brBorder = windowSkin->getBorderImage(graphics::WindowSkin::BottomRightBorder);
                const graphics::Image* blBorder = windowSkin->getBorderImage(graphics::WindowSkin::BottomLeftBorder);

                // draw background
                if (!windowRect.isEmpty()) {
                    graphics::RGBA tlColor = ApplyBrightness(windowSkin->getBgColor(graphics::WindowSkin::TopLeftBgColor));
                    graphics::RGBA trColor = ApplyBrightness(windowSkin->getBgColor(graphics::WindowSkin::TopRightBgColor));
                    graphics::RGBA brColor = ApplyBrightness(windowSkin->getBgColor(graphics::WindowSkin::BottomRightBgColor));
                    graphics::RGBA blColor = ApplyBrightness(windowSkin->getBgColor(graphics::WindowSkin::BottomLeftBgColor));

                    graphics::primitives::Rectangle(
                        GetPixels(),
                        GetPitch(),
                        _clipRect,
                        true,
                        windowRect,
                        tlColor,
                        trColor,
                        brColor,
                        blColor,
                        rgb565_mix()
                    );
                }

                // draw top left edge
                graphics::primitives::TexturedRectangle(
                    GetPixels(),
                    GetPitch(),
                    _clipRect,
                    core::Vec2i(x1 - tlBorder->getWidth(), y1 - tlBorder->getHeight()),
                    tlBorder->getPixels(),
                    tlBorder->getWidth(),
                    core::Recti(tlBorder->getDimensions()),
                    rgb565_mix_col(color)
                );

                // draw top right edge
                graphics::primitives::TexturedRectangle(
                    GetPixels(),
                    GetPitch(),
                    _clipRect,
                    core::Vec2i(x2 + 1, y1 - trBorder->getHeight()),
                    trBorder->getPixels(),
                    trBorder->getWidth(),
                    core::Recti(trBorder->getDimensions()),
                    rgb565_mix_col(color)
                );

                // draw bottom right edge
                graphics::primitives::TexturedRectangle(
                    GetPixels(),
                    GetPitch(),
                    _clipRect,
                    core::Vec2i(x2 + 1, y2 + 1 ),
                    brBorder->getPixels(),
                    brBorder->getWidth(),
                    core::Recti(brBorder->getDimensions()),
                    rgb565_mix_col(color)
                );

                // draw bottom left edge
                graphics::primitives::TexturedRectangle(
                    GetPixels(),
                    GetPitch(),
                    _clipRect,
                    core::Vec2i(x1 - blBorder->getWidth(), y2 + 1 ),
                    blBorder->getPixels(),
                    blBorder->getWidth(),
                    core::Recti(blBorder->getDimensions()),
                    rgb565_mix_col(color)
                );

                // draw top and bottom borders
                if (windowRect.getWidth() > 0)
                {
                    const graphics::Image* tBorder  = windowSkin->getBorderImage(graphics::WindowSkin::TopBorder);
                    const graphics::Image* bBorder  = windowSkin->getBorderImage(graphics::WindowSkin::BottomBorder);

                    int i;

                    // draw top border
                    i = x1;
                    while ((x2 - i) + 1 >= tBorder->getWidth()) {
                        graphics::primitives::TexturedRectangle(
                            GetPixels(),
                            GetPitch(),
                            _clipRect,
                            core::Vec2i(i, y1 - tBorder->getHeight()),
                            tBorder->getPixels(),
                            tBorder->getWidth(),
                            core::Recti(tBorder->getDimensions()),
                            rgb565_mix_col(color)
                        );

                        i += tBorder->getWidth();
                    }

                    if ((x2 - i) + 1 > 0) {
                        graphics::primitives::TexturedRectangle(
                            GetPixels(),
                            GetPitch(),
                            _clipRect,
                            core::Vec2i(i, y1 - tBorder->getHeight()),
                            tBorder->getPixels(),
                            tBorder->getWidth(),
                            core::Recti(0, 0, (x2 - i) + 1, tBorder->getHeight()),
                            rgb565_mix_col(color)
                        );
                    }

                    // draw bottom border
                    i = x1;
                    while ((x2 - i) + 1 >= bBorder->getWidth()) {
                        graphics::primitives::TexturedRectangle(
                            GetPixels(),
                            GetPitch(),
                            _clipRect,
                            core::Vec2i(i, y2 + 1),
                            bBorder->getPixels(),
                            bBorder->getWidth(),
                            core::Recti(bBorder->getDimensions()),
                            rgb565_mix_col(color)
                        );

                        i += bBorder->getWidth();
                    }

                    if ((x2 - i) + 1 > 0) {
                        graphics::primitives::TexturedRectangle(
                            GetPixels(),
                            GetPitch(),
                            _clipRect,
                            core::Vec2i(i, y2 + 1),
                            bBorder->getPixels(),
                            bBorder->getWidth(),
                            core::Recti(0, 0, (x2 - i) + 1, bBorder->getHeight()),
                            rgb565_mix_col(color)
                        );
                    }
                }

                // draw left and right borders
                if (windowRect.getHeight() > 0)
                {
                    const graphics::Image* lBorder  = windowSkin->getBorderImage(graphics::WindowSkin::LeftBorder);
                    const graphics::Image* rBorder  = windowSkin->getBorderImage(graphics::WindowSkin::RightBorder);

                    int i;

                    // draw left border
                    i = y1;
                    while ((y2 - i) + 1 >= lBorder->getHeight()) {
                        graphics::primitives::TexturedRectangle(
                            GetPixels(),
                            GetPitch(),
                            _clipRect,
                            core::Vec2i(x1 - lBorder->getWidth(), i),
                            lBorder->getPixels(),
                            lBorder->getWidth(),
                            core::Recti(lBorder->getDimensions()),
                            rgb565_mix_col(color)
                        );

                        i += lBorder->getHeight();
                    }

                    if ((y2 - i) + 1 > 0) {
                        graphics::primitives::TexturedRectangle(
                            GetPixels(),
                            GetPitch(),
                            _clipRect,
                            core::Vec2i(x1 - lBorder->getWidth(), i),
                            lBorder->getPixels(),
                            lBorder->getWidth(),
                            core::Recti(0, 0, lBorder->getWidth(), (y2 - i) + 1),
                            rgb565_mix_col(color)
                        );
                    }

                    // draw right border
                    i = y1;
                    while ((y2 - i) + 1 >= rBorder->getHeight()) {
                        graphics::primitives::TexturedRectangle(
                            GetPixels(),
                            GetPitch(),
                            _clipRect,
                            core::Vec2i(x2 + 1, i),
                            rBorder->getPixels(),
                            rBorder->getWidth(),
                            core::Recti(rBorder->getDimensions()),
                            rgb565_mix_col(color)
                        );

                        i += rBorder->getHeight();
                    }

                    if ((y2 - i) + 1 > 0) {
                        graphics::primitives::TexturedRectangle(
                            GetPixels(),
                            GetPitch(),
                            _clipRect,
                            core::Vec2i(x2 + 1, i),
                            rBorder->getPixels(),
                            rBorder->getWidth(),
                            core::Recti(0, 0, rBorder->getWidth(), (y2 - i) + 1),
                            rgb565_mix_col(color)
                        );
                    }
                }
            }

        } // namespace game_module
    } // namespace script
} // namespace rpgss
