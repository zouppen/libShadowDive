#include <stdlib.h>
#include <string.h>

#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/error.h"
#include "shadowdive/palette.h"
#include "shadowdive/altpal.h"


sd_altpal_file* sd_altpal_create() {
    sd_altpal_file *ap = (sd_altpal_file*)malloc(sizeof(sd_altpal_file));
    memset(ap, 0, sizeof(sd_altpal_file));
    return ap;
}

int sd_altpals_load(sd_altpal_file *ap, const char *filename) {
    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }

    for(uint8_t i = 0; i < 11; i++) {
        sd_palette_load(r, &ap->palettes[i]);
    }

    // Close & return
    sd_reader_close(r);
    return SD_SUCCESS;
}

int sd_altpals_save(sd_altpal_file *ap, const char *filename) {
    sd_writer *w = sd_writer_open(filename);
    if(!w) {
        return SD_FILE_OPEN_ERROR;
    }

    for(uint8_t i = 0; i < 11; i++) {
        sd_palette_save(w, &ap->palettes[i]);
    }

    // All done.
    sd_writer_close(w);
    return SD_SUCCESS;
}

void sd_altpal_delete(sd_altpal_file *ap) {
    free(ap);
}

