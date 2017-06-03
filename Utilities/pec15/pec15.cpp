// Example program
#include <iostream>
#include <string>
#include <stdint.h>

uint16_t pec15Table[256];
uint16_t CRC15poly = 0x4599;

void init_PEC15_Table();
uint16_t pec15(char*, int);

int main()
{
  init_PEC15_Table();
  char cmd[2];
  cmd[0] = 0x07;
  cmd[1] = 0x21;
  std::cout << "got pec15: " << pec15(cmd, 2) << "\n";
}
void init_PEC15_Table()
{
    uint16_t remainder;
for (int i = 0; i < 256; i++)
{
 remainder = i << 7;
 for (int bit = 8; bit > 0; --bit)
 {
 if (remainder & 0x4000)
 {
 remainder = ((remainder << 1));
 remainder = (remainder ^ CRC15poly);
 }
 else
 {
 remainder = ((remainder << 1));
 }
 }
 pec15Table[i] = remainder&0xFFFF;
}
}
uint16_t pec15 (char *data , int len)
{
uint16_t remainder,address;
remainder = 16;//PEC seed
for (int i = 0; i < len; i++)
{
 address = ((remainder >> 7) ^ data[i]) & 0xff;//calculate PEC table address
 remainder = (remainder << 8 ) ^ pec15Table[address];
}
return (remainder*2);//The CRC15 has a 0 in the LSB so the final value must be multiplied by 2
}
