extern int read_intel_hex(const char *filename);
extern int bytes_within_range(int begin, int end);
extern void get_ihex_data(int addr, int len, unsigned char *bytes);
extern void put_ihex_data(int addr, int len, const unsigned char *bytes);

#define MAX_MEMORY_SIZE 0x80000
extern unsigned char firmware_image[MAX_MEMORY_SIZE];
extern unsigned char firmware_mask[MAX_MEMORY_SIZE];


