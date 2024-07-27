#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    uint8_t boot_jump_instruction[3];
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_count;
    uint16_t dir_entries_count;
    uint16_t total_sectors;
    uint8_t media_descriptor_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t heads;
    uint32_t hidden_sectors;
    uint32_t large_sector_count;
    uint8_t drive_number;
    uint8_t _reserved;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t system_id[8];
} 
#ifdef __linux__
__attribute__((packed)) 
#endif
boot_sector;

typedef struct {
    uint8_t name[11];
    uint8_t attributes;
    uint8_t _reserved;
    uint8_t create_time_tenths;
    uint16_t created_time;
    uint16_t created_date;
    uint16_t accessed_date;
    uint16_t first_cluster_high;
    uint16_t modified_time;
    uint16_t modified_date;
    uint16_t first_cluster_low;
    uint32_t size;
} 
#ifdef __linux__
__attribute__((packed)) 
#endif
dir_entry;

boot_sector bt_boot_sector;
uint8_t* fat = NULL;
dir_entry* root_directory = NULL;
uint32_t root_directory_end;

bool read_boot_sector(FILE* disk) {
    return fread(&bt_boot_sector, sizeof(bt_boot_sector), 1, disk) > 0;
}

bool read_sectors(FILE* disk, uint32_t lba, uint32_t count, void* buffer_out) {
    bool res = true;
    res = res && (fseek(disk, lba * bt_boot_sector.bytes_per_sector, SEEK_SET) == 0);
    res = res && (fread(buffer_out, bt_boot_sector.bytes_per_sector, count, disk) == count);
    return res;
}

bool read_fat(FILE* disk) {
    fat = (uint8_t*)malloc(bt_boot_sector.sectors_per_fat * bt_boot_sector.bytes_per_sector);
    if (!fat) return false;
    return read_sectors(disk, bt_boot_sector.reserved_sectors, bt_boot_sector.sectors_per_fat, fat);
}

bool read_root_directory(FILE* disk) {
    uint32_t lba = bt_boot_sector.reserved_sectors + bt_boot_sector.sectors_per_fat * bt_boot_sector.fat_count;
    uint32_t size = sizeof(dir_entry) * bt_boot_sector.dir_entries_count;
    uint32_t sectors = (size + bt_boot_sector.bytes_per_sector - 1) / bt_boot_sector.bytes_per_sector;
    root_directory_end = sectors + lba;
    root_directory = (dir_entry*)malloc(sectors * bt_boot_sector.bytes_per_sector);
    if (!root_directory) return false;
    return read_sectors(disk, lba, sectors, root_directory);
}

dir_entry* find_file(const char* name) {
    for (uint32_t i = 0; i < bt_boot_sector.dir_entries_count; i++) {
        if (memcmp(name, root_directory[i].name, 11) == 0) {
            return &root_directory[i];
        }
    }
    return NULL;
}

bool read_file(dir_entry* file_entry, FILE* disk, uint8_t* output_buffer) {
    bool res = true;
    uint16_t current_cluster = file_entry->first_cluster_low;
    do {
        uint32_t lba = root_directory_end + (current_cluster - 2) * bt_boot_sector.sectors_per_cluster;
        res = res && read_sectors(disk, lba, bt_boot_sector.sectors_per_cluster, output_buffer);
        output_buffer += bt_boot_sector.sectors_per_cluster * bt_boot_sector.bytes_per_sector;

        uint32_t fat_index = current_cluster * 3 / 2;
        if (current_cluster % 2 == 0) {
            current_cluster = (*(uint16_t*)(fat + fat_index)) & 0x0FFF;
        } else {
            current_cluster = (*(uint16_t*)(fat + fat_index)) >> 4;
        }
    } while (res && current_cluster < 0x0FF8);
    return res;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Syntax: %s {disk_image} {filename}\n", argv[0]);
        return 1;
    }
    const char* disk_image = argv[1];
    FILE* disk = fopen(disk_image, "rb");
    if (!disk) {
        fprintf(stderr, "Cannot open disk image '%s'\n", disk_image);
        return 1;
    }

    if (!read_boot_sector(disk)) {
        fprintf(stderr, "Could not read boot sector\n");
        fclose(disk);
        return 1;
    }
    if (!read_fat(disk)) {
        fprintf(stderr, "Could not read FAT\n");
        fclose(disk);
        free(fat);
        return 1;
    }

    if (!read_root_directory(disk)) {
        fprintf(stderr, "Could not read root directory\n");
        fclose(disk);
        free(fat);
        free(root_directory);
        return 1;
    }

    dir_entry* file_entry = find_file(argv[2]);
    if (!file_entry) {
        fprintf(stderr, "Could not find file '%s'\n", argv[2]);
        fclose(disk);
        free(fat);
        free(root_directory);
        return 1;
    }

    uint8_t* buffer = (uint8_t*)malloc(file_entry->size + bt_boot_sector.bytes_per_sector);
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(disk);
        free(fat);
        free(root_directory);
        return 1;
    }

    if (!read_file(file_entry, disk, buffer)) {
        fprintf(stderr, "Could not read file '%s'\n", argv[2]);
        fclose(disk);
        free(fat);
        free(root_directory);
        free(buffer);
        return 1;
    }

    for (size_t i = 0; i < file_entry->size; i++) {
        if (isprint(buffer[i])) {
            fputc(buffer[i], stdout);
        } else {
            printf("<%02x>", buffer[i]);
        }
    }
    printf("\n");

    free(buffer);
    free(fat);
    free(root_directory);
    fclose(disk);
    return 0;
}
