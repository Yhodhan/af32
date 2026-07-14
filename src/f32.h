#pragma once

#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

// this is not portable across different systems
// as this is not intended to be use in the wild is okay
enum FAT_TYPE { FAT12, FAT16, FAT32 };

enum DIR_ATTR {
  ATTR_READ_ONLY = 0x01,
  ATTR_HIDDEN = 0x02,
  ATTR_SYSTEM = 0x04,
  ATTR_VOLUME_ID = 0x08,
  ATTR_ARCHIVE = 0x20,
  ATTR_LONG_NAME = ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID
};

struct __attribute__((packed)) BPB {
  uint8_t bs_jmpBoot[3];
  uint8_t bs_OEMName[8];
  uint16_t bpb_BytesPerSec;
  uint8_t bpb_SecPerClus;
  uint16_t bpb_RsvdSecCnt;
  uint8_t bpb_NumFats;
  uint16_t bpb_RootEntCnt;
  uint16_t bpb_TotSec16;
  uint8_t bpb_Media;
  uint16_t bpb_FATSz16;
  uint16_t bpb_SecPerTrk;
  uint16_t bpb_NumHeads;
  uint32_t bpb_HiddSec;
  uint32_t bpb_TotSec32;
  // exclusive fields of 32 fat
  uint32_t bpb_FATSz32;
  uint16_t bpb_ExtFlags;
  uint16_t bpb_FSVer;
  uint32_t bpb_RootClus;
  uint16_t bpb_FSInfo;
  uint16_t bpb_BkBootSec;
  uint8_t bpb_Reserved[12];
  uint8_t bs_DrvNum;
  uint8_t bs_Reserved;
  uint8_t bs_BootSig;
  uint32_t bs_VolID;
  uint8_t bs_VolLab[11];
  uint8_t bs_FilSysType[8];
  uint8_t bs_BootCode32[420];
  uint16_t bs_Sign;
};

struct __attribute__((packed)) FSInfo {
  uint32_t FSI_LeadSig = 0x41615252;
  uint8_t FSI_Reserved1[480] = {};
  uint32_t FSI_StrucSig = 0x61417272;
  uint32_t FSI_Free_Count = 0xFFFFFFFF;
  uint32_t FSI_Nxt_Free = 0xFFFFFFFF;
  uint8_t FSI_Reserved2[12] = {};
  uint32_t FSI_TrailSig = 0xAA550000;
};

struct __attribute__((packed)) DirEntry {
  uint8_t DIR_Name[11];
  uint8_t DIR_Attr;
  uint8_t DIR_NTRes;
  uint8_t DIR_CrtTimeTenth;
  uint16_t DIR_CrtTime;
  uint16_t DIR_CrtDate;
  uint16_t DIR_LstAccDate;
  uint16_t DIR_FstClusHI;
  uint16_t DIR_WrtTime;
  uint16_t DIR_WritDate;
  uint16_t DIR_FstClusLO;
  uint32_t DIR_FileSize;
};

class F32 {
public:
  F32(std::string &file);
  ~F32();

  void debug_bpb();

  uint32_t get_fat_entry(uint32_t N);
  uint32_t cluster_to_sector(uint32_t N);
  std::vector<uint8_t> read_cluster(uint32_t cluster);
  std::vector<uint32_t> get_cluster_chain(uint32_t start_cluster);
  std::vector<DirEntry> read_directory(uint32_t start_cluster);

private:
  std::ifstream disk;
  std::unique_ptr<BPB> bpb;

  uint32_t FATSz;
  uint32_t data_sec;
  FAT_TYPE fat_type;
  uint32_t total_sectors;
  uint32_t first_fat_sector;
  uint32_t first_data_sector;
  uint32_t count_of_clusters;

  // ----------------------------
  //         Exceptions
  // ----------------------------
  class fileException : public std::exception {
  public:
    virtual const char *what() const throw();
  };
};
