#include <freespace/freespace.h>
#include <freespace/freespace_util.h>
#include <freespace/freespace_printers.h>
#include "appControlHandler.h"


/**
 * main
 * This example uses the synchronous API to
 *  - find a device
 *  - open the device found
 *  - configure the device to output motion
 *  - read motion messages sent by the device
 * This example assume the device is already connected.
 */
int main(int argc, char* argv[]) {
    struct freespace_message message;
    FreespaceDeviceId device;
    int numIds; // The number of device ID found
    int rc; // Return code
    struct MultiAxisSensor angVel;

    // Flag to indicate that the application should quit
    // Set by the control signal handler
    int quit = 0;

    printVersionInfo(argv[0]);

    addControlHandler(&quit);

    // Initialize the freespace library
    rc = freespace_init();
    if (rc != FREESPACE_SUCCESS) {
        printf("Initialization error. rc=%d\n", rc);
	    return 1;
    }

    printf("Scanning for Freespace devices...\n");
     // Get the ID of the first device in the list of availble devices
    rc = freespace_getDeviceList(&device, 1, &numIds);
    if (numIds == 0) {
        printf("Didn't find any devices.\n");
        return 1;
    }

    printf("Found a device. Trying to open it...\n");
    // Prepare to communicate with the device found above
    rc = freespace_openDevice(device);
    if (rc != FREESPACE_SUCCESS) {
        printf("Error opening device: %d\n", rc);
        return 1;
    }

    // Display the device information.
    printDeviceInfo(device);

    // Make sure any old messages are cleared out of the system
    rc = freespace_flush(device);
    if (rc != FREESPACE_SUCCESS) {
        printf("Error flushing device: %d\n", rc);
        return 1;
    }

    // Configure the device for motion outputs
    printf("Sending message to enable motion data.\n");
    memset(&message, 0, sizeof(message)); // Make sure all the message fields are initialized to 0.

    message.messageType = FREESPACE_MESSAGE_DATAMODECONTROLV2REQUEST;
    message.dataModeControlV2Request.packetSelect = 8;  // MotionEngine Outout
    message.dataModeControlV2Request.mode = 0;          // Set full motion
    message.dataModeControlV2Request.formatSelect = 0;  // MEOut format 0
    message.dataModeControlV2Request.ff0 = 1;           // Pointer fields
    message.dataModeControlV2Request.ff3 = 1;           // Angular velocity fields
    
    rc = freespace_sendMessage(device, &message);
    if (rc != FREESPACE_SUCCESS) {
        printf("Could not send message: %d.\n", rc);
    }
        
    // A loop to read messages
    printf("Listening for messages.\n");
    while (!quit) {
        rc = freespace_readMessage(device, &message, 100);
        if (rc == FREESPACE_ERROR_TIMEOUT ||
            rc == FREESPACE_ERROR_INTERRUPTED) {
            // Both timeout and interrupted are ok.
            // Timeout happens if there aren't any events for a second.
            // Interrupted happens if you type CTRL-C or if you
            // type CTRL-Z and background the app on Linux.
            continue;
        }
        if (rc != FREESPACE_SUCCESS) {
            printf("Error reading: %d. Quitting...\n", rc);
            break;
        }

        // freespace_printMessage(stdout, &message); // This just prints the basic message fields
        if (message.messageType == FREESPACE_MESSAGE_MOTIONENGINEOUTPUT) {
            rc = freespace_util_getAngularVelocity(&message.motionEngineOutput, &angVel);
            if (rc == 0) {
                printf ("X: % 6.2f, Y: % 6.2f, Z: % 6.2f\n", angVel.x, angVel.y, angVel.z);
            }
        }
    }

    // Close communications with the device
    printf("Cleaning up...\n");
    freespace_closeDevice(device);
    /** --- END EXAMPLE FINALIZATION OF DEVICE --- **/

    // Cleanup the library
    freespace_exit();

    return 0;
}
