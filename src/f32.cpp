#include "f32.h"
#include <memory>

void F32::debug_bpb() {

  for (unsigned int i = 0; i < 3; i++) {
    printf("byte jmpBoot %d is: %x \n", i, bpb->bs_jmpBoot[i]);
  }

  for (unsigned int i = 0; i < 8; i++) {
    printf("byte OEMName %d is: %x \n", i, bpb->bs_jmpBoot[i]);
  }

  printf("bytes per sec %d \n", bpb->bpb_BytesPerSec);
  printf("sectors per cluster %d \n", bpb->bpb_SecPerClus);
  printf("Rsv Sec CNT %d \n", bpb->bpb_RsvdSecCnt);
  printf("Num of Fats %d \n", bpb->bpb_NumFats);
  printf("Tot Sec 16 %d \n", bpb->bpb_TotSec16);
  printf("Tot Sec 32 %d \n", bpb->bpb_TotSec32);

  printf("File system type: ");
  for (unsigned int i = 0; i < 8; i++) {
    printf("%c", bpb->bs_FilSysType[i]);
  }
  printf("\n");
}

void F32::parse_sector(std::ifstream &disk) {
  // extract the jump sector
  disk.read(reinterpret_cast<char *>(bpb->bs_jmpBoot), 3);
  disk.read(reinterpret_cast<char *>(bpb->bs_OEMName), 8);
  disk.read(reinterpret_cast<char *>(&bpb->bpb_BytesPerSec), 2);
  disk.read(reinterpret_cast<char *>(&bpb->bpb_SecPerClus), 1);
  disk.read(reinterpret_cast<char *>(&bpb->bpb_RsvdSecCnt), 2);
  disk.read(reinterpret_cast<char *>(&bpb->bpb_NumFats), 1);
  disk.read(reinterpret_cast<char *>(&bpb->bpb_RootEntCnt), 2);
  disk.read(reinterpret_cast<char *>(&bpb->bpb_TotSec16), 2);
  disk.read(reinterpret_cast<char *>(&bpb->bpb_Media), 1);
  disk.read(reinterpret_cast<char *>(&bpb->bpb_FATSz16), 2);
  disk.read(reinterpret_cast<char *>(&bpb->bpb_SecPerTrk), 2);
  disk.read(reinterpret_cast<char *>(&bpb->bpb_NumHeads), 2);
  disk.read(reinterpret_cast<char *>(&bpb->bpb_HiddSec), 4);
  disk.read(reinterpret_cast<char *>(&bpb->bpb_TotSec32), 4);
  disk.read(reinterpret_cast<char *>(&bpb->bpb_FATSz32), 4);
  disk.read(reinterpret_cast<char *>(&bpb->bpb_ExtFlags), 2);
  disk.read(reinterpret_cast<char *>(&bpb->bpb_FSVer), 2);
  disk.read(reinterpret_cast<char *>(&bpb->bpb_RootClus), 4);
  disk.read(reinterpret_cast<char *>(&bpb->bpb_FSInfo), 2);
  disk.read(reinterpret_cast<char *>(&bpb->bpb_BkBootSec), 2);
  disk.read(reinterpret_cast<char *>(&bpb->bpb_Reserved), 12);
  disk.read(reinterpret_cast<char *>(&bpb->bs_DrvNum), 1);
  disk.read(reinterpret_cast<char *>(&bpb->bs_BootSig), 1);
  disk.read(reinterpret_cast<char *>(&bpb->bs_VolID), 4);
  disk.read(reinterpret_cast<char *>(&bpb->bs_VolLab), 11);
  disk.read(reinterpret_cast<char *>(&bpb->bs_FilSysType), 8);
  disk.read(reinterpret_cast<char *>(&bpb->bs_BootCode32), 420);
  disk.read(reinterpret_cast<char *>(&bpb->bs_Sign), 2);
}

F32::F32(const std::string &file) {

  std::ifstream disk(file, std::ios::binary);

  if (!disk) {
    throw F32::fileException();
  }

  this->bpb = std::unique_ptr<BPB>(new BPB());

  parse_sector(disk);
}

F32::~F32() {}

// ----------------------
//      Exceptions
// ----------------------
const char *F32::fileException::what() const throw() {
  return ("File opening error: ");
}
