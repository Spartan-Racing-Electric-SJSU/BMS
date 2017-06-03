#ifndef TEST_MODE_H
#define TEST_MODE_H

void init_cfg();
void print_menu();
void run_command(uint16_t);
void print_config();
void print_rxconfig();
void print_cells();
void print_aux();
void print_menu();
void serial_print_hex(unsigned char);
void test_setup();
void test_loop();

#endif  // TEST_MODE_H
