/* Simple BMP Viewer OS */

#define VGA_WIDTH  640
#define VGA_HEIGHT 480

/* Multiboot structures */
typedef struct {
    unsigned int mod_start;
    unsigned int mod_end;
    unsigned int cmdline;
    unsigned int pad;
} module_t;

typedef struct {
    unsigned int flags;
    unsigned int mem_lower;
    unsigned int mem_upper;
    unsigned int boot_device;
    unsigned int cmdline;
    unsigned int mods_count;
    unsigned int mods_addr;
} multiboot_info_t;

/* BMP header */
#pragma pack(push, 1)
typedef struct {
    unsigned short signature;
    unsigned int file_size;
    unsigned short reserved1;
    unsigned short reserved2;
    unsigned int data_offset;
    unsigned int header_size;
    int width;
    int height;
    unsigned short planes;
    unsigned short bpp;
    unsigned int compression;
    unsigned int image_size;
    int x_pixels_per_meter;
    int y_pixels_per_meter;
    unsigned int colors_used;
    unsigned int colors_important;
} bmp_header_t;
#pragma pack(pop)

/* VGA registers */
#define VGA_AC_INDEX   0x3C0
#define VGA_AC_WRITE   0x3C0
#define VGA_AC_READ    0x3C1
#define VGA_DAC_WRITE  0x3C8
#define VGA_DAC_DATA   0x3C9
#define VGA_INSTAT     0x3DA

/* Set VGA 640x480 16-color mode */
void set_vga_mode() {
    __asm__ volatile(
        "mov $0x0012, %%ax\n"
        "int $0x10\n"
        : : : "ax"
    );
}

/* Set one palette color */
void set_palette_color(int index, unsigned char r, unsigned char g, unsigned char b) {
    __asm__ volatile("outb %0, %1" : : "a"((unsigned char)index), "Nd"(VGA_DAC_WRITE));
    __asm__ volatile("outb %0, %1" : : "a"(r), "Nd"(VGA_DAC_DATA));
    __asm__ volatile("outb %0, %1" : : "a"(g), "Nd"(VGA_DAC_DATA));
    __asm__ volatile("outb %0, %1" : : "a"(b), "Nd"(VGA_DAC_DATA));
}

/* Display BMP at given address */
void display_bmp(unsigned char *bmp_data) {
    bmp_header_t *header = (bmp_header_t *)bmp_data;
    
    /* Check signature */
    if (header->signature != 0x4D42) return;
    if (header->bpp != 8) return;
    
    int width = header->width;
    int height = header->height;
    
    /* Load palette */
    unsigned char *palette = bmp_data + 54; /* 54 = sizeof(bitmap header) */
    for (int i = 0; i < 256; i++) {
        set_palette_color(i, 
            palette[i * 4 + 2] >> 2,  /* Red */
            palette[i * 4 + 1] >> 2,  /* Green */
            palette[i * 4 + 0] >> 2   /* Blue */
        );
    }
    
    /* Get pixel data */
    unsigned char *pixels = bmp_data + header->data_offset;
    unsigned char *vga = (unsigned char *)0xA0000;
    
    /* BMP is bottom-up */
    for (int y = height - 1; y >= 0; y--) {
        for (int x = 0; x < width; x++) {
            /* Simple mapping - each byte = 2 pixels in 16-color mode */
            int pixel = pixels[y * width + x];
            int offset = y * (VGA_WIDTH / 8) + x / 8;
            int bit = 7 - (x % 8);
            
            unsigned char mask = 0x80 >> bit;
            if (pixel & 1) {
                vga[offset] |= mask;
            } else {
                vga[offset] &= ~mask;
            }
        }
    }
}

/* Main kernel entry */
void kernel_main(unsigned int magic, unsigned int addr) {
    set_vga_mode();
    
    multiboot_info_t *mbi = (multiboot_info_t *)addr;
    
    /* Check if modules were loaded */
    if (mbi->mods_count > 0) {
        module_t *mod = (module_t *)mbi->mods_addr;
        unsigned char *bmp_data = (unsigned char *)mod->mod_start;
        display_bmp(bmp_data);
    }
    
    /* Hang forever */
    while(1) {
        __asm__ volatile("hlt");
    }
}
