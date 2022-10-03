#include "smarty_arm_interface/smarty_arm_interface.h"

using namespace std;

/* RDDNode constructor */
SMARTY_ARM_Node::SMARTY_ARM_Node(ros::NodeHandle &node, Arm *armptr, std::string type): node_type(type) {
    nh_ = node;
    arm = armptr;

    if (node_type == "r") {
        smarty_arm_packet_sub = nh_.subscribe("/pti_right_output", 1, &SMARTY_ARM_Node::ptipacket_callback, this, ros::TransportHints().udp());
        smarty_arm_packet_pub = nh_.advertise<smarty_arm_interface::PTIPacket>("/right_smarty_arm_output", 1);
    }
    else if (node_type == "l") {
        smarty_arm_packet_sub = nh_.subscribe("/pti_left_output", 1, &SMARTY_ARM_Node::ptipacket_callback, this, ros::TransportHints().udp());
        smarty_arm_packet_pub = nh_.advertise<smarty_arm_interface::PTIPacket>("/left_smarty_arm_output", 1);
    }

    ROS_INFO("Node initialized");
}

SMARTY_ARM_Node::~SMARTY_ARM_Node() = default;


/* Publish arm packet through ROS */
void SMARTY_ARM_Node::publish_ptipacket() {

    smarty_arm_interface::PTIPacket packet_msg;

    packet_msg.wave.resize(3);
    packet_msg.test.resize(3);

    mutex_lock(&arm->mutex);
    for (int i = 0; i < 3; i ++) {
        packet_msg.wave[i] = arm->ptiPacket[i].wave_out;
        packet_msg.test[i] = arm->motor[i].motorIn.act_tau;
    }
    packet_msg.position.x = arm->ee[0].pos;
    packet_msg.position.y = arm->ee[1].pos;
    packet_msg.position.z = arm->ee[2].pos;
    packet_msg.angle.x = arm->ee[3].pos;
    packet_msg.angle.y = arm->ee[4].pos;
    packet_msg.angle.z = arm->ee[5].pos;
    packet_msg.twist.linear.x = arm->ee[0].vel;
    packet_msg.twist.linear.y = arm->ee[1].vel;
    packet_msg.twist.linear.z = arm->ee[2].vel;
    packet_msg.twist.angular.x = arm->ee[3].vel;
    packet_msg.twist.angular.y = arm->ee[4].vel;
    packet_msg.twist.angular.z = arm->ee[5].vel;
    mutex_unlock(&arm->mutex);

    // if (smarty_arm_packet_pub.getNumSubscribers() == 0) {
    //     if (node_type == "master") {
    //         ROS_ERROR_STREAM("Connection lost, trying to reconnect...");
    //         smarty_arm_packet_pub.shutdown();
    //         smarty_arm_packet_pub = nh_.advertise<smarty_arm_interface::PTIPacket>("/smarty_arm_output", 1);
    //     }
    // }

    smarty_arm_packet_pub.publish(packet_msg);

    // ROS_INFO("Published ARMRead message");
}

/* Subscriber callback */
/* Comment out callback for remote test */
void SMARTY_ARM_Node::ptipacket_callback(const smarty_arm_interface::PTIPacket::ConstPtr &packet_msg) {

    mutex_lock(&arm->mutex);
    for (int i = 0; i < 3; i++) {
        arm->ptiPacket[i].wave_in = packet_msg->wave[i];
    }

    mutex_unlock(&arm->mutex);
    ROS_INFO_THROTTLE(1, "Write into smarty arm memory");

}


/* Run loop */
void SMARTY_ARM_Node::run() {
    ros::Rate loop_rate(2000);
    while (ros::ok()) {
	/* Publisher (wrap) */
    publish_ptipacket();
	/* Subscriber callback loop */
	ros::spinOnce();
	loop_rate.sleep();
    }
}

int main(int argc, char** argv) {

    /* Instanciate input-output data varibles */
    Arm *arm;

    /* Map data structs to shared memory */
    /* Open and obtain shared memory pointers for master-input data */
    arm = initArm(char(*argv[1]));
    if (arm == nullptr) {
        fprintf(stderr, "Init arm failed.\n");
        printf("shm_open error, errno(%d): %s\n", errno, strerror(errno));
        exit(1);
    }

    /* Initialise ROS node */
    ros::init(argc, argv, "smarty_arm_interface");
    printf("Launch ros interface\n");

    ros::NodeHandle node("~");

    // initialize a node with "master" or "slave" setting
    SMARTY_ARM_Node smarty_arm(node, arm, std::string(argv[1]));
    ROS_INFO("Node starts running");
    smarty_arm.run();
}
