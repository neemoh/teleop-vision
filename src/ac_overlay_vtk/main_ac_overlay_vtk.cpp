//
// Created by nima on 4/13/17.
//
#include "ros/ros.h"
#include "geometry_msgs/Pose.h"
#include "geometry_msgs/PoseStamped.h"
#include "utils/Conversions.hpp"
#include "OverlayROSConfig.h"
#include <std_msgs/Float32.h>
#include "CalibratedCamera.h"
#include "BuzzWireTask.h"



enum class Tasks {None, CricleAC, MultiplePaths};

int main(int argc, char **argv)
{
    ros::init(argc, argv, "ac_overlay");
    OverlayROSConfig rc (ros::this_node::getName(),720, 576);

    // frequency of the generated images is based on the received images
    // loop_rate is the frequency of spinning and checking for new messages
    // and evaluating and publishing the desired pose according to the active
    // constraint
    ros::Rate loop_rate(rc.desired_pose_update_freq);

    // Create the window for the video feed
    std::string cv_window_name = "Augmented Stereo images";
    cvNamedWindow(cv_window_name.c_str() ,CV_WINDOW_NORMAL);

    std::stringstream ui_instructions;
    ui_instructions <<"Test. " ;

    cv::Mat cam_images[2];
    rc.LockAndGetImages(ros::Duration(1), cam_images);

    Rendering graphics;
    graphics.SetWorldToCameraTransform(rc.cam_rvec, rc.cam_tvec);
    cv::Matx33d cam_matrices[2];
    cam_matrices[0]= rc.cam_intrinsics[0].camMatrix;
    cam_matrices[1]= rc.cam_intrinsics[1].camMatrix;
    graphics.SetCameraIntrinsics(cam_matrices);
    graphics.ConfigureBackgroundImage(cam_images);
    graphics.SetEnableBackgroundImage(true);
    graphics.Render();

    // create the task
    double ring_radius = 0.004;
    BuzzWireTask buzztask(ring_radius, rc.show_reference_frames);

    // Add the task actors to the graphics
    graphics.AddActorsToScene(buzztask.GetActors());

    KDL::Frame pose_desired_tool[2];
    cv::Mat augmented_stereo_image;

    while (ros::ok())
    {

        // --------------------------------------------------------------------------------------
        // keyboard commands

        char key = (char)cv::waitKey(1);
        if (key == 27) // Esc
            ros::shutdown();
        else if (key == 'f')  //full screen
            VisualUtils::SwitchFullScreen(cv_window_name);
        else if (key == '1'){
            ROS_INFO("Task 1 Selected");
        }

        buzztask.SetCurrentToolPose(rc.pose_current_tool[0]);

        if(rc.GetNewImages(cam_images)) {

            // Time performance debug
            //ros::Time start =ros::Time::now();

            // print instructions
            for (int i = 0; i < 2; ++i)
                cv::putText(cam_images[i], ui_instructions.str(),
                            cv::Point(50, 50), 0, 0.8, cv::Scalar(20, 150, 20), 2);

            // update the moving actors
            buzztask.UpdateActors();

            // update the camera images and view angle (in case window changes size)
            graphics.UpdateBackgroundImage(cam_images);
            graphics.UpdateViewAngleForActualWindowSize();

            // Render!
            graphics.Render();

            // Copy the rendered image to memory, show it and/or publish it.
            graphics.GetRenderedImage(augmented_stereo_image);
            cv::imshow(cv_window_name, augmented_stereo_image);
            rc.publisher_stereo_overlayed.publish(
                    cv_bridge::CvImage(std_msgs::Header(),
                                       "bgr8", augmented_stereo_image).toImageMsg());
            // updating the desired pose
            pose_desired_tool[0] = buzztask.GetDesiredToolPose();

            if(buzztask.IsACParamChanged())
                rc.PublishACtiveConstraintParameters(0, buzztask.GetACParameters());
            // std::cout <<  "it took: " << (ros::Time::now() - start).toNSec() /1000000 << std::endl;

        } // if new image

        // the graphics take already about 30 to 40 milliseconds so publishing
        // the pose really doesn't happen faster here
        rc.PublishDesiredPose(pose_desired_tool);


        ros::spinOnce();
        loop_rate.sleep();
    }

    return 0;
}


