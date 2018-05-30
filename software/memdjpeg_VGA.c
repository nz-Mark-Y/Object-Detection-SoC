
/****************************************************************************************
 * The following defines are used to configure the required operation modes
 * 
 * IM_DECODE: This only has one mode at the moment
 * 
 * IM_PROCESS: This sets the desired filter stage processor
 *     Valid Options:
 *     - 'windowFilter' - The standard supplied software based filter
 *     - 'FPGAFilter' - Uses FPGA components to accelerate processing speed
 *     - 'improvedSoftwareFilter' (PREFERRED) - Improved & optimised software filter
 * 
 * IM_OUTPUT:  This only has one mode at the moment
 * 
 * WINDOW_FUNC: This sets the desired filter process to use
 *  Valid Options:
 *     - 'medianFilter'
 *     - 'convolutionFilter'
 *     - 'sobelFilter' (PREFERRED) 
****************************************************************************************/
#define IM_DECODE decodeJpeg
#define IM_PROCESS improvedSoftwareFilter
#define IM_OUTPUT outputVGA
#define WINDOW_FUNC sobelFilter

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/stat.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/time.h>

#include <jpeglib.h>
#include "ecrypt-sync.h"
#include "trivium.c"
#include "ShapeDetector.cpp"

#define VGA
#define WRITE_ALL 0
#define MAXKEYSIZEB ((ECRYPT_MAXKEYSIZE + 7) / 8)
#define MAXIVSIZEB ((ECRYPT_MAXIVSIZE + 7) / 8)

#ifdef VGA
    #include <sys/types.h>
    #include <sys/ipc.h>
    #include <sys/shm.h>
    #include <sys/mman.h>

    #include "arm_a9_hps_0.h"

    #define soc_cv_av
    #include "socal/socal.h"
    #include "socal/hps.h"
    #include "socal/alt_gpio.h"

    #define HW_REGS_BASE ( ALT_STM_OFST )
    #define HW_REGS_SPAN ( 0x04000000 )
    #define HW_REGS_MASK ( HW_REGS_SPAN - 1 )

    #define FPGA_AXI_BASE 0xC0000000
    #define FPGA_ONCHIP_BASE (FPGA_AXI_BASE + ONCHIP_SRAM_BASE)
    #define FPGA_ONCHIP_SPAN 0x00080000

	#define ALT_AXI_FPGASLVS_OFST (0xC0000000) // axi_master
	#define HW_FPGA_AXI_SPAN (0x40000000) // Bridge span 1GB
	#define HW_FPGA_AXI_MASK ( HW_FPGA_AXI_SPAN - 1 )

    #define FPGA_CHAR_BASE (FPGA_AXI_BASE + VGA_SUBSYSTEM_VGA_CHAR_BUFFER_AVALON_CHAR_BUFFER_SLAVE_BASE)
    #define FPGA_CHAR_SPAN VGA_SUBSYSTEM_VGA_CHAR_BUFFER_AVALON_CHAR_BUFFER_SLAVE_SPAN

    /* function prototypes */
    void VGA_text (int, int, char *);
    void VGA_text_clear();
    void VGA_box (int, int, int, int, short);

    // pixel buffer
    volatile unsigned int * vga_pixel_ptr = NULL ;
    void *vga_pixel_virtual_base;

    // character buffer
    volatile unsigned int * vga_char_ptr = NULL ;
    void *vga_char_virtual_base;
#endif /* VGA */

struct bmp_out_struct {
    unsigned char *bmp_buffer;
    unsigned long bmp_size;
    int row_stride;
    int width;
    int height;
    int pixel_size;
};

// Memory maps for filter interface
void *virtual_base;
void *h2p_pio_bridge0_addr;
void *h2p_pio_bridge1_addr;

unsigned char *bmp_buffer;

int trivium_decrypt_file(char* infile, char* outfile);
int trivium_test_cipher(void);
int FPGAFilter(struct bmp_out_struct *bmp_out);
int decodeMjpeg(unsigned char *mjpeg_buffer, unsigned long mjpeg_size);
int decodeJpeg(struct jpeg_decompress_struct *cinfo, struct bmp_out_struct *bmp_out);
int outputBmp(struct bmp_out_struct *bmp_out);
int outputVGA(struct bmp_out_struct *bmp_out);

/****************************************************************************************
 * Functions for decoding/encoding trivium files
****************************************************************************************/

int trivium_file(char *encrypted, char *decrypted, const u8 key[], const u8 iv[]) {
    //Note that the cipher is reversible, so passing in a decrypted file as the first argument will
    //  result in an encrypted output

    ECRYPT_ctx ctx;

    // Initialise cipher
    ECRYPT_init();
    ECRYPT_keysetup(&ctx, key, ECRYPT_MAXKEYSIZE, ECRYPT_MAXIVSIZE);
    ECRYPT_ivsetup(&ctx, iv);

    #define CHUNK 1024
    char buf[CHUNK];
    char outbuf[CHUNK];
    FILE *infile, *outfile;
    size_t nread;

    // Open file streams
    infile = fopen(encrypted, "rb");
    outfile = fopen(decrypted, "w+");
    if (infile && outfile) {

        while ((nread=fread(buf, 1, sizeof(buf), infile)) > 0) {
            ECRYPT_encrypt_blocks(&ctx, (u8*)buf, (u8*)outbuf, (CHUNK/ECRYPT_BLOCKLENGTH));
            if (nread%4) {
                size_t start = (CHUNK/ECRYPT_BLOCKLENGTH) * ECRYPT_BLOCKLENGTH;
                ECRYPT_encrypt_bytes(&ctx, (u8*)(buf+start), (u8*)(outbuf+start), nread%4);
            }
            fwrite(outbuf, 1, nread, outfile);
        }
        if (ferror(infile) || ferror(outfile)) {
            /* deal with error */
        }
        fclose(infile);
        fclose(outfile);

        return 0;
    }
	return 1;
}

int trivium_decrypt_file(char* infile, char* outfile) {

    // Keys
    //TODO make these read as input
    const u8 key[] = "Test key01";
    const u8 iv[] = "So Random\0";

    // Execute code
    int ret = trivium_file(infile, outfile, key, iv);

    return ret;
}

/****************************************************************************************
 * Function to filter an individual frame
 * Utilises software system
****************************************************************************************/

int medianFilter(unsigned char values[9]) {
    // Sort values like a horrible person
    int sorted = 0;
    while (!sorted) {
        sorted = 1;
        for (int i=1; i<9; ++i) {
            if (values[i-1] > values[i]) {
                unsigned char temp = values[i];
                values[i] = values[i-1];
                values[i-1] = temp;
                sorted = 0;
            }
        }
    }
    return values[4];
}

int convolutionFilter(unsigned char values[9]) {
    #if 0 // Edge detection
    const int kernel[9] = { 0,  1,  0,
                            1, -4,  1,
                            0,  1,  0 };
    const int div = 1;
    #elif 1 // Gaussian
    const int kernel[9] = { 1,  2,  1,
                            2,  4,  2,
                            1,  2,  1 };
    const int div = 16;
    #else // Identity
    const int kernel[9] = { 0,  0,  0,
                            0,  1,  0,
                            0,  0,  0 };
    const int div = 1;
    #endif

    int sum = 0;
    for (int i=0; i<9; ++i) {
        sum += kernel[i]*(int)values[i];
    }
    return (int)(sum / div);
}

int sobelFilter(unsigned char values[9]) {
    const int sobel_x[9] = { 1,  0, -1,
                             2,  0, -2,
                             1,  0, -1 };
    const int sobel_y[9] = { 1,  2,  1,
                             0,  0,  0,
                            -1, -2, -1 };

    int sumx = 0;
    int sumy = 0;
    for (int i=0; i<9; ++i) {
        sumx += sobel_x[i]*values[i];
        sumy += sobel_y[i]*values[i];
    }
    return (int)sqrt(sumx*sumx + sumy*sumy);
}

int windowFilter(struct bmp_out_struct *bmp_out) {
    // Take a few local copies to make the code a bit easier to read
    unsigned char *bmp_buffer = bmp_out->bmp_buffer;
    // Filter
    unsigned char *bmp_processed = (unsigned char*) malloc(bmp_out->bmp_size);
    // Iterate over full image
    for (int row=0; row<bmp_out->height; ++row) {
        for (int col=0; col<bmp_out->width; ++col) {
            // Work on each channel independently
            for (int chan=0; chan<bmp_out->pixel_size; ++chan) {
                // Black border
                unsigned char middle = 0;
                if ((row>0 && row<bmp_out->height-1) && (col>0 && col<bmp_out->width-1)) {
                    #define val(row, col) bmp_buffer[(row)*bmp_out->row_stride + (col)*bmp_out->pixel_size + chan]
                    // Load all adjacent values
                    unsigned char values[9] = {
                        val(row-1, col-1), val(row-1, col), val(row-1, col+1),
                        val(row-0, col-1), val(row-0, col), val(row-0, col+1),
                        val(row+1, col-1), val(row+1, col), val(row+1, col+1),
                    };
                    #undef val
                    middle = WINDOW_FUNC(values);
                }
                // Set output
                bmp_processed[(row*bmp_out->row_stride) + col*bmp_out->pixel_size + chan] = middle;
            }
        }
    }

    // Replace input with output
    memcpy(bmp_buffer, bmp_processed, bmp_out->bmp_size);
    return 1;
}

int improvedSoftwareFilter(struct bmp_out_struct *bmp_out) { return 1; }

/****************************************************************************************
 * Function to filter an individual frame
 * Utilises hardware system
****************************************************************************************/

int FPGAFilter(struct bmp_out_struct *bmp_out) {
    #ifdef PIO_0_COMPONENT_TYPE
    #ifdef PIO_1_COMPONENT_TYPE
        bmp_buffer = bmp_out->bmp_buffer;
        u32 incoming_packet;
        u32 temp;        
        unsigned int id = 0;
        unsigned int expected_id = 0;
        unsigned int id_flag = 0;
        u32 filter_type = 0x00000007; // set sobel filter
        #define val(row, col) bmp_buffer[(row)*bmp_out->row_stride + (col)*bmp_out->pixel_size]
        
        unsigned char *bmp_processed = (unsigned char*) malloc(bmp_out->bmp_size); // Create buffer to hold processed pixels
        for (int row = 0; row < bmp_out->height; ++row) {
            for (int col = 0; col < bmp_out->width; ++col) {
                if ((row > 0 && row < bmp_out->height - 1) && (col >= 0 && col < bmp_out->width - 1)) { // Don't process border pixels
                    temp = ((val(row-1, col) << 24 ) | (val(row-0, col) << 16) | (val(row+1, col) << 8) | (filter_type << 5) | id_flag);
                    *(uint32_t *)h2p_pio_bridge0_addr = temp;
                    id_flag++;  // Set an ID for the 3x3 window
                    if (id_flag > 29) { id_flag = 0; }

                    if (col == 0) { expected_id = expected_id + 2; }
                    if (col > 1) {
                        while (expected_id != id) { // Recieve filtered pixel
                            const int fpga_bridge_mask = (1 << (PIO_1_DATA_WIDTH)) - 1;
                            incoming_packet = (u32)(*(uint32_t *)h2p_pio_bridge1_addr & fpga_bridge_mask);
                            id = (int)((incoming_packet & 0x1F00) >> 8);
                        }
                        expected_id++;
                        if (expected_id > 29) { expected_id = 0; }
                        bmp_processed[(row*bmp_out->row_stride) + (col-1)*bmp_out->pixel_size] = (char)(incoming_packet & 0x00FF); // Cast to char
                    }
                } else {                       
                    bmp_processed[(row*bmp_out->row_stride) + col*bmp_out->pixel_size] = 0; // Black border
                }                   
                
            }
        }
        #undef val

        // Replace input with output
        memcpy(bmp_buffer, bmp_processed, bmp_out->bmp_size);
        return 1;
    #endif
    #endif
    return 0;
}

/****************************************************************************************
 * Main
 * Sets up system, decrypts, then calls decodeMjpeg
****************************************************************************************/

int main (int argc, char *argv[]) {
	int rc, i;
    int fd;

	char *syslog_prefix = (char*) malloc(1024);
	sprintf(syslog_prefix, "%s", argv[0]);
	openlog(syslog_prefix, LOG_PERROR | LOG_PID, LOG_USER);

	if (argc != 3) {
		fprintf(stderr, "USAGE: %s encrypted_file unencrypted_file\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	char *encfile = argv[1];
	char *infile = argv[2];

    #ifdef VGA
        // === get FPGA addresses ==================
        // Open /dev/mem
        if((fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1) {
            printf( "ERROR: could not open \"/dev/mem\"...\n" );
            return( 1 );
        }

        // === get VGA char addr =====================
        // get virtual addr that maps to physical
        vga_char_virtual_base = mmap( NULL, FPGA_CHAR_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, FPGA_CHAR_BASE );
        if( vga_char_virtual_base == MAP_FAILED ) {
            printf( "ERROR: mmap2() failed...\n" );
            close( fd );
            return(1);
        }

        virtual_base = mmap( NULL, HW_FPGA_AXI_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, ALT_AXI_FPGASLVS_OFST );
        if(virtual_base == MAP_FAILED) {
            printf("ERROR: mmap() failed...\n");
            close(fd);
            return(1);
        }

        h2p_pio_bridge0_addr = virtual_base + ((unsigned long)(0x0 + PIO_0_BASE) & (unsigned long)(HW_FPGA_AXI_MASK));
        h2p_pio_bridge1_addr = virtual_base + ((unsigned long)(0x0 + PIO_1_BASE) & (unsigned long)(HW_FPGA_AXI_MASK));

        vga_char_ptr =(unsigned int *)(vga_char_virtual_base); // Get the address that maps to the FPGA LED control

        // === get VGA pixel addr ====================
        // get virtual addr that maps to physical
        vga_pixel_virtual_base = mmap( NULL, FPGA_ONCHIP_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, FPGA_ONCHIP_BASE);
        if( vga_pixel_virtual_base == MAP_FAILED ) {
            printf( "ERROR: mmap3() failed...\n" );
            close( fd );
            return(1);
        }
        vga_pixel_ptr =(unsigned int *)(vga_pixel_virtual_base); // Get the address that maps to the FPGA pixel buffer

        // ===========================================        
        VGA_box (0, 0, 639, 479, 0x00); // clear the screen       
        VGA_text_clear(); // clear the text
    #endif

	// Variables for the source jpg
	struct stat file_info;
	unsigned long mjpg_size;
	unsigned char *mjpg_buffer;
    
    trivium_decrypt_file(encfile, infile); // Unencrypt file	

    // Load the jpeg data from a file into a memory buffer for the purpose of this demonstration.
	rc = stat(infile, &file_info);
	if (rc) {
		syslog(LOG_ERR, "FAILED to stat source jpg");
		exit(EXIT_FAILURE);
	}
	mjpg_size = file_info.st_size;
	mjpg_buffer = (unsigned char*) malloc(mjpg_size + 100);

	fd = open(infile, O_RDONLY);
	i = 0;
	while (i < mjpg_size) {
		rc = read(fd, mjpg_buffer + i, mjpg_size - i);
		syslog(LOG_INFO, "Input: Read %d/%lu bytes", rc, mjpg_size-i);
		i += rc;
	}
	close(fd);

    rc = decodeMjpeg(mjpg_buffer, mjpg_size); // Decode MJPEG
	free(mjpg_buffer); // And free the input buffer

    return rc;
}

/****************************************************************************************
 * Function to decompress, filter, and output mjpeg
 * Takes pointer to encoded MJPEG (in memory), and size
****************************************************************************************/

int decodeMjpeg(unsigned char *mjpeg_buffer, unsigned long mjpeg_size) {
	syslog(LOG_INFO, "Proc: Create Decompress struct");
	// Allocate a new decompress struct, with the default error handler.
	// The default error handler will exit() on pretty much any issue,
	// so it's likely you'll want to replace it or supplement it with
	// your own.
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr; 
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	syslog(LOG_INFO, "Proc: Set memory buffer as source");
	// Configure this decompressor to read its data from a memory
	// buffer starting at unsigned char *jpg_buffer, which is jpg_size
	// long, and which must contain a complete jpg already.

	jpeg_mem_src(&cinfo, mjpeg_buffer, mjpeg_size);

    /* Injecting MJPEG support (from raw video format)
     *
     * The jpeg_mem_src will continue to read from the same buffer just fine
     *  on its own. All we need to do is keep going and catching it before it
     *  can fail (i.e. we need to detect the final frame).
     *
     * * * * * * * * Begin * * * * * * * */
    clock_t begin = clock();
    unsigned int jpg_count = 0;
    while (*(cinfo.src->next_input_byte)) {
        struct bmp_out_struct bmp_out; // Prepare

        IM_DECODE(&cinfo, &bmp_out); // Decode

        // Process
        #ifdef IM_PROCESS
            IM_PROCESS(&bmp_out);
        #endif

        objectDetect(bmp_out.bmp_buffer, bmp_out.bmp_size, bmp_out.height, bmp_out.width);

        // Output
        #ifdef IM_OUTPUT
            IM_OUTPUT(&bmp_out);
        #endif

        // Clean up
        free((void*)bmp_out.bmp_buffer);
        jpg_count++;
    }
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    syslog(LOG_INFO, "Successfully loaded %u JPEGs in %fs", jpg_count, time_spent);

	// Destroy the object to free everything
	jpeg_destroy_decompress(&cinfo);

	syslog(LOG_INFO, "End of decompression");
	return EXIT_SUCCESS;
}


/****************************************************************************************
 * Function to decode individual frame
 * Takes pointer to encoded JPEG (in memory), return a pointer to the decoded BMP
****************************************************************************************/
int decodeJpeg(struct jpeg_decompress_struct *cinfo, struct bmp_out_struct *bmp_out) {
    // Have the decompressor scan the jpeg header. This won't populate
    // the cinfo struct output fields, but will indicate if the
    // jpeg is valid.
    int rc = jpeg_read_header(cinfo, true);

    if (rc != 1) {
        syslog(LOG_ERR, "File does not seem to be a normal JPEG");
        exit(EXIT_FAILURE);
    }

    // By calling jpeg_start_decompress, you populate cinfo
    // and can then allocate your output bitmap buffers for
    // each scanline.
    jpeg_start_decompress(cinfo);

    bmp_out->width = cinfo->output_width;
    bmp_out->height = cinfo->output_height;
    bmp_out->pixel_size = cinfo->output_components;

    bmp_out->bmp_size = bmp_out->width * bmp_out->height * bmp_out->pixel_size;
    bmp_out->bmp_buffer = (unsigned char*) malloc(bmp_out->bmp_size);

    // The row_stride is the total number of bytes it takes to store an
    // entire scanline (row).
    bmp_out->row_stride = bmp_out->width * bmp_out->pixel_size;

    //cinfo->out_color_space = JCS_GRAYSCALE;

    // Now that you have the decompressor entirely configured, it's time
    // to read out all of the scanlines of the jpeg.
    //
    // By default, scanlines will come out in RGBRGBRGB...  order,
    // but this can be changed by setting cinfo->out_color_space
    //
    // jpeg_read_scanlines takes an array of buffers, one for each scanline.
    // Even if you give it a complete set of buffers for the whole image,
    // it will only ever decompress a few lines at a time. For best
    // performance, you should pass it an array with cinfo->rec_outbuf_height
    // scanline buffers. rec_outbuf_height is typically 1, 2, or 4, and
    // at the default high quality decompression setting is always 1.
    while (cinfo->output_scanline < cinfo->output_height) {
        unsigned char *buffer_array[1];
        buffer_array[0] = bmp_out->bmp_buffer + \
                           (cinfo->output_scanline) * bmp_out->row_stride;

        jpeg_read_scanlines(cinfo, buffer_array, 1);

    }
    // Once done reading *all* scanlines, release all internal buffers,
    // etc by calling jpeg_finish_decompress. This lets you go back and
    // reuse the same cinfo object with the same settings, if you
    // want to decompress several jpegs in a row.
    //
    // If you didn't read all the scanlines, but want to stop early,
    // you instead need to call jpeg_abort_decompress(cinfo)
    jpeg_finish_decompress(cinfo);
    return 1;
}
/****************************************************************************************
 * Subroutine to write to a bmp file
****************************************************************************************/
int outputBmp(struct bmp_out_struct *bmp_out) {
    static unsigned int jpg_count = 0;
    int ret = 0;
    // Only output every 240 frames
    if (WRITE_ALL || !(jpg_count % 240)) {
        // Write the decompressed bitmap out to a ppm file, just to make sure
        // it worked.
        char outfile[20];
        sprintf(outfile, "output%d.ppm", jpg_count);
        int fd = open(outfile, O_CREAT | O_WRONLY, 0666);
        char buf[1024];

        int rc = sprintf(buf, "P6 %d %d 255\n", bmp_out->width, bmp_out->height);
        write(fd, buf, rc); // Write the PPM image header before data
        write(fd, bmp_out->bmp_buffer, bmp_out->bmp_size); // Write out all RGB pixel data

        close(fd);

        syslog(LOG_INFO, "Output frame %u successful", jpg_count);
        ret = 1;
    }
    ++jpg_count;
    return ret;
}

/****************************************************************************************
 * Subroutine to write frame to VGA monitor
****************************************************************************************/

int outputVGA(struct bmp_out_struct *bmp_out) {
    #ifdef VGA
        char *pixel_ptr;
        #define val(row, col, chan) bmp_out->bmp_buffer[(row)*bmp_out->row_stride + (col)*bmp_out->pixel_size + chan]
        for (int j=1; j<bmp_out->height; ++j) {
            if (j >= 480) break;
            for (int i=1; i<bmp_out->width; ++i) {
                if (i >= 640) break;
                pixel_ptr = (char *)vga_pixel_ptr + ((j)<<10) + (i);
                *(char *)pixel_ptr = (val(j, i, 0) & 0xe0) >> 0 | (val(j, i, 1) & 0xe0) >> 3 | (val(j, i, 2) & 0xc0) >> 6;
            }
        }
        return 1;
    #else
        return 0;
    #endif
}

#ifdef VGA
    /****************************************************************************************
     * Subroutine to send a string of text to the VGA monitor
    ****************************************************************************************/
    void VGA_text(int x, int y, char * text_ptr) {
        volatile char * character_buffer = (char *) vga_char_ptr ;	// VGA character buffer
        int offset;
        /* assume that the text string fits on one line */
        offset = (y << 7) + x;
        while ( *(text_ptr) ) {
            // write to the character buffer
            *(character_buffer + offset) = *(text_ptr);
            ++text_ptr;
            ++offset;
        }
    }

    /****************************************************************************************
     * Subroutine to clear text to the VGA monitor
    ****************************************************************************************/
    void VGA_text_clear() {
        volatile char * character_buffer = (char *) vga_char_ptr ;	// VGA character buffer
        int offset, x, y;
        for (x=0; x<70; x++){
            for (y=0; y<40; y++){
        /* assume that the text string fits on one line */
                offset = (y << 7) + x;
                // write to the character buffer
                *(character_buffer + offset) = ' ';
            }
        }
    }

    /****************************************************************************************
     * Draw a filled rectangle on the VGA monitor
    ****************************************************************************************/
    #define SWAP(X,Y) do{int temp=X; X=Y; Y=temp;}while(0)

    void VGA_box(int x1, int y1, int x2, int y2, short pixel_color) {
        char  *pixel_ptr ;
        int row, col;

        /* check and fix box coordinates to be valid */
        if (x1>639) x1 = 639;
        if (y1>479) y1 = 479;
        if (x2>639) x2 = 639;
        if (y2>479) y2 = 479;
        if (x1<0) x1 = 0;
        if (y1<0) y1 = 0;
        if (x2<0) x2 = 0;
        if (y2<0) y2 = 0;
        if (x1>x2) SWAP(x1,x2);
        if (y1>y2) SWAP(y1,y2);
        for (row = y1; row <= y2; row++) {
            for (col = x1; col <= x2; ++col) {
                //640x480
                pixel_ptr = (char *)vga_pixel_ptr + (row<<10) + col ;
                // set pixel color
                *(char *)pixel_ptr = pixel_color;
            }
		}
    }
	
#endif

