#include "unique_ptr.hpp"

#include "fat32.hpp"
#include "types.hpp"
#include "console.hpp"
#include "utils.hpp"

namespace {

//FAT 32 Boot Sector
struct fat_bs_t {
    uint8_t jump[3];
    char oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t number_of_fat;
    uint16_t root_directories_entries;
    uint16_t total_sectors;
    uint8_t media_descriptor;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_long;
    uint32_t sectors_per_fat_long;
    uint16_t drive_description;
    uint16_t version;
    uint32_t root_directory_cluster_start;
    uint16_t fs_information_sector;
    uint16_t boot_sectors_copy_sector;
    uint8_t filler[12];
    uint8_t physical_drive_number;
    uint8_t reserved;
    uint8_t extended_boot_signature;
    uint32_t volume_id;
    char volume_label[11];
    char file_system_type[8];
    uint8_t boot_code[420];
    uint16_t signature;
}__attribute__ ((packed));

struct fat_is_t {
    uint32_t signature_start;
    uint8_t reserved[480];
    uint32_t signature_middle;
    uint32_t free_clusters;
    uint32_t allocated_clusters;
    uint8_t reserved_2[12];
    uint32_t signature_end;
}__attribute__ ((packed));

static_assert(sizeof(fat_bs_t) == 512, "FAT Boot Sector is exactly one disk sector");

struct cluster_entry {
    char name[11];
    uint8_t attrib;
    uint8_t reserved;
    uint8_t creation_time_seconds;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t accessed_date;
    uint16_t cluster_high;
    uint16_t modification_time;
    uint16_t modification_date;
    uint16_t cluster_low;
    uint32_t file_size;
} __attribute__ ((packed));

static_assert(sizeof(cluster_entry) == 32, "A cluster entry is 32 bytes");

uint64_t cached_disk = -1;
uint64_t cached_partition = -1;
uint64_t partition_start;

fat_bs_t* fat_bs = nullptr;
fat_is_t* fat_is = nullptr;

void cache_bs(fat32::dd disk, const disks::partition_descriptor& partition){
    unique_ptr<fat_bs_t> fat_bs_tmp(new fat_bs_t());

    if(read_sectors(disk, partition.start, 1, fat_bs_tmp.get())){
        fat_bs = fat_bs_tmp.release();

        //TODO fat_bs->signature should be 0xAA55
        //TODO fat_bs->file_system_type should be FAT32
    } else {
        fat_bs = nullptr;
    }
}

void cache_is(fat32::dd disk, const disks::partition_descriptor& partition){
    auto fs_information_sector = partition.start + static_cast<uint64_t>(fat_bs->fs_information_sector);

    unique_ptr<fat_is_t> fat_is_tmp(new fat_is_t());

    if(read_sectors(disk, fs_information_sector, 1, fat_is_tmp.get())){
        fat_is = fat_is_tmp.release();

        //TODO fat_is->signature_start should be 0x52 0x52 0x61 0x41
        //TODO fat_is->signature_middle should be 0x72 0x72 0x41 0x61
        //TODO fat_is->signature_end should be 0x00 0x00 0x55 0xAA
    } else {
        fat_is = nullptr;
    }
}

uint64_t cluster_lba(uint64_t cluster){
    uint64_t fat_begin = partition_start + fat_bs->reserved_sectors;
    uint64_t cluster_begin = fat_begin + (fat_bs->number_of_fat * fat_bs->sectors_per_fat_long);

    return cluster_begin + (cluster - 2 ) * fat_bs->sectors_per_cluster;
}

bool entry_exists(const cluster_entry& entry){
    return !(entry.name[0] == 0x0 || static_cast<unsigned char>(entry.name[0]) == 0xE5);
}

bool is_long_name(const cluster_entry& entry){
    return entry.attrib == 0x0F;
}

vector<disks::file> files(const unique_heap_array<cluster_entry>& cluster){
    vector<disks::file> files;

    for(auto& entry : cluster){
        if(entry_exists(entry)){
            disks::file file;

            if(is_long_name(entry)){
                //It is a long file name
                //TODO Add suppport for long file name
                memcopy(file.name, "LONG", 4);
            } else {
                //It is a normal file name
                memcopy(file.name, entry.name, 11);
            }

            file.hidden = entry.attrib & 0x1;
            file.system = entry.attrib & 0x2;
            file.directory = entry.attrib & 0x10;
            file.size = entry.file_size;

            files.push_back(file);
        }
    }

    return move(files);
}

vector<disks::file> files(fat32::dd disk, uint64_t cluster_addr){
    unique_heap_array<cluster_entry> cluster(16 * fat_bs->sectors_per_cluster);

    if(read_sectors(disk, cluster_lba(cluster_addr), fat_bs->sectors_per_cluster, cluster.get())){
        return files(cluster);
    } else {
        return {};
    }
}

} //end of anonymous namespace

vector<disks::file> fat32::ls(dd disk, const disks::partition_descriptor& partition, const string& path){
    if(cached_disk != disk.uuid || cached_partition != partition.uuid){
        partition_start = partition.start;

        cache_bs(disk, partition);
        cache_is(disk, partition);

        cached_disk = disk.uuid;
        cached_partition = partition.uuid;
    }

    if(!fat_bs || !fat_is){
        //Something went wrong when reading the two base vectors
        return {};
    }

    auto cluster_addr = cluster_lba(fat_bs->root_directory_cluster_start);

    unique_heap_array<cluster_entry> root_cluster(16 * fat_bs->sectors_per_cluster);

    if(read_sectors(disk, cluster_addr, fat_bs->sectors_per_cluster, root_cluster.get())){
        if(path.empty()){
            return files(root_cluster);
        } else {
            for(auto& entry : root_cluster){
                if(entry_exists(entry) && !is_long_name(entry)){
                    if(path == entry.name){
                        return files(disk, entry.cluster_low + (entry.cluster_high << 16));
                    }
                }
            }
        }
    }

    return {};
}

uint64_t fat32::free_size(dd disk, const disks::partition_descriptor& partition){
    if(cached_disk != disk.uuid || cached_partition != partition.uuid){
        partition_start = partition.start;

        cache_bs(disk, partition);
        cache_is(disk, partition);

        cached_disk = disk.uuid;
        cached_partition = partition.uuid;
    }

    if(!fat_bs || !fat_is){
        //Something went wrong when reading the two base vectors
        return 0;
    }

    return fat_is->free_clusters * fat_bs->sectors_per_cluster * 512;
}