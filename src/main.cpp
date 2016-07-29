#include <stdio.h>
#include <iostream>
#include <math.h>
#include <vector>
#include <algorithm>

#include <ros/ros.h>

#include <freespace/freespace.h>
#include <freespace/freespace_util.h>
#include <freespace/freespace_printers.h>

//#include "appControlHandler.h"

// Message includes
#include <sensor_msgs/Imu.h>
#include <geometry_msgs/Vector3.h>
#include <geometry_msgs/Quaternion.h>

using namespace std;

class FreespaceNode {
	ros::Publisher imu_pub;
	ros::Publisher imu_nogravity_pub;
	int rc;
    struct freespace_message message;
    FreespaceDeviceId device;
    int numIds; // The number of device ID found
    struct MultiAxisSensor angVel;
	struct MultiAxisSensor acceleration;
	struct MultiAxisSensor accNoGravity;
	int quit;
	
	public:
		FreespaceNode(int argc, char **argv);
};

FreespaceNode::FreespaceNode(int argc, char **argv) : quit(0) {
	ros::init(argc, argv, "freespace_ros");
	ros::NodeHandle n;
	imu_pub = n.advertise<sensor_msgs::Imu>("freespace/imu", 1000);
	imu_nogravity_pub = n.advertise<sensor_msgs::Imu>("freespace/imu_no_gravity", 1000);
	//ros::Rate loop_rate(10);
    // Initialize the freespace library
    rc = freespace_init();
    if (rc != FREESPACE_SUCCESS) {
        printf("Initialization error. rc=%d\n", rc);
    }

    printf("Scanning for Freespace devices...\n");
     // Get the ID of the first device in the list of availble devices
    rc = freespace_getDeviceList(&device, 1, &numIds);
    if (numIds == 0) {
        printf("Didn't find any devices.\n");
    }

    printf("Found a device. Trying to open it...\n");
    // Prepare to communicate with the device found above
    rc = freespace_openDevice(device);
    if (rc != FREESPACE_SUCCESS) {
        printf("Error opening device: %d\n", rc);
    }

    // Display the device information.
    //printDeviceInfo(device);

    // Make sure any old messages are cleared out of the system
    rc = freespace_flush(device);
    if (rc != FREESPACE_SUCCESS) {
        printf("Error flushing device: %d\n", rc);
    }

    // Configure the device for motion outputs
    printf("Sending message to enable motion data.\n");
    memset(&message, 0, sizeof(message)); // Make sure all the message fields are initialized to 0.

    message.messageType = FREESPACE_MESSAGE_DATAMODECONTROLV2REQUEST;
    message.dataModeControlV2Request.packetSelect = 8;  // MotionEngine Outout
    message.dataModeControlV2Request.mode = 0;          // Set full motion
    message.dataModeControlV2Request.formatSelect = 0;  // MEOut format 0
    message.dataModeControlV2Request.ff0 = 1;           // Pointer fields
    message.dataModeControlV2Request.ff1 = 1;           // Linear acceleration field
    message.dataModeControlV2Request.ff2 = 1;           // Linear acceleration field without gravity
    message.dataModeControlV2Request.ff3 = 1;           // Angular velocity fields
    
    rc = freespace_sendMessage(device, &message);
    if (rc != FREESPACE_SUCCESS) {
        printf("Could not send message: %d.\n", rc);
    }
        
    // A loop to read messages
    printf("Listening for messages.\n");
    while (!quit&&ros::ok()) {
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
            printf("Error reading: %d.", rc);
        }

        if (message.messageType == FREESPACE_MESSAGE_MOTIONENGINEOUTPUT) {
            rc = freespace_util_getAngularVelocity(&message.motionEngineOutput, &angVel);
			int rc2 = freespace_util_getAcceleration(&(message.motionEngineOutput), &acceleration);
			int rc3 = freespace_util_getAccNoGravity(&(message.motionEngineOutput), &accNoGravity);
            if (rc == 0 && rc2 == 0 && rc3 == 0) {
				sensor_msgs::Imu message;
				message.angular_velocity.x = angVel.x;
				message.angular_velocity.y = angVel.y;
				message.angular_velocity.z = angVel.z;
				message.linear_acceleration.x = acceleration.x;
				message.linear_acceleration.y = acceleration.y;
				message.linear_acceleration.z = acceleration.z;
				message.header.stamp = ros::Time::now();
				sensor_msgs::Imu messageNoGravity;
				messageNoGravity.angular_velocity.x = angVel.x;
				messageNoGravity.angular_velocity.y = angVel.y;
				messageNoGravity.angular_velocity.z = angVel.z;
				messageNoGravity.linear_acceleration.x = accNoGravity.x;
				messageNoGravity.linear_acceleration.y = accNoGravity.y;
				messageNoGravity.linear_acceleration.z = accNoGravity.z;
				messageNoGravity.header.stamp = ros::Time::now();

				//message.orientation_covariance
				this->imu_nogravity_pub.publish(messageNoGravity);
				this->imu_pub.publish(message);
				ros::spinOnce();
                //printf ("X: % 6.2f, Y: % 6.2f, Z: % 6.2f\n", angVel.x, angVel.y, angVel.z);
            }
        }
    }

    // Close communications with the device
    printf("Cleaning up...\n");
    freespace_closeDevice(device);
    /** --- END EXAMPLE FINALIZATION OF DEVICE --- **/

    // Cleanup the library
    freespace_exit();


}

int main(int argc, char **argv) {
	FreespaceNode n(argc, argv);
	return 0;
}
