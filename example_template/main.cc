#include "hx_drv_tflm.h"

int main(int argc, char *argv[])
{
    hx_drv_uart_initial(UART_BR_115200);

    hx_drv_uart_print("Hello world!");
    return 0;
    

}