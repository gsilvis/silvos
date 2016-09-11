#ifndef __SILVOS_ATA_H
#define __SILVOS_ATA_H

#define ATA_SR_ERR     0x01    /* Error */
#define ATA_SR_IDX     0x02    /* Inlex */
#define ATA_SR_CORR    0x04    /* Corrected data */
#define ATA_SR_DRQ     0x08    /* Data request ready */
#define ATA_SR_DSC     0x10    /* Drive seek complete */
#define ATA_SR_DF      0x20    /* Drive write fault */
#define ATA_SR_DRDY    0x40    /* Drive ready */
#define ATA_SR_BSY     0x80    /* Busy */

#define ATA_ER_AMNF     0x01    /* No address mark */
#define ATA_ER_TK0NF    0x02    /* Track 0 not found */
#define ATA_ER_ABRT     0x04    /* Command aborted */
#define ATA_ER_MCR      0x08    /* No media */
#define ATA_ER_IDNF     0x10    /* ID mark not found */
#define ATA_ER_MC       0x20    /* No media */
#define ATA_ER_UNC      0x40    /* Uncorrectable data */
#define ATA_ER_BBK      0x80    /* Bad sector */

#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC

#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01 /* read-only */
#define ATA_REG_FEATURES   0x01 /* write-only */
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07 /* write-only */
#define ATA_REG_STATUS     0x07 /* read-only */

#endif
