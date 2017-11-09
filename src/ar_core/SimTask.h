//
// Created by nima on 25/05/17.
//

#ifndef ATAR_SIMTASK_H
#define ATAR_SIMTASK_H

#include <ros/ros.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vector>
#include <kdl/frames.hpp>
#include <btBulletDynamicsCommon.h>
#include "Rendering.h"


//note about vtkSmartPointer:
// One way to create a VTK object is
//      vtkObject* MyObject = vtkObject::New();
// This method, however, can (and likely will) lead to memory management
// issues at some point or another. You must manually delete the object
//      MyObject->Delete();
// Instead if vtkSmartPointer is used the memory will be released when the
//      object having the pointer goes out of scope:
// vtkSmartPointer<vtkObject> MyObject = vtkSmartPointer<vtkObject>::New();

extern std::string                      MESH_DIRECTORY;


class SimTask{
public:
    SimTask(ros::NodeHandlePtr n, const double haptic_loop_rate);

    virtual ~SimTask() {};

    // The main loop. Updates physics, graphics and task logic
    virtual void StepWorld();

    // The main loop. Updates physics, graphics and task logic
    virtual void TaskLoop() =0;

    // This is the function that is handled by the haptics thread.
    virtual void HapticsThread() = 0;

    // minor reset
    virtual void ResetCurrentAcquisition(){};

    // full reset ;
    virtual void ResetTask() {};

    virtual void StartManipulatorToWorldFrameCalibration(const uint arm_id){};

private:
    void InitBullet();

    // steps the physics simulation
    virtual void StepPhysics();

protected:

    ros::NodeHandlePtr                      nh;
    ros::Time                               time_last;

    Rendering *                             graphics;

    double                                  haptic_loop_rate;
    std::vector<vtkSmartPointer<vtkProp>>   graphics_actors;
    btDiscreteDynamicsWorld*                dynamics_world;

    //make sure to re-use collision shapes among rigid bodies whenever possible!
    btSequentialImpulseConstraintSolver*    solver;
    btBroadphaseInterface*                  overlappingPairCache;
    btCollisionDispatcher*                  dispatcher;
    btDefaultCollisionConfiguration*        collisionConfiguration;
};



#endif //ATAR_SIMTASK_H
