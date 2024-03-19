// Bitmap font routine originally by driedfruit,
// https://github.com/driedfruit/SDL_inprint Released into public domain.
// Modified to support multiple fonts & adding a background to text.

#include <SDL.h>
#include <SDL_gfxPrimitives.h>
#include <SDL_imageFilter.h>

#define CHARACTERS_PER_ROW 16   /* I like 16 x 8 fontsets. */
#define CHARACTERS_PER_COLUMN 8 /* 128 x 1 is another popular format. */

static Uint16 selected_font_w, selected_font_h;
static unsigned char *selected_font_bits =0;
static SDL_Surface *font = 0;
static SDL_Color pal[1];

void incolor1(SDL_Color *color) {
    pal[0].r = color->r;
    pal[0].g = color->g;
    pal[0].b = color->b;
}


void drawBg(
        SDL_Surface *dst,
        SDL_Rect d_rect,
        Uint8 r,
        Uint8 g,
        Uint8 b,
        Uint8 a
) {
    boxRGBA(
            dst,
            d_rect.x,
            d_rect.y,
            d_rect.x + d_rect.w-1,
            d_rect.y + d_rect.h-1,
            r,
            g,
            b,
            a
    );

}

void drawChar(SDL_Surface *dst, SDL_Rect s_rect, SDL_Rect d_rect) {
    uint32_t *pix = (uint32_t *) font->pixels;
    for (uint16_t x = s_rect.x; x < s_rect.x + s_rect.w; x++) {
        for (uint16_t y = s_rect.y; y < s_rect.y + s_rect.h; y++) {
            uint32_t pixel = (pix[x + (y * font->w)] >> 24 & 0xff) > 0;
            uint32_t color = ((255 << 24) + (pal[0].b << 16) + (pal[0].g << 8) + (pal[0].r)) * pixel;
            pix[x + (y * font->w)] = color;
        }
    }
    SDL_BlitSurface(font, &s_rect, dst, &d_rect);
}

void prepare_inline_font(unsigned char *bits, int font_width, int font_height) {

    selected_font_w = font_width;
    selected_font_h = font_height;
    selected_font_bits = bits;

    Uint32 rmask, gmask, bmask, amask;

    /* SDL interprets each pixel as a 32-bit number, so our masks must depend
       on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    font = SDL_CreateRGBSurface(0, font_width, font_height, 32, rmask, gmask, bmask, amask);

    SDL_SetAlpha(font, SDL_SRCALPHA, 0);

    SDL_Color color = {
            255, 255, 255, 255
    };
    incolor1(&color);

    int size = selected_font_w * selected_font_h / 8;
    for (int i = 0; i < size; ++i) {
        for (Uint8 bit = 0; bit < 8; bit++) {
            Uint8 hasPixel = (((0xff ^ selected_font_bits[i]) >> bit) & 0x1) ? 1 : 0;
            int x = (i * 8 + bit) % selected_font_w;
            int y = (i * 8 + bit) / selected_font_w;
            pixelRGBA(font, x, y, 255 * hasPixel, 255 * hasPixel, 255 * hasPixel, 255 * hasPixel);
        }
    }
}

void kill_inline_font(void) {
//    SDL_DestroyTexture(inline_font);
//    inline_font = NULL;
}


void infont() {
    Uint32 format;
    int access;
    int w, h;

//    if (font == NULL) {
//        // prepare_inline_font();
//        return;
//    }
//    SDL_QueryTexture(font, &format, &access, &w, &h);

//    selected_font = font;
    selected_font_w = w;
    selected_font_h = h;
}


void incolor(Uint32 fore, Uint32 unused) {
    /* Color must be in 0x00RRGGBB format ! */
    SDL_Color color = {
            (Uint8) ((fore & 0x00FF0000) >> 16),
            (Uint8) ((fore & 0x0000FF00) >> 8),
            (Uint8) ((fore & 0x000000FF)),
            unused
    };
    incolor1(&color);
}

void inprint(
        SDL_Surface *dst,
        const char *str,
        Sint16 x,
        Sint16 y,
        Uint32 fgcolor,
        Uint32 bgcolor
) {

    SDL_Rect s_rect;
    SDL_Rect d_rect;
    SDL_Rect bg_rect;

    static uint32_t previous_fgcolor;

    d_rect.x = x;
    d_rect.y = y;
    s_rect.w = selected_font_w / CHARACTERS_PER_ROW;
    s_rect.h = selected_font_h / CHARACTERS_PER_COLUMN;
    d_rect.w = s_rect.w;
    d_rect.h = s_rect.h;

//    if (dst == NULL)
//        dst = selected_renderer;

    for (; *str; str++) {
        int id = (int) *str;
#if (CHARACTERS_PER_COLUMN != 1)
        int row = id / CHARACTERS_PER_ROW;
        int col = id % CHARACTERS_PER_ROW;
        s_rect.x = (col * s_rect.w);
        s_rect.y = (row * s_rect.h);
#else
        s_rect.x = id * s_rect.w;
        s_rect.y = 0;
#endif
        if (id == '\n') {
            d_rect.x = x;
            d_rect.y += s_rect.h;
            continue;
        }
        if (fgcolor != previous_fgcolor) {
            incolor(fgcolor, 0);
            previous_fgcolor = fgcolor;
        }

        if (bgcolor != -1) {
            bg_rect = d_rect;
            bg_rect.w = selected_font_w / CHARACTERS_PER_ROW - 1;
            // Silly hack to get big font background aligned correctly.
            if (bg_rect.h == 11) {
                bg_rect.y++;
            }

            drawBg(dst,
                   bg_rect,
                   (Uint8) ((bgcolor & 0x00FF0000) >> 16),
                   (Uint8) ((bgcolor & 0x0000FF00) >> 8),
                   (Uint8) (bgcolor & 0x000000FF),
                   0xFF
            );

        }
        drawChar(dst, s_rect, d_rect);
        d_rect.x += s_rect.w;
    }
}

