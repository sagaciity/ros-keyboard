#include <ros/ros.h>
#include <std_msgs/Float32.h>

#include <iostream>

#include "keyboard.h"
using namespace std;

int main(int argc, char **argv) {
    ros::init(argc, argv, "keyboard");
    ros::NodeHandle n("~");

    // Create a publisher for the key events
    ros::Publisher pub_steering_command = n.advertise<std_msgs::Float32>("left_right", 10);

    bool allow_repeat = false;
    int repeat_delay, repeat_interval;

    // Takes these values from params of launch files if necessary
    n.param<bool>("allow_repeat", allow_repeat, false);
    n.param<int>("repeat_delay", repeat_delay, SDL_DEFAULT_REPEAT_DELAY);
    n.param<int>("repeat_interval", repeat_interval, SDL_DEFAULT_REPEAT_INTERVAL);

    if (!allow_repeat)
        repeat_delay = 0;  // disable if we do not allow repeat

    keyboard::Keyboard kbd(repeat_delay, repeat_interval);
    ros::Rate r(50);

    std_msgs::Float32 steering_command;
    double steer_ang_prev = 0, steer_ang, inc_ang = 2.0;

    keyboard::Key k;
    bool pressed, new_event;
    while (ros::ok() && kbd.get_key(new_event, pressed, k.code, k.modifiers)) {
        if (new_event || pressed) {
            k.header.stamp = ros::Time::now();
            if (pressed) {
                // pub_down.publish(k);
                if (new_event && k.code == 119)
                    inc_ang = std::max(inc_ang + 0.5, 10.0);
                else if (new_event && k.code == 115)
                    inc_ang = std::min(inc_ang - 0.5, 2.0);
                else if (k.code == 275) {
                    steer_ang = std::min(steer_ang_prev + inc_ang, 500.0);
                    steer_ang_prev = steer_ang;
                } else if (k.code == 276) {
                    steer_ang = std::max(steer_ang_prev - inc_ang, -500.0);
                    steer_ang_prev = steer_ang;
                }
            }
            steering_command.data = steer_ang_prev;
            pub_steering_command.publish(steering_command);
        }
        ros::spinOnce();
        r.sleep();
    }

    ros::waitForShutdown();
}
