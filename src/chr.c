#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "shadowdive/error.h"
#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/memreader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/chr.h"

#define UNUSED(x) (void)(x)

sd_chr_file* sd_chr_create() {
    sd_chr_file *chr = malloc(sizeof(sd_chr_file));
    memset(chr, 0, sizeof(sd_chr_file));
    return chr;
}

int sd_chr_load(sd_chr_file *chr, const char *filename) {
    sd_mreader *mr;
    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }

    // Make sure this looks like a CHR
    if(sd_read_udword(r) != 0xAFAEADAD) {
        goto error_0;
    }

    // Read player information
    mr = sd_mreader_open_from_reader(r, 444);
    sd_mreader_xor(mr, 0xB0);

    // Get player stats
    sd_mread_buf(mr, chr->name, 16);
    sd_mskip(mr, 2);
    chr->wins = sd_mread_uword(mr);
    chr->losses = sd_mread_uword(mr);
    chr->rank = sd_mread_ubyte(mr);
    chr->har = sd_mread_ubyte(mr);

    uint16_t stats_a = sd_mread_uword(mr);
    uint16_t stats_b = sd_mread_uword(mr);
    uint16_t stats_c = sd_mread_uword(mr);
    uint8_t stats_d = sd_mread_ubyte(mr);
    chr->arm_power = (stats_a >> 0) & 0x1F;
    chr->leg_power = (stats_a >> 5) & 0x1F;
    chr->arm_speed = (stats_a >> 10) & 0x1F;
    chr->leg_speed = (stats_b >> 0) & 0x1F;
    chr->armor     = (stats_b >> 5) & 0x1F;
    chr->stun_resistance = (stats_b >> 10) & 0x1F;
    chr->power = (stats_c >> 0) & 0x7F;
    chr->agility = (stats_c >> 7) & 0x7F;
    chr->endurance = (stats_d >> 0) & 0x7F;

    sd_mskip(mr, 5);

    chr->credits = sd_mread_udword(mr);
    chr->color_1 = sd_mread_ubyte(mr);
    chr->color_2 = sd_mread_ubyte(mr);
    chr->color_3 = sd_mread_ubyte(mr);

    sd_mread_buf(mr, chr->trn_name, 13);
    sd_mread_buf(mr, chr->trn_desc, 31);
    sd_mread_buf(mr, chr->trn_image, 13);

    sd_mskip(mr, 51);
    chr->difficulty = sd_mread_ubyte(mr);
    sd_mskip(mr, 10);
    sd_mread_buf(mr, chr->enhancements, 11);
    sd_mskip(mr, 69);
    chr->enemies_inc_unranked = sd_mread_uword(mr);
    chr->enemies_ex_unranked = sd_mread_uword(mr);

    // Close memory reader, we're done with the user block.
    sd_mreader_close(mr);

    // Read enemies block
    mr = sd_mreader_open_from_reader(r, 68 * chr->enemies_inc_unranked);
    sd_mreader_xor(mr, (chr->enemies_inc_unranked * 68) & 0xFF);

    // Handle enemy data
    for(int i = 0; i < chr->enemies_inc_unranked; i++) {
        // Reserve & zero out
        chr->enemies[i] = malloc(sizeof(sd_chr_enemy));
        memset(chr->enemies[i], 0, sizeof(sd_chr_enemy));

        // Read enemy data
        sd_mread_buf(mr, chr->enemies[i]->name, 16);
        sd_mskip(mr, 2);
        chr->enemies[i]->wins = sd_mread_uword(mr);
        chr->enemies[i]->losses = sd_mread_uword(mr);
        chr->enemies[i]->rank = sd_mread_ubyte(mr);
        chr->enemies[i]->har = sd_mread_ubyte(mr);
        
        sd_mskip(mr, 44);
    }

    // Close memory reader for enemy data block
    sd_mreader_close(mr);

    // Read HAR palette
    memset((void*)&chr->pal, 0, sizeof(sd_palette));
    char d[3];
    for(int k = 0; k < 48; k++) {
        sd_read_buf(r, d, 3);
        chr->pal.data[k][0] = ((d[0] << 2) | (d[0] >> 4));
        chr->pal.data[k][1] = ((d[1] << 2) | (d[1] >> 4));
        chr->pal.data[k][2] = ((d[2] << 2) | (d[2] >> 4));
    }

    // No idea what this is. TODO: Find out.
    sd_skip(r, 4);

    // Load sprite
    chr->photo = sd_sprite_create();
    int ret = sd_sprite_load(r, chr->photo);
    if(ret != SD_SUCCESS) {
        goto error_1;
    }

    // Fix photo size
    chr->photo->img->w++;
    chr->photo->img->h++;

    // Close & return
    sd_reader_close(r);
    return SD_SUCCESS;

error_1:
    for(int i = 0; i < chr->enemies_inc_unranked; i++) {
        if(chr->enemies[i] != NULL) {
            free(chr->enemies[i]);
        }
    }
    sd_sprite_delete(chr->photo);

error_0:
    sd_reader_close(r);
    return SD_FILE_PARSE_ERROR;
}

int sd_chr_save(sd_chr_file *chr, const char *filename) {
    sd_writer *w = sd_writer_open(filename);
    if(!w) {
        return SD_FILE_OPEN_ERROR;
    }

    // TODO

    // Close & return
    sd_writer_close(w);
    return SD_SUCCESS;
}

void sd_chr_delete(sd_chr_file *chr) {
    for(int i = 0; i < chr->enemies_inc_unranked; i++) {
        if(chr->enemies[i] != NULL) {
            free(chr->enemies[i]);
        }
    }
    sd_sprite_delete(chr->photo);
    free(chr);
}
