// intel_acpi_arc
// Author: Rabit <home@rabits.org>
//
// A simple way to allow user to control the Arc videocard power with setgid bit.
// It this simple and restricted just to 3 static methods to lower the danger of setgid method.
//
// HowTo:
// 1. Check your ACPI tables: (acpidump/acpiextract/iasl):
//    * Find the path to your discrete videocard device
//    * Find the status, poweron, poweroff methods
//    * Put path and method names to the corresponding constants
// 2. Compile the binary: $ gcc -O2 -s intel_arc_acpi.c -o intel_arc_acpi
// 3. Move binary intel_arc_acpi to /usr/local/bin/ directory
// 4. Set executable and setgit bits: $ sudo chmod a+x,u+s /usr/local/bin/intel_arc_acpi
// 5. Use as: $ intel_arc_acpi <status|on|off>

#include <stdio.h>
#include <string.h>

// This ACPI path works for Lenovo Yoga i7 with Intel Arc A370M 4GB GDDR6
const char *DEV_PATH = "\\_SB.PC00.PEG2";

// Methods of the device.
//const char *DEV_ON = "PON";
//const char *DEV_OFF = "POFF";
const char *DEV_ON = "PXP._ON";
const char *DEV_OFF = "PXP._OFF";
const char *DEV_STATUS = "PSTA";

int main(int argc, const char *argv[]) {
    FILE *fp;
    if( argc != 2 ) {
        printf("Usage: $ %s <status|on|off>\n", argv[0]);
        return 1;
    }

    if( strcmp(argv[1], "off") == 0 ) {
        // The card is going to be disabled - make sure it's unbinded from the driver
        // Disconnect the pcie device and remove it, it will automatically be unbinded from the driver
        fp = fopen("/sys/class/drm/card1/device/remove", "w");
        if( fp == NULL ) {
            printf("INFO: Unable to open '/sys/class/drm/card1/device/remove' interface for write.\n");
        } else {
            printf("INFO: Disconnecting device '/sys/class/drm/card1'...\n");
            fprintf(fp, "1\n");
            fflush(fp);
            fclose(fp);
        }
    }

    fp = fopen("/proc/acpi/call", "w");
    if( fp == NULL ) {
        printf("ERROR: Unable to open '/proc/acpi/call' interface for write. Make sure acpi_call module is loaded and you have enough permissions.\n");
        return 2;
    }

    // Write command to the acpi call interface
    if( strcmp(argv[1], "on") == 0 ) {
        // on
        fprintf(fp, "%s.%s\n", DEV_PATH, DEV_ON);
    } else if( strcmp(argv[1], "off") == 0 ) {
        // off
        fprintf(fp, "%s.%s\n", DEV_PATH, DEV_OFF);
    } else if( strcmp(argv[1], "status") == 0 ) {
        // status
        fprintf(fp, "%s.%s\n", DEV_PATH, DEV_STATUS);
    } else {
        printf("ERROR: Unknown argument provided.\n");
        printf("Usage: $ %s <status|on|off>\n", argv[0]);
        fclose(fp);
        return 3;
    }

    // Flush buffers to send data
    fflush(fp);
    fclose(fp);

    // Get the result and print it to stdout
    fp = fopen("/proc/acpi/call", "r");
    if( fp == NULL ) {
        printf("ERROR: Unable to open '/proc/acpi/call' interface for read. Make sure acpi_call module is loaded and you have enough permissions.\n");
        return 4;
    }

    char c = fgetc(fp);
    while( c != EOF ) {
        printf("%c", c);
        c = fgetc(fp);
    }
    printf("\n");

    fclose(fp);

    if( strcmp(argv[1], "on") == 0 ) {
        // The card was enabled - make sure it's binded to the driver
        // Rescan will not just check if all the pci devices are here, but also will notify the
        // i915 driver to bind itself
        fp = fopen("/sys/bus/pci/rescan", "w");
        if( fp == NULL ) {
            printf("ERROR: Unable to open '/sys/bus/pci/rescan' interface for write...\n");
            return 5;
        }
        printf("INFO: Rescaning the PCI bus to connect '/sys/class/drm/card1' back...\n");
        fprintf(fp, "1\n");
        fflush(fp);
        fclose(fp);
    }

    return 0;
}
