#include "bk.h"
#include "internal/reader.h"
#include "internal/writer.h"
#include "animation.h"
#include <stdlib.h>
#include <string.h>

sd_bk_file* sd_bk_load(const char *filename) {
    // Initialize reader
    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return 0;
    }

    // Allocate structure
    sd_bk_file *bk = malloc(sizeof(sd_bk_file));

    // Header
    bk->file_id = sd_read_udword(r);
    bk->unknown_a = sd_read_ubyte(r);
    bk->img_w = sd_read_uword(r);
    bk->img_h = sd_read_uword(r);
    memset(bk->anims, 0, sizeof(bk->anims));

    // Read animations
    uint8_t animno = 0;
    while(1) {
        sd_skip(r, 4);
        animno = sd_read_ubyte(r);
        if(animno >= 50 || !sd_reader_ok(r)) {
            break;
        }

        // Initialize animation
        bk->anims[animno] = sd_bk_anim_create();
        sd_bk_anim_load(r, bk->anims[animno]);
    }

    // Read background image
    bk->background = sd_vga_image_create(bk->img_w, bk->img_h);
    sd_read_buf(r, bk->background->data, bk->img_h * bk->img_w);

    // Read palettes
    bk->num_palettes = sd_read_ubyte(r);
    bk->palettes = malloc(bk->num_palettes * sizeof(sd_palette*));
    for(uint8_t i = 0; i < bk->num_palettes; i++) {
        bk->palettes[i] = (sd_palette*)malloc(sizeof(sd_palette));
        sd_palette_load(r, bk->palettes[i]);
    }

    // Read footer
    sd_read_buf(r, bk->footer, 30);

    // Close & return
    sd_reader_close(r);
    return bk;
}

int sd_bk_save(const char* filename, sd_bk_file *bk) {
    return 0;
}

void sd_bk_delete(sd_bk_file *bk) {
    sd_vga_image_delete(bk->background);
    for(int i = 0; i < 50; i++) {
        if(bk->anims[i]) {
            sd_bk_anim_delete(bk->anims[i]);
        }
    }
    for(int i = 0; i < bk->num_palettes; i++) {
        free(bk->palettes[i]);
    }
    free(bk->palettes);
    free(bk);
}
