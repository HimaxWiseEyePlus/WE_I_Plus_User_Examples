#include "main_functions.h"
#include "hx_drv_tflm.h" 

int main(int argc, char* argv[]) {
  setup();
  while (true) {
    loop();
  }
}
