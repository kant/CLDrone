/****************************************************************************
 *
 *   Copyright (c) 2015 Crossline Drone Project Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name CLDrone or Crossline Drone nor the names of its c
 *    ontributors may be used to endorse or promote products derived from 
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#include "body_axis_velocity_controller_node.h"
#include "tf/tf.h"
#include <math.h>

BodyAxisVelocityController::BodyAxisVelocityController(std::string prefix)
{
    
    localVelocityPublisher = nodeHandle.advertise<geometry_msgs::TwistStamped>("/mavros/setpoint_velocity/cmd_vel",10);
    localPositionSubscriber = nodeHandle.subscribe("/mavros/local_position/pose", 10, &BodyAxisVelocityController::localPositionReceived,this);
    bodyAxisVelocitySubscriber = nodeHandle.subscribe("/CLDrone/body_axis_velocity/cmd_vel",10, &BodyAxisVelocityController::bodyAxisVelocityReceived,this);

}

void BodyAxisVelocityController::localPositionReceived(const geometry_msgs::PoseStampedConstPtr& msg)
{
   currentPosition = *msg;
}

void BodyAxisVelocityController::bodyAxisVelocityReceived(const geometry_msgs::TwistStampedConstPtr& msg)
{

	geometry_msgs::TwistStamped bodyAxisVelocity = *msg;

	geometry_msgs::TwistStamped localVelocityTransfered = transferBodyAxisToLocal(bodyAxisVelocity,currentPosition);

	localVelocityTransfered.header.stamp = ros::Time::now();

	localVelocityPublisher.publish(localVelocityTransfered);
}

geometry_msgs::TwistStamped BodyAxisVelocityController::transferBodyAxisToLocal(geometry_msgs::TwistStamped bodyAxisVelocity,geometry_msgs::PoseStamped currentPosition)
{
	geometry_msgs::TwistStamped transferedLocalVelocity;

	tf::Pose currentPose;
	tf::poseMsgToTF(currentPosition.pose,currentPose);
	double yawAngle = tf::getYaw(currentPose.getRotation());
    
    transferedLocalVelocity.twist.linear.x = bodyAxisVelocity.twist.linear.x * cos(yawAngle) - bodyAxisVelocity.twist.linear.y * sin(yawAngle);
    transferedLocalVelocity.twist.linear.y = bodyAxisVelocity.twist.linear.x * sin(yawAngle) + bodyAxisVelocity.twist.linear.y * cos(yawAngle);
    transferedLocalVelocity.twist.linear.z = bodyAxisVelocity.twist.linear.z;
    transferedLocalVelocity.twist.angular.z = bodyAxisVelocity.twist.angular.z;
    return transferedLocalVelocity;

}


int main(int argc, char **argv)
{
	ros::init(argc, argv, "body_axis_velocity_controller_node");
    ros::NodeHandle nodeHandle;
    std::string prefix;
    nodeHandle.param<std::string>("prefix",prefix,std::string("ardrone"));
    BodyAxisVelocityController controller(prefix);

    ros::spin();

    return 0;

}