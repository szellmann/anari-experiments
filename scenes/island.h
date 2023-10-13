#pragma once

#include <asg/asg.hpp>
#include "../scenes.h"
#include <visionaray/math/vector.h>

namespace island
{ 
    asg::CylinderGeometry makeCylinder(float height, float radius)
    {
        float* cylVertex = new float[6] { 0.f,-height/2.f,0.f,
                                          0.f,height/2.f,0.f };
        float* cylRadius = new float[1] { radius };
        uint8_t* cylCap = new uint8_t[1] { 1 };
        asg::CylinderGeometry cyl = asg::newCylinderGeometry(
                cylVertex,cylRadius,nullptr,cylCap,2,nullptr,0);

        return cyl;
    }

    asg::Transform makeTransform(int axis, float angle, float tx, float ty, float tz)
    {
        float cosa = cosf(angle);
        float sina = sinf(angle);

        if (axis == 0) {
            float R[] = {1.f,0.f,0.f,
                         0.f,cosa,sina,
                         0.f,-sina,cosa,
                         tx,ty,tz};
            return asg::newTransform(R);
        } else if (axis == 1) {
            float R[] = {cosa,0.f,sina,
                         0.f,1.f,0.f,
                         -sina,0.f,cosa,
                         tx,ty,tz};
            return asg::newTransform(R);
        } else { assert(axis == 2);
            float R[] = {cosa,sina,0.f,
                         -sina,cosa,0.f,
                         0.f,0.f,1.f,
                         tx,ty,tz};
            return asg::newTransform(R);
        }
    }
    asg::TriangleGeometry makeCube(float width, float height, float depth)
    {
        float w2 = width*.5f;
        float h2 = height*.5f;
        float d2 = depth*.5f;
        float* cubeVertex = new float[24] {-w2,-h2,-d2,
                                            w2,-h2,-d2,
                                            w2, h2,-d2,
                                           -w2, h2,-d2,
                                            w2,-h2, d2,
                                           -w2,-h2, d2,
                                           -w2, h2, d2,
                                            w2, h2, d2};

        uint32_t* cubeIndex = new uint32_t[36] {0,1,2, 0,2,3, 4,5,6, 4,6,7,
                                                1,4,7, 1,7,2, 5,0,3, 5,3,6,
                                                5,4,1, 5,1,0, 3,2,7, 3,7,6};
        asg::TriangleGeometry cube = asg::newTriangleGeometry(
                cubeVertex,NULL,NULL,8,cubeIndex,12,NULL,NULL,NULL,NULL);

        return cube;
    }
}
struct Sky
{
     asg::Object m_sceneGraph = nullptr;
};
struct Ship
{
    Ship(){
        initSceneGraph();
    }
  
    void initSceneGraph()
    {
        if (m_sceneGraph != NULL) // this should never happen!
            m_sceneGraph->release();

        // start with root node of scene graph
        m_sceneGraph = asg::newObject();
        auto obj = asg::newObject();
        asg::Transform obj_trans = island::makeTransform(1, -1.5708f,0,0.25 + -1.04308 * pow(10,-7),0);
        obj->addChild(obj_trans);
        m_sceneGraph->addChild(obj);

        // the shape of one square
        
        //indexed face set
       
        float* points = new float[105] {3, 1, -1,
			            1.5, 1, -1.25,
			            0, 1, -1.5,
			            -1.5, 1, -1.25,
			            -2.5, 1, -0.75,
			            -3, 1, 0,
			            -2.5, 1, 0.75,
			            -1.5, 1, 1.25,
			            0, 1, 1.5,
			            1.5, 1, 1.25,
			            3, 1, 1,
			            2.75, 0.5, -0.75,
			            1.5, 0.5, -1,
			            0.25, 0.5, -1.25,
			            -1.5, 0.5, -1,
			            -2, 0.5, -0.5,
			            -2.25, 0.5, 0,
			            -2, 0.5, 0.5,
			            -1.5, 0.5, 1,
			            0.25, 0.5, 1.25,
			            1.5, 0.5, 1,
			            2.75, 0.5, 0.75,
			            2.5, 0, -0.5,
			            1.25, 0, -0.75,
			            0.25, 0, -0.75,
			            -0.75, 0, -0.5,
			            -1.25, 0, 0,
			            -0.75, 0, 0.5,
			            0.25, 0, 0.75,
			            1.25, 0, 0.75,
			            2.5, 0, 0.5,
			            2.25, -0.25, 0,
			            1.25, -0.5, 0,
			            0.25, -0.5, 0,
			            -0.5, -0.25, 0 };

         uint32_t* index = new uint32_t[60] { 0, 11, 1, 
                                              1, 11, 12,
			                                  1, 12, 2,
                                              2, 12, 13,
			                                  2, 13, 3,
                                              3, 13, 14,
                                              3, 14, 4, 
                                              4, 14, 15,
			                                  4, 15, 5,
                                              5, 15, 16,
			                                  5, 16, 17,
                                              5, 17, 6,
			                                  6, 17, 18,
                                              6, 18, 7,
			                                  7, 18, 19,
                                              7, 19, 8,
			                                  8, 19, 20, 
                                              8, 20, 9,
			                                  9, 20, 21,
                                              9, 21, 10 };

        asg::TriangleGeometry ind_face_set1 = asg::newTriangleGeometry(
                points,NULL,NULL,35,index,20,NULL,NULL,NULL,NULL);
        asg::Material ind_face_set1_mat = asg::newMaterial("matte");
               ind_face_set1_mat->setParam(asg::Param("kd",0.8f, 0.713178f, 0.76912f));
        asg::Transform trans_ind_face_set1 = island::makeTransform(0,0.f,0, 0.75, 0.0);
        trans_ind_face_set1->addChild(asg::newSurface(ind_face_set1, ind_face_set1_mat));
        obj->addChild(trans_ind_face_set1);

          uint32_t* index2 = new uint32_t[54] { 11, 22, 12, 12, 22, 23,
			                                    12, 23, 13, 13, 23, 24,
			                                    13, 24, 14, 14, 24, 25,
			                                    14, 25, 15, 15, 25, 26,
			                                    15, 26, 16, 16, 26, 17,
			                                    17, 26, 27, 17, 27, 18,
			                                    18, 27, 28, 18, 28, 19,
			                                    19, 28, 29, 19, 29, 20,
			                                    20, 29, 30, 20, 30, 21 } ; 

        asg::TriangleGeometry ind_face_set2 = asg::newTriangleGeometry(
                points,NULL,NULL,35,index2,18,NULL,NULL,NULL,NULL);
        asg::Material ind_face_set2_mat = asg::newMaterial("matte");
        ind_face_set2_mat->setParam(asg::Param("kd",0.425949f, 0.528619f, 0.8f));
        asg::Transform trans_ind_face_set2 = island::makeTransform(0,0.f,0.25, 0.25, -2.98023/ pow(10,8));
        trans_ind_face_set2->addChild(asg::newSurface(ind_face_set2, ind_face_set2_mat));
        obj->addChild(trans_ind_face_set2);


        uint32_t* index3 = new uint32_t[42] { 22, 31, 23,
                                              23, 31, 32,
			                                  23, 32, 24,
                                              24, 32, 33,
			                                  24, 33, 25,
                                              25, 33, 34,
			                                  25, 34, 26,
                                              26, 34, 27,
			                                  27, 34, 33, 
                                              27, 33, 28,
			                                  28, 33, 32,
                                              28, 32, 29,
			                                  29, 32, 31,
                                              29, 31, 30 
                                            };
        
        asg::TriangleGeometry ind_face_set3 = asg::newTriangleGeometry(
                points,NULL,NULL,35,index3,14,NULL,NULL,NULL,NULL);
        asg::Material ind_face_set3_mat = asg::newMaterial("matte");
        ind_face_set3_mat->setParam(asg::Param("kd",0.723297f, 0.742473f, 0.8f));
        obj->addChild(asg::newSurface(ind_face_set3, ind_face_set3_mat));

          uint32_t* index4 = new uint32_t[15] { 0, 10, 11, 
                                                11, 10, 21,
			                                    11, 21, 22, 
                                                22, 21, 30,
			                                    22, 30, 31
                                            };
        
        asg::TriangleGeometry ind_face_set4 = asg::newTriangleGeometry(
                points,NULL,NULL,35,index4,5,NULL,NULL,NULL,NULL);
        asg::Material ind_face_set4_mat = asg::newMaterial("matte");
        obj->addChild(asg::newSurface(ind_face_set4, ind_face_set4_mat));


        uint32_t* index5 = new uint32_t[27] { 0, 1, 10,
                                              1, 9, 10,
			                                  1, 2, 9,
                                              2, 8, 9,
			                                  2, 3, 8,
                                              3, 7, 8,
			                                  3, 4, 7,
                                              4, 6, 7,
			                                  4, 5, 6}; 
            
        asg::TriangleGeometry ind_face_set5 = asg::newTriangleGeometry(
                points,NULL,NULL,35,index5,9,NULL,NULL,NULL,NULL);
        asg::Material ind_face_set5_mat = asg::newMaterial("matte");
        obj->addChild(asg::newSurface(ind_face_set5, ind_face_set5_mat));


        asg::Object cube = asg::newObject();
        asg::TriangleGeometry cube_geom = island::makeCube(1, 1,1);
        asg::Material cube_mat = asg::newMaterial("matte");
         cube_mat->setParam(asg::Param("kd",0.8f, 0.133383f, 0.185233f));
        asg::Transform transcube = island::makeTransform(0,0.f,0.342307, 0.933647, 0.0146482);
        transcube->addChild(asg::newSurface(cube_geom,cube_mat));
        obj->addChild(transcube);


        asg::Object cyl1 = asg::newObject();
        asg::CylinderGeometry cyl_geom1 = island::makeCylinder(2, 0.2f);
        asg::Material cyl_mat1 = asg::newMaterial("matte");
        cyl_mat1->setParam(asg::Param("kd",0.454304f, 0.522407f, 0.8f));
        asg::Transform trans1 = island::makeTransform(0,0.f,-0.7f, 1.5f, 0);
        trans1->addChild(asg::newSurface(cyl_geom1,cyl_mat1));
        obj->addChild(trans1);

        
        asg::Object cyl2 = asg::newObject();
        asg::CylinderGeometry cyl_geom2 = island::makeCylinder(2, 0.2f);
        asg::Material cyl_mat2 = asg::newMaterial("matte");
        asg::Transform trans2 = island::makeTransform(0,0,0,1.5,0);
        trans2->addChild(asg::newSurface(cyl_geom2,cyl_mat2));
        obj->addChild(trans2);


        asg::Object cyl3 = asg::newObject();
        asg::CylinderGeometry cyl_geom3 = island::makeCylinder(2, 0.2f);
        asg::Material cyl_mat3 = asg::newMaterial("matte");
        asg::Transform trans3 = island::makeTransform(0,0,0.7,1.5,0);
        trans3->addChild(asg::newSurface(cyl_geom3,cyl_mat3));
        obj->addChild(trans3);


        asg::Object cyl4 = asg::newObject();
        asg::CylinderGeometry cyl_geom4 = island::makeCylinder(2, 0.2f);
        asg::Material cyl_mat4 = asg::newMaterial("matte");
        asg::Transform trans4 = island::makeTransform(0,0,1.4,1.5,0);
        trans4->addChild(asg::newSurface(cyl_geom4,cyl_mat4));
        obj->addChild(trans4);

        
        // colors

        asg::Material greenColor = asg::newMaterial("matte");
        greenColor->setParam(asg::Param("kd",0.f,1.f,0.f));

        asg::Material redColor = asg::newMaterial("matte");
        redColor->setParam(asg::Param("kd",1.f,0.f,0.f));

       
    }
    asg::Object m_sceneGraph = nullptr;
};
struct Island
{
    asg::Object m_sceneGraph = nullptr;
    Island(){
        initSceneGraph();
    }
    void initSceneGraph()
    {
        m_sceneGraph = asg::newObject();
        auto obj = asg::newObject();
        m_sceneGraph->addChild(obj);
        asg::Object ground = asg::newObject();
        asg::Transform ground_trans = island::makeTransform(0,0.f,0, -2.f, 0.0);
        ground->addChild(ground_trans);
        obj->addChild(ground);
        //Surface

        float* points = new float[210] {27.6391, -1.75557, -31.7062,
				      -30.8926, -1.75558 ,-30.8222,
				      -30.8926 ,-1.75564, 30.763,
				      -0.060009 ,-0.79579, -9.09962,
				      -1.45977, 2.05291, -4.67986,
				      1.39347, 6.73367, -1.55778,
				      -0.782147, 4.83622 ,-1.29684,
				      -0.775763, 4.88207 ,1.01202,
				      -0.850866, -0.014241, 5.83507,
				      0.195148,-1.62194, 11.4319,
				      31.3791 ,-1.75563 ,29.4807,
				      -29.6103, -1.75558 ,-31.5087,
				      -31.5791 ,-1.75558 ,-29.5399,
				      -31.7244 ,-1.75558 ,-25.7478,
				      -31.5791 ,-1.75564, 29.4807,
				      13.5377, -1.67449, -3.09326,
				      9.88464, -1.02293, -1.59336,
				      6.71738, 0.428669 ,-2.94258,
				      4.9882 ,2.40808 ,-2.09548,
				      4.32984 ,0.958669 ,-4.10522,
				      1.19158 ,6.82022, -2.72392,
				      -0.403269 ,-1.16022 ,8.54765,
				      -0.086815 ,1.44414, 3.97684,
				      -4.69863, -0.883111 ,5.84248,
				      -0.359063, 4.3193, -3.94846,
				      31.1086 ,-1.75563, 30.1946,
				      30.1242 ,-1.75563 ,31.179,
				      30.1242 ,-1.75557, -31.2382,
				      31.1086 ,-1.75557 ,-30.2538,
				      8.65026 ,-1.56, 7.5159,
				      -8.86154 ,-1.63307, -8.16872,
				      -8.00656 ,-1.64386, 9.11234,
				      3.48848, 2.37778, 2.38558,
				      0.051859 ,-1.32694 ,-10.7094,
				      2.57863, 4.03321 ,-4.01883,
				      -5.9281 ,-0.25699 ,1.17408,
				      -2.40146 ,0.668804 ,4.04841,
				      4.70488 ,-1.53651, -11.6068,
				      3.83393, 4.66891 ,-1.45455,
				      0.226306 ,5.7347 ,-0.844334,
				      28.5722, -1.75563, 31.5958,
				      31.5254, -1.75557 ,-28.7018,
				      -28.7722 ,-1.75564 ,31.5958,
				      6.04388 ,-1.22726, 7.04125,
				      9.21036 ,-1.0032 ,-4.04637,
				      7.16227, 0.405077, -1.80648,
				      5.3018, -0.265163, 4.57253,
				      3.76122 ,3.86475, 1.58433,
				      4.80156, 2.19551, 0.685778,
				      3.30443 ,4.7467, 1.45998,
				      1.12275 ,5.83811 ,0.611587,
				      0.774506 ,0.806252 ,-6.9796,
				      0.026434, -1.74649 ,-13.8902,
				      -2.83608 ,0.214381, -5.9377,
				      0.720275, 7.02623 ,-1.68385,
				      -9.07951 ,-1.36281 ,1.31578,
				      -11.0086, -1.62174 ,1.61912,
				      -3.31646, 1.85285, -2.1656,
				      -7.69019 ,-0.979046 ,-2.60775,
				      -1.14155, 1.53432, 3.68085,
				      -0.540742 ,4.4997, 1.97302,
				      7.59332, -0.844461, 3.55232,
				      6.58523, -0.264617, 2.96109,
				      4.27657, -1.28339, -10.0978,
				      2.1282 ,1.19615 ,-6.26808,
				      2.65516 ,-0.258378 ,-7.89528,
				      -5.21086 ,-0.807199, -6.25868,
				      2.47902,6.51247 ,-0.498268,
				      -3.00833, 2.08575 ,0.849215,
				      1.93217 ,-0.378062 ,6.08267};

         uint32_t* index = new uint32_t[369] { 27, 0, 41, 2, 42, 14,
				  40, 14, 42, 45, 16, 44,
				  48, 45, 18, 17, 45, 44,
				  17, 18, 45, 38, 47, 48,
				  32, 48, 47, 38, 49, 47,
				  53, 3, 66, 53, 4, 51,
				  53, 51, 3, 66, 3, 33,
				  24, 51, 4, 5, 67, 20,
				  67, 38, 20, 23, 35, 55,
				  58, 55, 35, 57, 58, 35,
				  60, 7, 68, 16, 62, 61,
				  40, 15, 29, 16, 61, 29,
				  29, 61, 43, 32, 46, 62,
				  61, 46, 43, 43, 46, 69,
				  61, 62, 46, 44, 15, 63,
				  37, 63, 15, 44, 63, 65,
				  65, 63, 3, 51, 65, 3,
				  51, 64, 65, 19, 65, 64,
				  66, 30, 58, 31, 21, 23,
				  21, 8, 23, 8, 36, 23,
				  40, 10, 41, 40, 41, 15,
				  17, 65, 19, 17, 19, 18,
				  37, 15, 41, 52, 11, 30,
				  57, 24, 4, 20, 34, 64,
				  6, 24, 57, 6, 54, 24,
				  5, 54, 39, 11, 14, 56,
				  31, 56, 14, 23, 36, 35,
				  68, 36, 60, 35, 36, 68,
				  68, 7, 57, 57, 7, 6,
				  31, 14, 40, 32, 69, 46,
				  22, 69, 32, 22, 32, 49,
				  49, 50, 60, 14, 12, 13,
				  15, 44, 16, 17, 44, 65,
				  49, 38, 67, 20, 64, 51,
				  38, 34, 20, 21, 9, 43,
				  21, 69, 8, 21, 43, 69,
				  60, 59, 22, 60, 22, 49,
				  41, 0, 11, 45, 62, 16,
				  48, 62, 45, 32, 62, 48,
				  64, 18, 19, 57, 66, 58,
				  31, 9, 21, 60, 36, 59,
				  27, 41, 28, 38, 18, 34,
				  38, 48, 18, 49, 67, 50,
				  67, 5, 50, 7, 50, 39,
				  50, 5, 39, 11, 1, 12,
				  12, 14, 11, 11, 52, 41,
				  41, 52, 37, 52, 33, 37,
				  63, 33, 3, 63, 37, 33,
				  57, 4, 53, 57, 53, 66,
				  64, 34, 18, 51, 24, 20,
				  5, 20, 54, 54, 20, 24,
				  39, 54, 6, 6, 7, 39,
				  56, 30, 11, 35, 68, 57,
				  25, 10, 26, 26, 10, 40,
				  9, 31, 40, 8, 69, 22,
				  8, 22, 36, 22, 59, 36,
				  49, 32, 47, 50, 7, 60,
				  43, 9, 29, 29, 15, 16,
				  66, 33, 30, 30, 33, 52,
				  31, 55, 56, 23, 55, 31,
				  29, 9, 40, 30, 56, 55,
				  58, 30, 55  };

        asg::TriangleGeometry ind_sand = asg::newTriangleGeometry(
        points,NULL,NULL,70,index,123,NULL,NULL,NULL,NULL);
        asg::Material sand_mat = asg::newMaterial("matte");
        sand_mat->setParam(asg::Param("kd",0.8f, 0.631111f, 0.340331f));
      
        ground->addChild(asg::newSurface(ind_sand, sand_mat));
        //palm
//    SoInput palmInput;
//    if(!palmInput.openFile(Scene::getPathName("vegetation.iv")))
//    {
// 	   std::cerr << "failed to read surface.iv" << std::endl;
// 	   return;
//    }
//    SoSeparator *palmSep = SoDB::readAll(&palmInput);
//    palmInput.closeFile();
//    setPart("vegetation", palmSep);

    }

 
};
struct IslandScene: Scene
{
    IslandScene(ANARIDevice dev, ANARIWorld wrld)
        : Scene(dev,wrld)
    {
        asg::Object scene = asg::newObject();

       // scene->addChild(sky.m_sceneGraph);
        scene->addChild(ship.m_sceneGraph);
        scene->addChild(island.m_sceneGraph);
        root = (ASGObject)*scene;

        // Build up ANARI world
        ASG_SAFE_CALL(asgBuildANARIWorld(root,device,world,
                                         ASG_BUILD_WORLD_FLAG_FULL_REBUILD,0));

        // Compute bounding box
        ASG_SAFE_CALL(asgComputeBounds(root,&bbox.min.x,&bbox.min.y,&bbox.min.z,
                                       &bbox.max.x,&bbox.max.y,&bbox.max.z,0));
    }
    virtual void beforeRenderFrame()
    {
        if (rebuildANARIWorld) {
            ASG_SAFE_CALL(asgBuildANARIWorld(root,device,world,
                        ASG_BUILD_WORLD_FLAG_FULL_REBUILD & ~ASG_BUILD_WORLD_FLAG_LIGHTS,0));

            anariCommit(device,world);
        }

        rebuildANARIWorld = false;
    }
    bool needFrameReset()
    {
        return rebuildANARIWorld;
    }
    bool rebuildANARIWorld = false;

    virtual visionaray::aabb getBounds()
    {
        return bbox;
    }

    visionaray::aabb bbox;

    Island island;
    Ship ship;
    Sky sky;
};
