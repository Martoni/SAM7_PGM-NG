extern int establish_comm_uart(void);
extern int bootloader_version(char *str, int str_size);
extern int run_code(unsigned int addr);
extern int write_word(unsigned int addr, unsigned int word);
extern int read_word(unsigned int addr, unsigned int * word);
extern int read_memory(int addr, int len);
extern int write_memory(int addr, const unsigned char *data, int len);

