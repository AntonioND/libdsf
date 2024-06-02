// SPDX-License-Identifier: CC0-1.0
//
// SPDX-FileContributor: Antonio Niño Díaz, 2024

#include <stdio.h>

#include <nds.h>
#include <dsf.h>

#include "font_0_256.h"
#include "font2_0_16.h"

#include "font_fnt_bin.h"
#include "font2_fnt_bin.h"

__attribute__((noreturn)) void wait_forever(void)
{
    while (1)
    {
        swiWaitForVBlank();

        scanKeys();
        if (keysHeld() & KEY_START)
            exit(1);
    }
}

void draw_textured_quad(int x, int y, int sx, int sy)
{
    glBegin(GL_QUADS);

        GFX_TEX_COORD = TEXTURE_PACK(inttot16(0), inttot16(0));
        GFX_VERTEX16 = (y << 16) | (x & 0xFFFF); // Up-left
        GFX_VERTEX16 = 1 << 5;

        GFX_TEX_COORD = TEXTURE_PACK(inttot16(0), inttot16(sy));
        GFX_VERTEX_XY = ((y + sy - 1) << 16) | (x & 0xFFFF); // Down-left

        GFX_TEX_COORD = TEXTURE_PACK(inttot16(sx), inttot16(sy));
        GFX_VERTEX_XY = ((y + sy - 1) << 16) | ((x + sx - 1) & 0xFFFF); // Down-right

        GFX_TEX_COORD = TEXTURE_PACK(inttot16(sx), inttot16(0));
        GFX_VERTEX_XY = (y << 16) | ((x + sx - 1) & 0xFFFF); // Up-right

    glEnd();
}

int main(int argc, char **argv)
{
    // Setup sub screen for the text console
    consoleDemoInit();

    dsf_handle handle;
    dsf_error rc = DSF_LoadFontMemory(&handle, font_fnt_bin, font_fnt_bin_size);
    if (rc != DSF_NO_ERROR)
    {
        printf("DSF_LoadFontMemory() 1 failed: %d\n", rc);
        wait_forever();
    }

    dsf_handle handle2;
    rc = DSF_LoadFontMemory(&handle2, font2_fnt_bin, font2_fnt_bin_size);
    if (rc != DSF_NO_ERROR)
    {
        printf("DSF_LoadFontMemory() 2 failed: %d\n", rc);
        wait_forever();
    }

    videoSetMode(MODE_0_3D);

    glInit();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_ANTIALIAS);
    glEnable(GL_BLEND);

    // The background must be fully opaque and have a unique polygon ID
    // (different from the polygons that are going to be drawn) so that
    // antialias works.
    glClearColor(0, 0, 0, 31);
    glClearPolyID(63);

    glClearDepth(0x7FFF);

    glViewport(0, 0, 255, 191);

    // Setup some VRAM as memory for textures and texture palettes
    vramSetBankA(VRAM_A_TEXTURE);
    vramSetBankB(VRAM_B_TEXTURE);
    vramSetBankF(VRAM_F_TEX_PALETTE);

    // Load fonts as textures to VRAM

    int textureID;

    {
        glGenTextures(1, &textureID);
        glBindTexture(0, textureID);

        if (glTexImage2D(0, 0, GL_RGB256, 256, 256, 0,
                         TEXGEN_TEXCOORD | GL_TEXTURE_COLOR0_TRANSPARENT,
                         font_0_256Bitmap) == 0)
        {
            printf("Failed to load texture 1");
            wait_forever();
        }
        if (glColorTableEXT(0, 0, 256, 0, 0, font_0_256Pal) == 0)
        {
            printf("Failed to load palette 1");
            wait_forever();
        }
    }

    int textureID2;

    {
        glGenTextures(1, &textureID2);
        glBindTexture(0, textureID2);

        if (glTexImage2D(0, 0, GL_RGB16, 128, 128, 0,
                         TEXGEN_TEXCOORD | GL_TEXTURE_COLOR0_TRANSPARENT,
                         font2_0_16Bitmap) == 0)
        {
            printf("Failed to load texture 2");
            wait_forever();
        }
        if (glColorTableEXT(0, 0, 16, 0, 0, font2_0_16Pal) == 0)
        {
            printf("Failed to load palette 2");
            wait_forever();
        }
    }

    // Render some text to a buffer and load it to VRAM as a texture

    int textureID3;
    size_t out_width, out_height;

    {
        void *out_texture;
        dsf_error r = DSF_StringRenderToTexture(handle2,
                            "VAWATa\ntajl", GL_RGB16,
                            font2_0_16Bitmap, 128, 128,
                            &out_texture, &out_width, &out_height);
        if (r != DSF_NO_ERROR)
        {
            printf("DSF_StringRenderToTexture() failed: %d", r);
            wait_forever();
        }

        glGenTextures(1, &textureID3);
        glBindTexture(0, textureID3);

        if (glTexImage2D(0, 0, GL_RGB16, out_width, out_height, 0,
                         TEXGEN_TEXCOORD | GL_TEXTURE_COLOR0_TRANSPARENT,
                         out_texture) == 0)
        {
            printf("Failed to load texture 3");
            wait_forever();
        }
        if (glColorTableEXT(0, 0, 16, 0, 0, font2_0_16Pal) == 0)
        {
            printf("Failed to load palette 3");
            wait_forever();
        }

        free(out_texture);
    }

    int x = 30;
    int y = 120;

    while (1)
    {
        // Synchronize game loop to the screen refresh
        swiWaitForVBlank();

        // Print some text in the demo console
        // -----------------------------------

        // Clear console
        printf("\x1b[2J");

        // Print some controls
        printf("PAD:    Move text\n");
        printf("START:  Exit to loader\n");
        printf("\n");

        // Handle user input
        // -----------------

        scanKeys();

        uint16_t keys = keysHeld();

        if (keys & KEY_LEFT)
            x--;
        if (keys & KEY_RIGHT)
            x++;

        if (keys & KEY_UP)
            y--;
        if (keys & KEY_DOWN)
            y++;

        if (keys & KEY_START)
            break;

        // Render 3D scene
        // ---------------

        int factor = 2;

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrthof32(0, 256 << factor, 192 << factor, 0, inttof32(1), inttof32(-1));

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glScalef32(inttof32(1 << factor), inttof32(1 << factor), inttof32(1));

        glColor3f(1, 1, 1);

        // Draw opaque text using a bunch of 3D quads

        glBindTexture(0, textureID);
        glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK);

        // This character isn't included in the font
        DSF_StringRender3D(handle,
                           "Sample: AWAV.\nÿ_ßðñÑü(o´Áá)|\nInvalid char: ŋ",
                           0, 0, 0);

        // Draw translucent text using a bunch of 3D quads using different
        // polygon IDs

        glBindTexture(0, textureID);

        DSF_StringRender3DAlpha(handle, "ID30", 10, 90, 0,
                                POLY_ALPHA(20) | POLY_CULL_BACK, 30);
        DSF_StringRender3DAlpha(handle, "ID50", 100, 90, 0,
                                POLY_ALPHA(20) | POLY_CULL_BACK, 50);

        glBindTexture(0, textureID2);

        DSF_StringRender3DAlpha(handle2, "ID50-vAwA", x, y, 0,
                                POLY_ALPHA(20) | POLY_CULL_BACK, 50);

        // Draw pre-rendered text as a single quad

        glBindTexture(0, textureID3);
        glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK);

        draw_textured_quad(x + 50, y + 20, out_width, out_height);

        // Render frame

        glFlush(GL_TRANS_MANUALSORT);
    }

    return 0;
}
