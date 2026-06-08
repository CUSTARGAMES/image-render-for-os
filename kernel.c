/* Simple BMP Viewer OS - All in one file */

#define VGA_WIDTH  640
#define VGA_HEIGHT 480
#define VGA_MEMORY 0xA0000

/* Multiboot module structure */
typedef struct {
    unsigned int mod_start;
    unsigned int mod_end;
    unsigned int cmdline;
    unsigned int pad;
} module_t;

/* Multiboot info structure */
typedef struct {
    unsigned int flags;
    unsigned int mem_lower;
    unsigned int mem_upper;
    unsigned int boot_device;
    unsigned int cmdline;
    unsigned int mods_count;
    unsigned int mods_addr;
} multiboot_info_t;

/* BMP header structures */
#pragma pack(push, 1)
typedef struct {
    unsigned short signature;   /* 'BM' */
    unsigned int   file_size;
    unsigned short reserved1;
    unsigned short reserved2;
    unsigned int   data_offset;
} bmp_header_t;

typedef struct {
    unsigned int   header_size;
    int            width;
    int            height;
    unsigned short planes;
    unsigned short bpp;
    unsigned int   compression;
    unsigned int   image_size;
    int            x_pixels_per_meter;
    int            y_pixels_per_meter;
    unsigned int   colors_used;
    unsigned int   colors_important;
} bmp_info_header_t;

typedef struct {
    unsigned char b;
    unsigned char g;
    unsigned char r;
    unsigned char reserved;
} bmp_palette_entry_t;
#pragma pack(pop)

/* Simple string functions */
int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s2 && *s1 == *s2) { s1++; s2++; }
    return *s1 - *s2;
}

unsigned int strlen(const char *s) {
    unsigned int len = 0;
    while (*s++) len++;
    return len;
}

/* Write to VGA register */
static void vga_write_reg(unsigned short port, unsigned char index, unsigned char value) {
    /* Some VGA registers need different handling */
    if (port == 0x3C0) {
        __asm__ volatile("inb $0x3DA, %%al\n\t" : : : "al");
        __asm__ volatile("outb %0, $0x3C0" : : "a"(index));
        __asm__ volatile("outb %0, $0x3C0" : : "a"(value));
    } else {
        __asm__ volatile("outb %0, %w1" : : "a"(index), "Nd"(port));
        __asm__ volatile("outb %0, %w1" : : "a"(value), "Nd"((unsigned short)(port + 1)));
    }
}

/* Set 640x480 16-color VGA mode (mode 0x12) */
void set_vga_mode(void) {
    /* Disable interrupts during mode switch */
    __asm__ volatile("cli");
    
    /* Set mode 0x12 (640x480 16 colors) */
    __asm__ volatile(
        "mov $0x0012, %%ax\n\t"
        "int $0x10\n\t"
        : : : "ax"
    );
    
    __asm__ volatile("sti");
}

/* Set VGA palette */
void set_palette(unsigned char *palette, int num
