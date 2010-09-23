/*
 * Copyright (c) 2008, Willow Garage, Inc.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Willow Garage, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <gtest/gtest.h>
#include <tf2/tf2_core.h>
#include <sys/time.h>
#include <ros/ros.h>
#include "LinearMath/btVector3.h"

void seed_rand()
{
  //Seed random number generator with current microseond count
  timeval temp_time_struct;
  gettimeofday(&temp_time_struct,NULL);
  srand(temp_time_struct.tv_usec);
};

void generate_rand_vectors(double scale, uint64_t runs, std::vector<double>& xvalues, std::vector<double>& yvalues, std::vector<double>&zvalues)
{
  seed_rand();
  for ( uint64_t i = 0; i < runs ; i++ )
  {
    xvalues[i] = 1.0 * ((double) rand() - (double)RAND_MAX /2.0) /(double)RAND_MAX;
    yvalues[i] = 1.0 * ((double) rand() - (double)RAND_MAX /2.0) /(double)RAND_MAX;
    zvalues[i] = 1.0 * ((double) rand() - (double)RAND_MAX /2.0) /(double)RAND_MAX;
  }
}


using namespace tf;

TEST(tf, setTransformNoInsertOnSelfTransform)
{
  tf::Transformer mTR(true);
  StampedTransform tranStamped(btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(10.0), "same_frame", "same_frame");
  EXPECT_FALSE(mTR.setTransform(tranStamped));
}

TEST(tf, setTransformNoInsertWithNan)
{
  tf::Transformer mTR(true);
  StampedTransform tranStamped(btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(10.0), "same_frame", "other_frame");
  EXPECT_TRUE(mTR.setTransform(tranStamped));

  tranStamped.setOrigin(tf::Point(1.0,1.0,0.0/0.0));
  EXPECT_TRUE(std::isnan(tranStamped.getOrigin().z()));
  EXPECT_FALSE(mTR.setTransform(tranStamped));

}

TEST(tf, setTransformNoInsertWithNoFrameID)
{
  tf::Transformer mTR(true);
  StampedTransform tranStamped(btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(10.0), "parent_frame", "");
  EXPECT_FALSE(mTR.setTransform(tranStamped));
}

TEST(tf, setTransformNoInsertWithNoParentID)
{
  tf::Transformer mTR(true);
  StampedTransform tranStamped(btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(10.0), "", "my_frame");
  EXPECT_FALSE(mTR.setTransform(tranStamped));
}


TEST(tf, ListOneInverse)
{
  unsigned int runs = 4;
  double epsilon = 1e-6;
  seed_rand();
  
  tf::Transformer mTR(true);
  std::vector<double> xvalues(runs), yvalues(runs), zvalues(runs);
  for ( uint64_t i = 0; i < runs ; i++ )
  {
    xvalues[i] = 10.0 * ((double) rand() - (double)RAND_MAX /2.0) /(double)RAND_MAX;
    yvalues[i] = 10.0 * ((double) rand() - (double)RAND_MAX /2.0) /(double)RAND_MAX;
    zvalues[i] = 10.0 * ((double) rand() - (double)RAND_MAX /2.0) /(double)RAND_MAX;

    StampedTransform tranStamped(btTransform(btQuaternion(0,0,0,1), btVector3(xvalues[i],yvalues[i],zvalues[i])), ros::Time().fromNSec(10 + i),  "my_parent", "child");
    mTR.setTransform(tranStamped);
  }

  //  std::cout << mTR.allFramesAsString() << std::endl;
  //  std::cout << mTR.chainAsString("child", 0, "my_parent2", 0, "my_parent2") << std::endl;

  for ( uint64_t i = 0; i < runs ; i++ )

  {
    Stamped<Pose> inpose (btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(10 + i), "child");

    try{
    Stamped<Pose> outpose;
    outpose.setIdentity(); //to make sure things are getting mutated
    mTR.transformPose("my_parent",inpose, outpose);
    EXPECT_NEAR(outpose.getOrigin().x(), xvalues[i], epsilon);
    EXPECT_NEAR(outpose.getOrigin().y(), yvalues[i], epsilon);
    EXPECT_NEAR(outpose.getOrigin().z(), zvalues[i], epsilon);
    }
    catch (tf::TransformException & ex)
    {
      std::cout << "TransformExcepion got through!!!!! " << ex.what() << std::endl;
      bool exception_improperly_thrown = true;
      EXPECT_FALSE(exception_improperly_thrown);
    }
  }
  
}

TEST(tf, ListTwoInverse)
{
  unsigned int runs = 4;
  double epsilon = 1e-6;
  seed_rand();
  
  tf::Transformer mTR(true);
  std::vector<double> xvalues(runs), yvalues(runs), zvalues(runs);
  for ( unsigned int i = 0; i < runs ; i++ )
  {
    xvalues[i] = 10.0 * ((double) rand() - (double)RAND_MAX /2.0) /(double)RAND_MAX;
    yvalues[i] = 10.0 * ((double) rand() - (double)RAND_MAX /2.0) /(double)RAND_MAX;
    zvalues[i] = 10.0 * ((double) rand() - (double)RAND_MAX /2.0) /(double)RAND_MAX;

    StampedTransform tranStamped(btTransform(btQuaternion(0,0,0,1), btVector3(xvalues[i],yvalues[i],zvalues[i])), ros::Time().fromNSec(10 + i),  "my_parent", "child");
    mTR.setTransform(tranStamped);
    StampedTransform tranStamped2(btTransform(btQuaternion(0,0,0,1), btVector3(xvalues[i],yvalues[i],zvalues[i])), ros::Time().fromNSec(10 + i),  "child", "grandchild");
    mTR.setTransform(tranStamped2);
  }

  //  std::cout << mTR.allFramesAsString() << std::endl;
  //  std::cout << mTR.chainAsString("child", 0, "my_parent2", 0, "my_parent2") << std::endl;

  for ( unsigned int i = 0; i < runs ; i++ )

  {
    Stamped<Pose> inpose (btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(10 + i), "grandchild");

    try{
    Stamped<Pose> outpose;
    outpose.setIdentity(); //to make sure things are getting mutated
    mTR.transformPose("my_parent",inpose, outpose);
    EXPECT_NEAR(outpose.getOrigin().x(), 2*xvalues[i], epsilon);
    EXPECT_NEAR(outpose.getOrigin().y(), 2*yvalues[i], epsilon);
    EXPECT_NEAR(outpose.getOrigin().z(), 2*zvalues[i], epsilon);
    }
    catch (tf::TransformException & ex)
    {
      std::cout << "TransformExcepion got through!!!!! " << ex.what() << std::endl;
      bool exception_improperly_thrown = true;
      EXPECT_FALSE(exception_improperly_thrown);
    }
  }
  
}


TEST(tf, ListOneForward)
{
  unsigned int runs = 4;
  double epsilon = 1e-6;
  seed_rand();
  
  tf::Transformer mTR(true);
  std::vector<double> xvalues(runs), yvalues(runs), zvalues(runs);
  for ( uint64_t i = 0; i < runs ; i++ )
  {
    xvalues[i] = 10.0 * ((double) rand() - (double)RAND_MAX /2.0) /(double)RAND_MAX;
    yvalues[i] = 10.0 * ((double) rand() - (double)RAND_MAX /2.0) /(double)RAND_MAX;
    zvalues[i] = 10.0 * ((double) rand() - (double)RAND_MAX /2.0) /(double)RAND_MAX;

    StampedTransform tranStamped(btTransform(btQuaternion(0,0,0,1), btVector3(xvalues[i],yvalues[i],zvalues[i])), ros::Time().fromNSec(10 + i),  "my_parent", "child");
    mTR.setTransform(tranStamped);
  }

  //  std::cout << mTR.allFramesAsString() << std::endl;
  //  std::cout << mTR.chainAsString("child", 0, "my_parent2", 0, "my_parent2") << std::endl;

  for ( uint64_t i = 0; i < runs ; i++ )

  {
    Stamped<Pose> inpose (btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(10 + i), "my_parent");

    try{
    Stamped<Pose> outpose;
    outpose.setIdentity(); //to make sure things are getting mutated
    mTR.transformPose("child",inpose, outpose);
    EXPECT_NEAR(outpose.getOrigin().x(), -xvalues[i], epsilon);
    EXPECT_NEAR(outpose.getOrigin().y(), -yvalues[i], epsilon);
    EXPECT_NEAR(outpose.getOrigin().z(), -zvalues[i], epsilon);
    }
    catch (tf::TransformException & ex)
    {
      std::cout << "TransformExcepion got through!!!!! " << ex.what() << std::endl;
      bool exception_improperly_thrown = true;
      EXPECT_FALSE(exception_improperly_thrown);
    }
  }
  
}

TEST(tf, ListTwoForward)
{
  unsigned int runs = 4;
  double epsilon = 1e-6;
  seed_rand();
  
  tf::Transformer mTR(true);
  std::vector<double> xvalues(runs), yvalues(runs), zvalues(runs);
  for ( unsigned int i = 0; i < runs ; i++ )
  {
    xvalues[i] = 10.0 * ((double) rand() - (double)RAND_MAX /2.0) /(double)RAND_MAX;
    yvalues[i] = 10.0 * ((double) rand() - (double)RAND_MAX /2.0) /(double)RAND_MAX;
    zvalues[i] = 10.0 * ((double) rand() - (double)RAND_MAX /2.0) /(double)RAND_MAX;

    StampedTransform tranStamped(btTransform(btQuaternion(0,0,0,1), btVector3(xvalues[i],yvalues[i],zvalues[i])), ros::Time().fromNSec(10 + i),  "my_parent", "child");
    mTR.setTransform(tranStamped);
    StampedTransform tranStamped2(btTransform(btQuaternion(0,0,0,1), btVector3(xvalues[i],yvalues[i],zvalues[i])), ros::Time().fromNSec(10 + i),  "child", "grandchild");
    mTR.setTransform(tranStamped2);
  }

  //  std::cout << mTR.allFramesAsString() << std::endl;
  //  std::cout << mTR.chainAsString("child", 0, "my_parent2", 0, "my_parent2") << std::endl;

  for ( unsigned int i = 0; i < runs ; i++ )

  {
    Stamped<Pose> inpose (btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(10 + i), "my_parent");

    try{
    Stamped<Pose> outpose;
    outpose.setIdentity(); //to make sure things are getting mutated
    mTR.transformPose("grandchild",inpose, outpose);
    EXPECT_NEAR(outpose.getOrigin().x(), -2*xvalues[i], epsilon);
    EXPECT_NEAR(outpose.getOrigin().y(), -2*yvalues[i], epsilon);
    EXPECT_NEAR(outpose.getOrigin().z(), -2*zvalues[i], epsilon);
    }
    catch (tf::TransformException & ex)
    {
      std::cout << "TransformExcepion got through!!!!! " << ex.what() << std::endl;
      bool exception_improperly_thrown = true;
      EXPECT_FALSE(exception_improperly_thrown);
    }
  }
  
}

TEST(tf, TransformThrougRoot)
{
  unsigned int runs = 4;
  double epsilon = 1e-6;
  seed_rand();
  
  tf::Transformer mTR(true);
  std::vector<double> xvalues(runs), yvalues(runs), zvalues(runs);
  for ( unsigned int i = 0; i < runs ; i++ )
  {
    xvalues[i] = 10.0 * ((double) rand() - (double)RAND_MAX /2.0) /(double)RAND_MAX;
    yvalues[i] = 10.0 * ((double) rand() - (double)RAND_MAX /2.0) /(double)RAND_MAX;
    zvalues[i] = 10.0 * ((double) rand() - (double)RAND_MAX /2.0) /(double)RAND_MAX;

    StampedTransform tranStamped(btTransform(btQuaternion(0,0,0,1), btVector3(xvalues[i],yvalues[i],zvalues[i])), ros::Time().fromNSec(1000 + i*100),  "my_parent", "childA");
    mTR.setTransform(tranStamped);
    StampedTransform tranStamped2(btTransform(btQuaternion(0,0,0,1), btVector3(xvalues[i],yvalues[i],zvalues[i])), ros::Time().fromNSec(1000 + i*100),  "my_parent", "childB");
    mTR.setTransform(tranStamped2);
  }

  //  std::cout << mTR.allFramesAsString() << std::endl;
  //  std::cout << mTR.chainAsString("child", 0, "my_parent2", 0, "my_parent2") << std::endl;

  for ( unsigned int i = 0; i < runs ; i++ )

  {
    Stamped<Pose> inpose (btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(1000 + i*100), "childA");

    try{
    Stamped<Pose> outpose;
    outpose.setIdentity(); //to make sure things are getting mutated
    mTR.transformPose("childB",inpose, outpose);
    EXPECT_NEAR(outpose.getOrigin().x(), 0*xvalues[i], epsilon);
    EXPECT_NEAR(outpose.getOrigin().y(), 0*yvalues[i], epsilon);
    EXPECT_NEAR(outpose.getOrigin().z(), 0*zvalues[i], epsilon);
    }
    catch (tf::TransformException & ex)
    {
      std::cout << "TransformExcepion got through!!!!! " << ex.what() << std::endl;
      bool exception_improperly_thrown = true;
      EXPECT_FALSE(exception_improperly_thrown);
    }
  }
  
}

TEST(tf, TransformThroughNO_PARENT)
{
  unsigned int runs = 4;
  double epsilon = 1e-6;
  seed_rand();
  
  tf::Transformer mTR(true);
  std::vector<double> xvalues(runs), yvalues(runs), zvalues(runs);
  for ( unsigned int i = 0; i < runs ; i++ )
  {
    xvalues[i] = 10.0 * ((double) rand() - (double)RAND_MAX /2.0) /(double)RAND_MAX;
    yvalues[i] = 10.0 * ((double) rand() - (double)RAND_MAX /2.0) /(double)RAND_MAX;
    zvalues[i] = 10.0 * ((double) rand() - (double)RAND_MAX /2.0) /(double)RAND_MAX;

    StampedTransform tranStamped(btTransform(btQuaternion(0,0,0,1), btVector3(xvalues[i],yvalues[i],zvalues[i])), ros::Time().fromNSec(10 + i),  "my_parentA", "childA");
    mTR.setTransform(tranStamped);
    StampedTransform tranStamped2(btTransform(btQuaternion(0,0,0,1), btVector3(xvalues[i],yvalues[i],zvalues[i])), ros::Time().fromNSec(10 + i),  "my_parentB", "childB");
    mTR.setTransform(tranStamped2);
  }

  //  std::cout << mTR.allFramesAsString() << std::endl;
  //  std::cout << mTR.chainAsString("child", 0, "my_parent2", 0, "my_parent2") << std::endl;

  for ( unsigned int i = 0; i < runs ; i++ )

  {
    Stamped<btTransform> inpose (btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(10 + i), "childA");
    bool exception_thrown = false;

    try{
    Stamped<btTransform> outpose;
    outpose.setIdentity(); //to make sure things are getting mutated
    mTR.transformPose("childB",inpose, outpose);
    EXPECT_NEAR(outpose.getOrigin().x(), 0*xvalues[i], epsilon);
    EXPECT_NEAR(outpose.getOrigin().y(), 0*yvalues[i], epsilon);
    EXPECT_NEAR(outpose.getOrigin().z(), 0*zvalues[i], epsilon);
    }
    catch (tf::TransformException & ex)
    {
      exception_thrown = true;
    }
    EXPECT_TRUE(exception_thrown);
  }
  
}




TEST(tf, NO_PARENT_SET)
{
  double epsilon = 1e-6;
  
  std::vector<std::string> children;
  std::vector<std::string> parents;



  children.push_back("b");
  parents.push_back("a");
  children.push_back("a");
  parents.push_back("NO_PARENT");

  tf::Transformer mTR(true);

  for (uint64_t i = 0; i <  children.size(); i++)
    {
      StampedTransform tranStamped(btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(10),  parents[i], children[i]);
      mTR.setTransform(tranStamped);
    }

  //std::cout << mTR.allFramesAsString() << std::endl;


  Stamped<btTransform> inpose (btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(10), "a");
  Stamped<btTransform> outpose;
  outpose.setIdentity(); //to make sure things are getting mutated
  mTR.transformPose("a",inpose, outpose);
  EXPECT_NEAR(outpose.getOrigin().x(), 0, epsilon);
  EXPECT_NEAR(outpose.getOrigin().y(), 0, epsilon);
  EXPECT_NEAR(outpose.getOrigin().z(), 0, epsilon);
  
}


TEST(tf, Exceptions)
{

 tf::Transformer mTR(true);

 
 Stamped<btTransform> outpose;

 //connectivity when no data
 EXPECT_FALSE(mTR.canTransform("parent", "me", ros::Time().fromNSec(10000000)));
 try 
 {
   mTR.transformPose("parent",Stamped<Pose>(btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(10000000) , "me"), outpose);
   EXPECT_FALSE("ConnectivityException Not Thrown");   
 }
 catch ( tf::LookupException &ex)
 {
   EXPECT_TRUE("Lookupgh Exception Caught");
 }
 catch (tf::TransformException& ex)
 {
   printf("%s\n",ex.what());
   EXPECT_FALSE("Other Exception Caught");
 }
 
 mTR.setTransform( StampedTransform(btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(100000), "parent", "me"));

 //Extrapolation not valid with one value
 EXPECT_FALSE(mTR.canTransform("parent", "me", ros::Time().fromNSec(200000)));
 try 
 {
   mTR.transformPose("parent",Stamped<Pose>(btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(200000) , "me"), outpose);
   EXPECT_TRUE("ExtrapolationException Not Thrown");
 }
 catch ( tf::ExtrapolationException &ex)
 {
   EXPECT_TRUE("Extrapolation Exception Caught");
 }
 catch (tf::TransformException& ex)
 {
   printf("%s\n",ex.what());
   EXPECT_FALSE("Other Exception Caught");
 }
 

 mTR.setTransform( StampedTransform (btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(300000), "parent", "me"));

 //NO Extration when Interpolating
 //inverse list
 EXPECT_TRUE(mTR.canTransform("parent", "me", ros::Time().fromNSec(200000)));
 try 
 {
   mTR.transformPose("parent",Stamped<Pose>(btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(200000) , "me"), outpose);
   EXPECT_TRUE("ExtrapolationException Not Thrown");
 }
 catch ( tf::ExtrapolationException &ex)
 {
   EXPECT_FALSE("Extrapolation Exception Caught");
 }
 catch (tf::TransformException& ex)
 {
   printf("%s\n",ex.what());
   EXPECT_FALSE("Other Exception Caught");
 }



 //forward list
 EXPECT_TRUE(mTR.canTransform("me", "parent", ros::Time().fromNSec(200000)));
 try 
 {
   mTR.transformPose("me",Stamped<Pose>(btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(200000) , "parent"), outpose);
   EXPECT_TRUE("ExtrapolationException Not Thrown");
 }
 catch ( tf::ExtrapolationException &ex)
 {
   EXPECT_FALSE("Extrapolation Exception Caught");
 }
 catch (tf::TransformException& ex)
 {
   printf("%s\n",ex.what());
   EXPECT_FALSE("Other Exception Caught");
 }
  

 //Extrapolating backwards
 //inverse list
 EXPECT_FALSE(mTR.canTransform("parent", "me", ros::Time().fromNSec(1000)));
 try 
 {
   mTR.transformPose("parent",Stamped<Pose> (btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(1000) , "me"), outpose);
   EXPECT_FALSE("ExtrapolationException Not Thrown");
 }
 catch ( tf::ExtrapolationException &ex)
 {
   EXPECT_TRUE("Extrapolation Exception Caught");
 }
 catch (tf::TransformException& ex)
 {
   printf("%s\n",ex.what());
   EXPECT_FALSE("Other Exception Caught");
 }
 //forwards list
 EXPECT_FALSE(mTR.canTransform("me", "parent", ros::Time().fromNSec(1000)));
 try 
 {
   mTR.transformPose("me",Stamped<Pose> (btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(1000) , "parent"), outpose);
   EXPECT_FALSE("ExtrapolationException Not Thrown");
 }
 catch ( tf::ExtrapolationException &ex)
 {
   EXPECT_TRUE("Extrapolation Exception Caught");
 }
 catch (tf::TransformException& ex)
 {
   printf("%s\n",ex.what());
   EXPECT_FALSE("Other Exception Caught");
 }
  


 // Test extrapolation inverse and forward linkages FORWARD

 //inverse list
 EXPECT_FALSE(mTR.canTransform("parent", "me", ros::Time().fromNSec(350000)));
 try 
 {
   mTR.transformPose("parent", Stamped<Pose> (btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(350000) , "me"), outpose);
   EXPECT_FALSE("ExtrapolationException Not Thrown");
 }
 catch ( tf::ExtrapolationException &ex)
 {
   EXPECT_TRUE("Extrapolation Exception Caught");
 }
 catch (tf::TransformException& ex)
 {
   printf("%s\n",ex.what());
   EXPECT_FALSE("Other Exception Caught");
 }

 //forward list
 EXPECT_FALSE(mTR.canTransform("parent", "me", ros::Time().fromNSec(350000)));
 try 
 {
   mTR.transformPose("me", Stamped<Pose> (btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(350000) , "parent"), outpose);
   EXPECT_FALSE("ExtrapolationException Not Thrown");
 }
 catch ( tf::ExtrapolationException &ex)
 {
   EXPECT_TRUE("Extrapolation Exception Caught");
 }
 catch (tf::TransformException& ex)
 {
   printf("%s\n",ex.what());
   EXPECT_FALSE("Other Exception Caught");
 }
  



}



TEST(tf, NoExtrapolationExceptionFromParent)
{
  tf::Transformer mTR(true, ros::Duration().fromNSec(1000000));
  


  mTR.setTransform(  StampedTransform (btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(1000), "parent", "a"));
  mTR.setTransform(  StampedTransform (btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(10000),  "parent", "a"));
  
  
  mTR.setTransform(  StampedTransform (btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(1000),  "parent", "b"));
  mTR.setTransform(  StampedTransform (btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(10000),  "parent", "b"));

  mTR.setTransform(  StampedTransform (btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(1000),  "parent's parent", "parent"));
  mTR.setTransform(  StampedTransform (btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(1000),  "parent's parent's parent", "parent's parent"));

  mTR.setTransform(  StampedTransform (btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(10000),  "parent's parent", "parent"));
  mTR.setTransform(  StampedTransform (btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(10000),  "parent's parent's parent", "parent's parent"));

  Stamped<Point> output;

  try
  {
    mTR.transformPoint( "b", Stamped<Point>(Point(1,1,1), ros::Time().fromNSec(2000), "a"), output);
  }
  catch (ExtrapolationException &ex)
  {
    EXPECT_FALSE("Shouldn't have gotten this exception");
  }



};



TEST(tf, ExtrapolationFromOneValue)
{
  tf::Transformer mTR(true, ros::Duration().fromNSec(1000000));
  


  mTR.setTransform(  StampedTransform (btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(1000),  "parent", "a"));

  mTR.setTransform(  StampedTransform (btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(1000),  "parent's parent", "parent"));


  Stamped<Point> output;

  bool excepted = false;
  //Past time
  try
  {
    mTR.transformPoint( "parent", Stamped<Point>(Point(1,1,1), ros::Time().fromNSec(10), "a"), output);
  }
  catch (ExtrapolationException &ex)
  {
    excepted = true;
  }
  
  EXPECT_TRUE(excepted);

  excepted = false;
  //Future one element
  try
  {
    mTR.transformPoint( "parent", Stamped<Point>(Point(1,1,1), ros::Time().fromNSec(100000), "a"), output);
  }
  catch (ExtrapolationException &ex)
  {
    excepted = true;
  }
  
  EXPECT_TRUE(excepted);

  //Past multi link
  excepted = false;
  try
  {
    mTR.transformPoint( "parent's parent", Stamped<Point>(Point(1,1,1), ros::Time().fromNSec(1), "a"), output);
  }
  catch (ExtrapolationException &ex)
  {
    excepted = true;
  }
  
  EXPECT_TRUE(excepted);

  //Future case multi link
  excepted = false;
  try
  {
    mTR.transformPoint( "parent's parent", Stamped<Point>(Point(1,1,1), ros::Time().fromNSec(10000), "a"), output);
  }
  catch (ExtrapolationException &ex)
  {
    excepted = true;
  }
  
  EXPECT_TRUE(excepted);

  mTR.setTransform(  StampedTransform (btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)), ros::Time().fromNSec(20000),  "parent", "a"));

  excepted = false;
  try
  {
    mTR.transformPoint( "parent", Stamped<Point>(Point(1,1,1), ros::Time().fromNSec(10000), "a"), output);
  }
  catch (ExtrapolationException &ex)
  {
    excepted = true;
  }
  
  EXPECT_FALSE(excepted);

};



TEST(tf, RepeatedTimes)
{
  Transformer mTR;
  mTR.setTransform(  StampedTransform (btTransform(btQuaternion(1,0,0), btVector3(0,0,0)), ros::Time().fromNSec(4000),  "parent", "b"));
  mTR.setTransform(  StampedTransform (btTransform(btQuaternion(1,1,0), btVector3(0,0,0)), ros::Time().fromNSec(4000),  "parent", "b"));

  tf::StampedTransform  output;
  try{
    mTR.lookupTransform("parent", "b" , ros::Time().fromNSec(4000), output);
    EXPECT_TRUE(!std::isnan(output.getOrigin().x()));
    EXPECT_TRUE(!std::isnan(output.getOrigin().y()));
    EXPECT_TRUE(!std::isnan(output.getOrigin().z()));
    EXPECT_TRUE(!std::isnan(output.getRotation().x()));
    EXPECT_TRUE(!std::isnan(output.getRotation().y()));
    EXPECT_TRUE(!std::isnan(output.getRotation().z()));
    EXPECT_TRUE(!std::isnan(output.getRotation().w()));
  }
  catch (...)
  {
    EXPECT_FALSE("Excetion improperly thrown");
  }
  

}

TEST(tf, canTransform)
{
  Transformer mTR;

  //confirm zero length list disconnected will return true
  EXPECT_TRUE(mTR.canTransform("some_frame","some_frame", ros::Time()));
  EXPECT_TRUE(mTR.canTransform("some_frame","some_frame", ros::Time::now()));

  //Create a two link tree between times 10 and 20
  for (int i = 10; i < 20; i++)
  {
    mTR.setTransform(  StampedTransform (btTransform(btQuaternion(1,0,0), btVector3(0,0,0)), ros::Time().fromSec(i),  "parent", "child"));
    mTR.setTransform(  StampedTransform (btTransform(btQuaternion(1,0,0), btVector3(0,0,0)), ros::Time().fromSec(i),  "parent", "other_child"));
  }

  // four different timestamps related to tf state
  ros::Time zero_time = ros::Time().fromSec(0);
  ros::Time old_time = ros::Time().fromSec(5);
  ros::Time valid_time = ros::Time().fromSec(15);
  ros::Time future_time = ros::Time().fromSec(25);


  //confirm zero length list disconnected will return true
  EXPECT_TRUE(mTR.canTransform("some_frame","some_frame", zero_time));
  EXPECT_TRUE(mTR.canTransform("some_frame","some_frame", old_time));
  EXPECT_TRUE(mTR.canTransform("some_frame","some_frame", valid_time));
  EXPECT_TRUE(mTR.canTransform("some_frame","some_frame", future_time));

  // Basic API Tests

  //Valid data should pass
  EXPECT_TRUE(mTR.canTransform("child", "parent", valid_time));
  EXPECT_TRUE(mTR.canTransform("child", "other_child", valid_time));

  //zero data should pass
  EXPECT_TRUE(mTR.canTransform("child", "parent", zero_time));
  EXPECT_TRUE(mTR.canTransform("child", "other_child", zero_time));

  //Old data should fail
  EXPECT_FALSE(mTR.canTransform("child", "parent", old_time));
  EXPECT_FALSE(mTR.canTransform("child", "other_child", old_time));

  //Future data should fail
  EXPECT_FALSE(mTR.canTransform("child", "parent", future_time));
  EXPECT_FALSE(mTR.canTransform("child", "other_child", future_time));

  //Same Frame should pass for all times
  EXPECT_TRUE(mTR.canTransform("child", "child", zero_time));
  EXPECT_TRUE(mTR.canTransform("child", "child", old_time));
  EXPECT_TRUE(mTR.canTransform("child", "child", valid_time));
  EXPECT_TRUE(mTR.canTransform("child", "child", future_time));

  // Advanced API Tests

  // Source = Fixed
  //zero data in fixed frame should pass
  EXPECT_TRUE(mTR.canTransform("child", zero_time, "parent", valid_time, "child"));
  EXPECT_TRUE(mTR.canTransform("child", zero_time, "other_child", valid_time, "child"));
  //Old data in fixed frame should pass
  EXPECT_TRUE(mTR.canTransform("child", old_time, "parent", valid_time, "child"));
  EXPECT_TRUE(mTR.canTransform("child", old_time, "other_child", valid_time, "child"));
  //valid data in fixed frame should pass
  EXPECT_TRUE(mTR.canTransform("child", valid_time, "parent", valid_time, "child"));
  EXPECT_TRUE(mTR.canTransform("child", valid_time, "other_child", valid_time, "child"));
  //future data in fixed frame should pass
  EXPECT_TRUE(mTR.canTransform("child", future_time, "parent", valid_time, "child"));
  EXPECT_TRUE(mTR.canTransform("child", future_time, "other_child", valid_time, "child"));

  //transforming through fixed into the past
  EXPECT_FALSE(mTR.canTransform("child", valid_time, "parent", old_time, "child"));
  EXPECT_FALSE(mTR.canTransform("child", valid_time, "other_child", old_time, "child"));
  //transforming through fixed into the future
  EXPECT_FALSE(mTR.canTransform("child", valid_time, "parent", future_time, "child"));
  EXPECT_FALSE(mTR.canTransform("child", valid_time, "other_child", future_time, "child"));

  // Target = Fixed
  //zero data in fixed frame should pass
  EXPECT_TRUE(mTR.canTransform("child", zero_time, "parent", valid_time, "parent"));
  //Old data in fixed frame should pass
  EXPECT_FALSE(mTR.canTransform("child", old_time, "parent", valid_time, "parent"));
  //valid data in fixed frame should pass
  EXPECT_TRUE(mTR.canTransform("child", valid_time, "parent", valid_time, "parent"));
  //future data in fixed frame should pass
  EXPECT_FALSE(mTR.canTransform("child", future_time, "parent", valid_time, "parent"));

  //transforming through fixed into the zero
  EXPECT_TRUE(mTR.canTransform("child", valid_time, "parent", zero_time, "parent"));
  //transforming through fixed into the past
  EXPECT_TRUE(mTR.canTransform("child", valid_time, "parent", old_time, "parent"));
  //transforming through fixed into the valid
  EXPECT_TRUE(mTR.canTransform("child", valid_time, "parent", valid_time, "parent"));
  //transforming through fixed into the future
  EXPECT_TRUE(mTR.canTransform("child", valid_time, "parent", future_time, "parent"));

}

TEST(tf, lookupTransform)
{
  Transformer mTR;
  //Create a two link tree between times 10 and 20
  for (int i = 10; i < 20; i++)
  {
    mTR.setTransform(  StampedTransform (btTransform(btQuaternion(1,0,0), btVector3(0,0,0)), ros::Time().fromSec(i),  "parent", "child"));
    mTR.setTransform(  StampedTransform (btTransform(btQuaternion(1,0,0), btVector3(0,0,0)), ros::Time().fromSec(i),  "parent", "other_child"));
  }

  // four different timestamps related to tf state
  ros::Time zero_time = ros::Time().fromSec(0);
  ros::Time old_time = ros::Time().fromSec(5);
  ros::Time valid_time = ros::Time().fromSec(15);
  ros::Time future_time = ros::Time().fromSec(25);

  //output
  tf::StampedTransform output;

  // Basic API Tests

  try
  {
    //confirm zero length list disconnected will return true
    mTR.lookupTransform("some_frame","some_frame", zero_time, output);
    mTR.lookupTransform("some_frame","some_frame", old_time, output);
    mTR.lookupTransform("some_frame","some_frame", valid_time, output);
    mTR.lookupTransform("some_frame","some_frame", future_time, output);
    mTR.lookupTransform("child","child", future_time, output);
    mTR.lookupTransform("other_child","other_child", future_time, output);

    //Valid data should pass
    mTR.lookupTransform("child", "parent", valid_time, output);
    mTR.lookupTransform("child", "other_child", valid_time, output);
    
    //zero data should pass
    mTR.lookupTransform("child", "parent", zero_time, output);
    mTR.lookupTransform("child", "other_child", zero_time, output);
  }
  catch (tf::TransformException &ex)
  {
    printf("Exception improperly thrown: %s", ex.what());
    EXPECT_FALSE("Exception thrown");
  }
  try
  {
    //Old data should fail
    mTR.lookupTransform("child", "parent", old_time, output);
    EXPECT_FALSE("Exception should have been thrown");
  }
  catch (tf::TransformException)
  {
    EXPECT_TRUE("Exception Thrown Correctly");
  }
  try {
    //Future data should fail
    mTR.lookupTransform("child", "parent", future_time, output);
    EXPECT_FALSE("Exception should have been thrown");
  }
  catch (tf::TransformException)
  {
    EXPECT_TRUE("Exception Thrown Correctly");
  }
    
  try {
    //Same Frame should pass for all times
    mTR.lookupTransform("child", "child", zero_time, output);
    mTR.lookupTransform("child", "child", old_time, output);
    mTR.lookupTransform("child", "child", valid_time, output);
    mTR.lookupTransform("child", "child", future_time, output);
    
    // Advanced API Tests
    
    // Source = Fixed
    //zero data in fixed frame should pass
    mTR.lookupTransform("child", zero_time, "parent", valid_time, "child", output);
    mTR.lookupTransform("child", zero_time, "other_child", valid_time, "child", output);
    //Old data in fixed frame should pass
    mTR.lookupTransform("child", old_time, "parent", valid_time, "child", output);
    mTR.lookupTransform("child", old_time, "other_child", valid_time, "child", output);
    //valid data in fixed frame should pass
    mTR.lookupTransform("child", valid_time, "parent", valid_time, "child", output);
    mTR.lookupTransform("child", valid_time, "other_child", valid_time, "child", output);
    //future data in fixed frame should pass
    mTR.lookupTransform("child", future_time, "parent", valid_time, "child", output);
    mTR.lookupTransform("child", future_time, "other_child", valid_time, "child", output);
  }
  catch (tf::TransformException &ex)
  {
    printf("Exception improperly thrown: %s", ex.what());
    EXPECT_FALSE("Exception incorrectly thrown");
  }

  try {
    //transforming through fixed into the past
    mTR.lookupTransform("child", valid_time, "parent", old_time, "child", output);
    EXPECT_FALSE("Exception should have been thrown");
  }
  catch (tf::TransformException)
  {
    EXPECT_TRUE("Exception Thrown Correctly");
  }

  try {
    //transforming through fixed into the future
    mTR.lookupTransform("child", valid_time, "parent", future_time, "child", output);
    EXPECT_FALSE("Exception should have been thrown");
  }
  catch (tf::TransformException)
  {
    EXPECT_TRUE("Exception Thrown Correctly");
  }

  try {
    // Target = Fixed
    //zero data in fixed frame should pass
    mTR.lookupTransform("child", zero_time, "parent", valid_time, "parent", output);
    //valid data in fixed frame should pass
    mTR.lookupTransform("child", valid_time, "parent", valid_time, "parent", output);
  }
  catch (tf::TransformException &ex)
  {
    printf("Exception improperly thrown: %s", ex.what());
    EXPECT_FALSE("Exception incorrectly thrown");
  }

  try {
  //Old data in fixed frame should pass
  mTR.lookupTransform("child", old_time, "parent", valid_time, "parent", output);
      EXPECT_FALSE("Exception should have been thrown");
  }
  catch (tf::TransformException)
  {
    EXPECT_TRUE("Exception Thrown Correctly");
  }
  try {
    //future data in fixed frame should pass
    mTR.lookupTransform("child", future_time, "parent", valid_time, "parent", output);
    EXPECT_FALSE("Exception should have been thrown");
  }
  catch (tf::TransformException)
  {
    EXPECT_TRUE("Exception Thrown Correctly");
  }

  try {
    //transforming through fixed into the zero
    mTR.lookupTransform("child", valid_time, "parent", zero_time, "parent", output);
    //transforming through fixed into the past
    mTR.lookupTransform("child", valid_time, "parent", old_time, "parent", output);
    //transforming through fixed into the valid
    mTR.lookupTransform("child", valid_time, "parent", valid_time, "parent", output);
    //transforming through fixed into the future
    mTR.lookupTransform("child", valid_time, "parent", future_time, "parent", output);
  }
  catch (tf::TransformException &ex)
  {
    printf("Exception improperly thrown: %s", ex.what());
    EXPECT_FALSE("Exception improperly thrown");
  }
  

  //make sure zero goes to now for zero length
  try
  {
    ros::Time now1 = ros::Time::now();

    mTR.lookupTransform("a", "a", ros::Time(),output);
    EXPECT_LE(now1.toSec(), output.stamp_.toSec());
    EXPECT_LE(output.stamp_.toSec(), ros::Time::now().toSec());
  }
  catch (tf::TransformException &ex)
  {
    printf("Exception improperly thrown: %s", ex.what());
    EXPECT_FALSE("Exception improperly thrown");
  }
  
}



bool expectInvalidQuaternion(tf::Quaternion q)
{
  try
  {
    tf::assertQuaternionValid(q);
    printf("this should have thrown\n");
    return false;
  }
  catch (tf::InvalidArgument &ex)  
  {
    return true;
  }
  catch  (...)
  {
    printf("A different type of exception was expected\n");
    return false;
  }
  return false;
}


int main(int argc, char **argv){
  testing::InitGoogleTest(&argc, argv);
  ros::Time::init(); //needed for ros::TIme::now()
  ros::init(argc, argv, "tf_unittest");
  return RUN_ALL_TESTS();
}