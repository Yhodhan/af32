#pragma once

#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
// this is not portable across different systems
// as this is not intended to be use in the wild is okay

typedef struct __attribute__((packed)) {
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
} BPB;

class F32 {
public:
  F32(const std::string &file);
  ~F32();

  void debug_bpb();
  void parse_sector(std::ifstream &disk);

private:
  std::unique_ptr<BPB> bpb;

  class fileException : public std::exception {
  public:
    virtual const char *what() const throw();
  };
};
