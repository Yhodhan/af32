#include "f32.h"
#include <cctype>
#include <cstdint>
#include <cstring>

void F32::debug_bpb() {

  for (unsigned int i = 0; i < 3; i++) {
    printf("byte jmpBoot %d is: %x \n", i, bpb->bs_jmpBoot[i]);
  }

  for (unsigned int i = 0; i < 8; i++) {
    printf("byte OEMName %d is: %x \n", i, bpb->bs_OEMName[i]);
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

void F32::parse_sector() {
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

F32::F32(std::string &file) : disk(file, std::ios::binary) {

  if (!disk) {
    throw F32::fileException();
  }

  this->bpb = std::unique_ptr<BPB>(new BPB());
  parse_sector();

  uint32_t root_dir_sector =
      ((this->bpb->bpb_RootEntCnt * 32) + (bpb->bpb_BytesPerSec - 1)) /
      bpb->bpb_BytesPerSec;

  // Compute main sectors

  this->first_fat_sector = bpb->bpb_RsvdSecCnt;

  this->FATSz = (bpb->bpb_FATSz16 != 0) ? bpb->bpb_FATSz16 : bpb->bpb_FATSz32;

  this->total_sectors =
      (bpb->bpb_TotSec16 != 0) ? bpb->bpb_TotSec16 : bpb->bpb_TotSec32;

  this->first_data_sector =
      bpb->bpb_RsvdSecCnt + (bpb->bpb_NumFats * FATSz) + root_dir_sector;

  this->data_sec = this->total_sectors -
                   (bpb->bpb_RsvdSecCnt + (bpb->bpb_NumFats * FATSz)) +
                   root_dir_sector;

  // NOTE: This is actually not needed because it is designed to work with FAT32
  // but is nice to have if planned to extend to FAT16 in future
  this->count_of_clusters = data_sec / bpb->bpb_SecPerClus;

  if (count_of_clusters < 4085) {
    this->fat_type = FAT12;
  } else if (count_of_clusters < 65525) {
    this->fat_type = FAT16;
  } else {
    this->fat_type = FAT32;
  }
}

F32::~F32() { disk.close(); }

// ----------------------
//     IO functions
// ----------------------
uint32_t F32::cluster_to_sector(uint32_t n) {
  return ((n - 2) * bpb->bpb_SecPerClus) + first_data_sector;
}

// ---------------------------------------------------------------------------
// This function is reading from the FAT table (the array of indexes).
// The input is the index of the cluster we want to read.
// ---------------------------------------------------------------------------
uint32_t F32::get_fat_entry(uint32_t cluster) {
  uint32_t FATOffset = (fat_type == FAT16) ? cluster * 2 : cluster * 4;

  uint32_t fat_sec_num =
      bpb->bpb_RsvdSecCnt + (FATOffset / bpb->bpb_BytesPerSec);

  uint32_t fat_ent_offset = FATOffset % bpb->bpb_BytesPerSec;

  std::vector<uint8_t> sec_buff(bpb->bpb_BytesPerSec);
  disk.seekg(fat_sec_num * bpb->bpb_BytesPerSec);
  disk.read(reinterpret_cast<char *>(sec_buff.data()), bpb->bpb_BytesPerSec);

  uint32_t val;
  std::memcpy(&val, &sec_buff[fat_ent_offset], 4);
  return val & 0x0FFFFFFF;
}

// ---------------------------------------------------------------------------
// This function brings all the chains of the cluster from the FAT table
// ---------------------------------------------------------------------------
std::vector<uint32_t> F32::get_cluster_chain(uint32_t start_cluster) {
  std::vector<uint32_t> chain;
  uint32_t cluster = start_cluster;

  while (cluster < 0x0FFFFFF8 and cluster != 0x0FFFFFF7) {
    chain.push_back(cluster);
    cluster = get_fat_entry(cluster);
  }

  return chain;
}

// ---------------------------------------------------------------------------
// This function reads from the real disk
// given a cluster number it calculates its offset and then read it from disk
// returns the raw bytes from the disk
// ---------------------------------------------------------------------------
std::vector<uint8_t> F32::read_cluster(uint32_t cluster) {
  uint32_t sector = cluster_to_sector(cluster);
  uint data_size = bpb->bpb_BytesPerSec * bpb->bpb_SecPerClus;
  std::vector<uint8_t> bytes(data_size);

  disk.seekg(sector * bpb->bpb_BytesPerSec);
  disk.read(reinterpret_cast<char *>(bytes.data()), data_size);

  return bytes;
}

// ---------------------------------------------------------------------------
// This function fetches all the clusters chain associated to a directory
// then reads from the disk those chunks of data
// it reiterpretates those raw bytes as an array of directory entries
// then read each directory entry to see their attr and stores in the return
// value the associeated entries
// ---------------------------------------------------------------------------
std::vector<DirEntry> F32::read_directory(uint32_t start_cluster) {
  std::vector<uint32_t> chain = get_cluster_chain(start_cluster);
  std::vector<uint8_t> data;

  for (auto cluster : chain) {
    auto bytes = read_cluster(cluster);
    data.insert(data.end(), bytes.begin(), bytes.end());
  }

  size_t count = data.size() / sizeof(DirEntry);
  std::vector<DirEntry> entries;
  const DirEntry *raw = reinterpret_cast<const DirEntry *>(data.data());

  for (size_t i = 0; i < count; i++) {
    if (raw[i].DIR_Name[0] == 0x00)
      break;
    if (raw[i].DIR_Name[0] == 0xE5)
      continue;
    if (raw[i].DIR_Attr == 0x0F)
      continue;
    entries.push_back(raw[i]);
  }

  return entries;
}

std::vector<std::string> split_path(const std::string path) {
  std::vector<std::string> result;
  std::string del = "/";
  size_t start = 0, pos;

  while ((pos = path.find(del)) != std::string::npos) {
    if (pos > start)
      result.push_back(path.substr(start, pos - start));
    start = pos + 1;
  }

  if (start < path.size())
    result.push_back(path.substr(start));

  return result;
}

std::string to_short_name(const std::string &file) {
  std::string result(11, ' ');

  std::string name = file;
  std::string ext;

  size_t dot = file.find('.');

  if (dot != std::string::npos) {
    name = file.substr(0, dot);
    ext = file.substr(dot + 1);
  }

  for (size_t i = 0; i < name.size() && i < 8; i++) {
    result[i] = std::toupper(static_cast<unsigned char>(name[i]));
  }

  for (size_t i = 0; i < ext.size() && i < 3; i++) {
    result[8 + i] = std::toupper(static_cast<unsigned char>(ext[i]));
  }

  return result;
}

bool F32::find_entry(const std::string &path, DirEntry &out) {
  auto parts = split_path(path);
  uint32_t current_cluster = this->bpb->bpb_RootClus;

  for (size_t i = 0; i < parts.size(); i++) {
    auto entries = read_directory(current_cluster);

    bool found = false;
    auto wanted = to_short_name(parts[i]);

    for (const auto &entry : entries) {
      std::string name(reinterpret_cast<const char *>(entry.DIR_Name), 11);
      if (name == wanted) {
        if (i == parts.size() - 1) {out = entry; return true;}
        if (!(entry.DIR_Attr & 0x10)) return false; // expected directory, got a file
        
        current_cluster = (entry.DIR_FstClusHI << 16) | entry.DIR_FstClusLO;
        found = true;
        break;
      }
    }

    if (!found)
      return false;
  }

  return false;
}

// ----------------------
//      Exceptions
// ----------------------
const char *F32::fileException::what() const throw() {
  return ("File opening error: ");
}
