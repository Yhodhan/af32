#include "f32.h"

int main(void) {

  auto f32 = std::unique_ptr<F32>(new F32("fat32.img"));

  return 0;
}
