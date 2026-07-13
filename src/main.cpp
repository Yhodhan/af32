#include "f32.h"

int main(void) {

  std::string disk = "fat32.img";
  
  auto f32 = std::unique_ptr<F32>(new F32(disk));

  return 0;
}
