#include "my_trivium.h"
#include "trivium.c"

// A mess of includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <math.h>
#include <time.h>

#define soc_cv_av
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_gpio.h"
#include "arm_a9_hps_0.h"
#include "hwlib.h"

#define HW_REGS_BASE ( ALT_STM_OFST )
#define HW_REGS_SPAN ( 0x04000000 )
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )

#define USE_FPGA 1

// Optionally reverse the byte order for displaying the key and iv
// Since the processor will reverse the order displaying it this way makes it
//	easier to copy and paste into vhdl with bit order 80 DOWNTO 1
#define REVERSE 1

void send_to_FPGA(u32 data, void *virtual_base) {
	#ifdef FPGA_BRIDGE_COMPONENT_NAME
		void *h2p_lw_fpga_bridge_addr = virtual_base + ((unsigned long)(ALT_LWFPGASLVS_OFST + FPGA_BRIDGE_BASE) & (unsigned long)(HW_REGS_MASK));

		const int fpga_bridge_mask = (1 << (FPGA_BRIDGE_DATA_WIDTH)) - 1;

		*(uint32_t *)h2p_lw_fpga_bridge_addr = data & fpga_bridge_mask;
	#else
			printf("FPGA Bridge not found\n");
	#endif
}

void print_key(const u8 key[]) {
	printf("Key: ");
	#if !REVERSE
		for (int i=0; i<MAXKEYSIZEB; ++i) {
			printf("%02x", key[i]);
		}
	#else
		for (int i=MAXKEYSIZEB; i; --i) {
			printf("%02x", key[i-1]);
		}
	#endif
	printf("\n");
}

void print_iv(const u8 iv[]) {
	printf("IV: ");
	#if !REVERSE
		for (int i=0; i<MAXIVSIZEB; ++i) {
			printf("%02x", iv[i]);
		}
	#else
		for (int i=MAXIVSIZEB; i; --i) {
			printf("%02x", iv[i-1]);
		}
	#endif
	printf("\n");
}

int test_trivium(void) {
    #define LEN 32
    ECRYPT_ctx ctx;
    const u8 key[MAXKEYSIZEB] = "Test key01";
    const u8 iv[MAXIVSIZEB] = "So Random\0";
    const u8 input[LEN] = "#include \"ecrypt-sync.h\"\r\n#inclu";
    u8 output[LEN];
    u8 second[LEN];
    int passed = 1;
    int i;

    if (USE_FPGA == 1) {
		int fd;
		if (( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) {
			printf( "ERROR: could not open \"/dev/mem\"...\n" );
		}

		// Map hardware registers into memory
		void *virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, HW_REGS_BASE );

		if (virtual_base == MAP_FAILED) {
			printf("ERROR: mmap() failed...\n");
			close(fd);
		}

		u32 temp;

		// send reset signal
		send_to_FPGA(0x00000000, virtual_base);

		// send Key A
		temp = key[0];
		temp += (((u32)key[1]) << 8);
		temp += (((u32)key[2]) << 16);
		temp &= ~(0xFFC00000);
		temp |= 0x00800000;
		send_to_FPGA(temp, virtual_base);

		// send Key B
		temp = (((u32)key[2]) >> 4);
		temp += (((u32)key[3]) << 4);
		temp += (((u32)key[4]) << 12);
		temp |= 0x00900000;
		send_to_FPGA(temp, virtual_base);

		// send Key C
		temp = key[5];
		temp += (((u32)key[6]) << 8);
		temp += (((u32)key[7]) << 16);
		temp &= ~(0xFFC00000);
		temp |= 0x00A00000;
		send_to_FPGA(temp, virtual_base);

		// send Key D
		temp = (((u32)key[7]) >> 4);
		temp += (((u32)key[8]) << 4);
		temp += (((u32)key[9]) << 12);
		temp |= 0x00B00000;
		send_to_FPGA(temp, virtual_base);

		// send IV A
		temp = iv[0];
		temp += (((u32)iv[1]) << 8);
		temp += (((u32)iv[2]) << 16);
		temp &= ~(0xFFC00000);
		temp |= 0x00C00000;
		send_to_FPGA(temp, virtual_base);

		// send IV B
		temp = (((u32)iv[2]) >> 4);
		temp += (((u32)iv[3]) << 4);
		temp += (((u32)iv[4]) << 12);
		temp |= 0x00D00000;
		send_to_FPGA(temp, virtual_base);

		// send IV C
		temp = iv[5];
		temp += (((u32)iv[6]) << 8);
		temp += (((u32)iv[7]) << 16);
		temp &= ~(0xFFC00000);
		temp |= 0x00E00000;
		send_to_FPGA(temp, virtual_base);

		// send IV D
		temp = (((u32)iv[7]) >> 4);
		temp += (((u32)iv[8]) << 4);
		temp += (((u32)iv[9]) << 12);
		temp |= 0x00F00000;
		send_to_FPGA(temp, virtual_base);

		// send Start signal
		send_to_FPGA(0x00100000, virtual_base);

		//wait 1 second
		usleep(1000);

		int i, j;

		// send data
		for (i = 0; i < LEN; i++) {
			for (j = 0; j < 8; j++) {
				temp = (((u32)input[i]) << j);
				temp |= 0x00700000;
				send_to_FPGA(temp, virtual_base);
			}
		}
		
		if (munmap(virtual_base, HW_REGS_SPAN) != 0) {
			printf("ERROR: munmap() failed...\n");
			close(fd);
		}

		close(fd);	

	} else {	
		ECRYPT_init();

		ECRYPT_ivsetup(&ctx, iv);
		ECRYPT_encrypt_blocks(&ctx, input, output, (LEN/ECRYPT_BLOCKLENGTH));

		ECRYPT_ivsetup(&ctx, iv);
		ECRYPT_encrypt_blocks(&ctx, output, second, (LEN/ECRYPT_BLOCKLENGTH));
	}

    // Output results of encryption/decryption and test
	printf("Testing...\n");
	print_key(key);
	print_iv(iv);
	printf("\n");

	for (i=0; i<LEN; ++i) {
		passed &= (second[i] == input[i]);
		printf("%02x", input[i]);
	}
	printf("\n\t\t\tv v v v v v v v\n");
	for (i=0; i<LEN; ++i) {
		printf("%02x", output[i]);
	}
	printf("\n\t\t\tv v v v v v v v\n");
	for (i=0; i<LEN; ++i) {
		printf("%02x", second[i]);
	}
	printf("\n");

    // Display test result
	if (passed) {
		printf("PASSED\n");
	} else {
		printf("FAILED\n");
	}

    return passed;
}

int trivium_file(char *encrypted, char *decrypted, const u8 key[], const u8 iv[]) {
    //Note that the cipher is reversible, so passing in a decrypted file as the first argument will
    //  result in an encrypted output

    ECRYPT_ctx ctx;

    // Initialise cipher
    ECRYPT_init();
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

int fd;
int trivium_setup(void) {
    // Hint: This would be a good place to setup your memory mappings
	#ifdef ARM
	if((fd = open( "/dev/mem", ( O_RDWR | O_SYNC ))) == -1) {
		printf("ERROR: could not open \"/dev/mem\"...\n");
		return(1);
	}
	#endif

    return 0;
}

int trivium_finish(void) {
	// And now clean up all the memory mappings
    close(fd);
    return 0;
}

int trivium_decrypt_file(char* infile, char* outfile, const u8 key[], const u8 iv[]) {
    trivium_setup();

    // Execute code
    int ret = trivium_file(infile, outfile, key, iv);

    trivium_finish();
    return ret;
}

int trivium_test_cipher(void) {
    trivium_setup();
	
	// Test for input reversible cipher
    int ret = test_trivium();

    trivium_finish();
    return ret;
}
