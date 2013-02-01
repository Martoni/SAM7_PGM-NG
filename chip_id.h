
struct chip_info_struct {
	const char *name;	// part number
	unsigned int id_code;	// value read from ID reg @ 0xFFFFF240
	int numpages;		// number of flash pages
	int pagesize;		// size of each flash page
	int num_lock_bits;	// number of lock bits
	unsigned int flasharea;	// beginning of flash memory
	unsigned int ramarea;	// beginning of usable ram (while boot agent running)
	int set_nvm_bits;	// which general purpose NVM bits need to be set
	int clear_nvm_bits;	// which general purpose NVM bits need to be cleared
};

typedef const struct chip_info_struct chipinfo_t;

extern chipinfo_t * get_chip_info(unsigned int id);
extern unsigned int chip_lock_mask(chipinfo_t *chip);

