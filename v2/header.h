unsigned char header_bin[] = {
  0x7f, 0x45, 0x4c, 0x46, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x86, 0x00, 0x00, 0x01, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x34, 0x00, 0x20, 0x00, 0x02, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x69, 0x69, 0x69, 0x69,
  0x69, 0x69, 0x69, 0x69, 0x05, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x69, 0x69, 0x69, 0x69,
  0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x31, 0xc0, 0xb0, 0x03,
  0xb3, 0x00, 0xcd, 0x80, 0xc3, 0x31, 0xc0, 0xb0, 0x04, 0xb3, 0x01, 0xcd,
  0x80, 0xc3, 0x31, 0xdb, 0x31, 0xd2, 0x42, 0xb9, 0x00, 0x00, 0x00, 0x80,
  0xbf, 0x69, 0x69, 0x69, 0x69, 0x89, 0xce
};
unsigned int header_bin_len = 151;
uint32_t header_tape_addr = 0x80000000;
uint32_t header_file_size_loc = 0x00000044;
uint32_t header_tape_size = 0x00000068;
uint32_t header_sub_input = 0x00000074;
uint32_t header_sub_output = 0x0000007D;
uint32_t header_tape_andmask = 0x00000091;
