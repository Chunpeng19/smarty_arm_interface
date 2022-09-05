#ifndef SMARTY_ARM_NODE_H
#define SMARTY_ARM_NODE_H

/* C++ headers */
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <vector>
#include <algorithm>

/* ROS headers */
#include <ros/ros.h>
#include <std_msgs/Float64.h>
#include <std_srvs/Empty.h>
#include "smarty_arm_interface/EEPacket.h"

/* C headers */
extern "C" {
#include "shm.h"
};

class SMARTY_ARM_Node {
 public:
    explicit SMARTY_ARM_Node(ros::NodeHandle& node, Rdda *rdda, std::string type);

    ~SMARTY_ARM_Node();

    void run();

 private:
    ros::NodeHandle nh_;

    ros::Subscriber smarty_arm_packet_sub;

    ros::Publisher smarty_arm_packet_pub; 

    Rdda *rdda;

    std::string node_type;

    void publish_eepacket();
    void eepacket_callback(const smarty_arm_interface::EEPacket::ConstPtr &msg);

    bool initSlave(std_srvs::Empty::Request &req, std_srvs::Empty::Response &res);
};

#endif /* SMARTY_ARM_NODE */
