#pragma once

#include <asg/asg.hpp>
#include "../scenes.h"

namespace grabber
{
    asg::TriangleGeometry makeCube(float width, float height, float depth)
    {
        float w2 = width*.5f;
        float h2 = height*.5f;
        float d2 = depth*.5f;
        float* cubeVertex = new float[] {-w2,-h2,-d2,
                                          w2,-h2,-d2,
                                          w2, h2,-d2,
                                         -w2, h2,-d2,
                                          w2,-h2, d2,
                                         -w2,-h2, d2,
                                         -w2, h2, d2,
                                          w2, h2, d2};

        uint32_t* cubeIndex = new uint32_t[] {0,1,2, 0,2,3, 4,5,6, 4,6,7,
                                              1,4,7, 1,7,2, 5,0,3, 5,3,6,
                                              5,4,1, 5,1,0, 3,2,7, 3,7,6};
        asg::TriangleGeometry cube = asg::newTriangleGeometry(
                cubeVertex,NULL,NULL,8,cubeIndex,12,NULL,NULL,NULL,NULL);

        return cube;
    }

    asg::CylinderGeometry makeCylinder(float height, float radius)
    {
        float* cylVertex = new float[] { 0.f,-height/2.f,0.f,
                                         0.f,height/2.f,0.f };
        float* cylRadius = new float[] { radius };
        uint8_t* cylCap = new uint8_t[] { 1 };
        asg::CylinderGeometry cyl = asg::newCylinderGeometry(
                cylVertex,cylRadius,nullptr,cylCap,2,nullptr,0);

        return cyl;
    }

    asg::Transform makeRotation(int axis, float angle)
    {
        float cosa = cosf(angle);
        float sina = sinf(angle);

        if (axis == 0) {
            float R[] = {1.f,0.f,0.f,
                         0.f,cosa,sina,
                         0.f,-sina,cosa,
                         0.f,0.f,0.f};
            return asg::newTransform(R);
        } else if (axis == 1) {
            float R[] = {cosa,0.f,sina,
                         0.f,1.f,0.f,
                         -sina,0.f,cosa,
                         0.f,0.f,0.f};
            return asg::newTransform(R);
        } else { assert(axis == 2);
            float R[] = {cosa,sina,0.f,
                         -sina,cosa,0.f,
                         0.f,0.f,1.f,
                         0.f,0.f,0.f};
            return asg::newTransform(R);
        }
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
} // ::grabber

struct Grabber
{
    Grabber()
    {
        initSceneGraph();
    }

    void initSceneGraph()
    {
        asg::Object grabber = asg::newObject();

        asg::Transform trans = nullptr;

        // the part of the grabber starting with its shoulder to its finger
        asg::Object fromShoulder = asg::newObject();

        asg::Object shoulder = asg::newObject();
        static float vals[12][3];
        for(int i=0; i<6; i++)
        {
            vals[i][0] = vals[i+6][0] = sin(-2.*M_PI/6*i)*s_shoulderWidth*.5;
            vals[i][2] = vals[i+6][2] = cos(-2.*M_PI/6*i)*s_shoulderWidth*.5;
            vals[i][1] = -s_shoulderHeight;
            vals[i+6][1] = s_shoulderHeight;
        }
        static uint32_t ind[] = {
            0,1,2, 0,2,3, 0,3,4, 0,4,5,
            0,6,7, 0,7,1,
            1,7,8, 1,8,2,
            2,8,9, 2,9,3,
            3,9,10, 3,10,4,
            4,10,11, 4,11,5,
            5,11,6, 5,6,0,
            11,10,9, 11,9,8, 11,8,7, 11,7,6,
        };
        asg::TriangleGeometry faceset = asg::newTriangleGeometry(
                (float*)vals,NULL,NULL,12,ind,sizeof(ind)/sizeof(ind[0])/3,NULL);
        asg::Material dfltColor = asg::newMaterial("matte");
        dfltColor->setParam(asg::Param("kd",.80f,.8f,.8f));
        shoulder->addChild(asg::newSurface(faceset,dfltColor));
        if (m_shoulderRot != nullptr)
            m_shoulderRot->release();
        m_shoulderRot = grabber::makeRotation(1,m_startShoulderRot);
        fromShoulder->addChild(shoulder);
        grabber->addChild(fromShoulder);

        // the part of the grabber starting with its upper arm to its finger
        asg::Object from_upper_arm = asg::newObject();
        trans = grabber::makeTransform(0,0.f,0.f,
                                       (s_shoulderHeight+s_upperArmLength)/2,0.f);
        from_upper_arm->addChild(trans);
        asg::TriangleGeometry upper_arm = grabber::makeCube(s_upperArmWidth,
                                                            s_upperArmLength,
                                                            s_upperArmWidth);
        trans->addChild(asg::newSurface(upper_arm,dfltColor));
        fromShoulder->addChild(from_upper_arm);

        // the part of the grabber starting with its elbow to its finger
        asg::Object from_elbow = asg::newObject();
        trans = grabber::makeTransform(2,M_PI/2.f,
                                       0.f,s_upperArmLength/2.f+s_elbowRadius,0.f);
        from_elbow->addChild(trans);
        asg::CylinderGeometry elbow = grabber::makeCylinder(s_elbowWidth,s_elbowRadius);
        trans->addChild(asg::newSurface(elbow,dfltColor));
        m_azimuth = grabber::makeRotation(1,m_startAzimuth);
        trans->addChild(m_azimuth);
        // m_azimuth->axis = SoRotationXYZ::Y;
        // m_azimuth->angle = m_startAzimuth;
        // from_elbow->addChild(m_azimuth);
        from_upper_arm->getChild(0)->addChild(from_elbow);

        // the part of the grabber starting with its under arm to its finger
        asg::Object from_under_arm = asg::newObject();
        trans = grabber::makeTransform(0,M_PI/2.f,
                                       0.f,s_elbowRadius+s_underArmLength/2.f,0.f);
        from_under_arm->addChild(trans);
        asg::CylinderGeometry under_arm_piece = grabber::makeCylinder(
                s_underArmLength,s_underArmRadius1);
        trans->addChild(asg::newSurface(under_arm_piece,dfltColor));
        //...
        //m_azimuth->addChild(from_under_arm);

        printSceneGraph((ASGObject)*grabber,true);
        m_sceneGraph = grabber;
    }

    constexpr static float s_shoulderWidth = 3.f;
    constexpr static float s_shoulderHeight = .7f;
    constexpr static float s_upperArmLength = 2.4f;
    constexpr static float s_upperArmWidth = 1.f;
    constexpr static float s_elbowWidth = 1.4f;
    constexpr static float s_elbowRadius = .7f;
    constexpr static float s_underArmLength = 4.f;
    constexpr static float s_underArmRadius1 = .4f;
    constexpr static float s_underArmRadius2 = .3f;
    constexpr static float s_underArmRadius3 = .2f;
    constexpr static float s_wristRadius = .3f;
    constexpr static float s_handRadius = .25f;
    constexpr static float s_fingerLength = .3f;
    constexpr static float s_fingerRadius = .4f;

    float m_startAzimuth = M_PI/32.f;
    float m_startShoulderRot = 0.f;

    asg::Object m_sceneGraph = nullptr;

    asg::Transform m_shoulderRot = nullptr;

    asg::Transform m_azimuth = nullptr;
};

struct Gameboard
{
    Gameboard()
    {
        initGameboard();
    }

    void initGameboard()
    {
        // init the squares of the gamebord
        int index = 0;

        // for all squares
        for (int i=0; i<7; ++i) {
            for (int j=0; j<7; ++j) {

                if (((i < 2) || (i > 4)) && ((j < 2) || (j > 4))) {
                    // the squares at the corners of the board are not valid
                    m_squares[index] = InvalidField;
                } else {
                    // all other squares are occupied by a piece
                    m_squares[index] = OccupiedField; 
                }

                index++;
            }
        }
        // the middle square (index 24) is empty
        m_squares[24] = EmptyField;

        // now use the m_squares array to init the scenegraph
        initSceneGraph();
    }

    void initSceneGraph()
    {
        if (m_sceneGraph != NULL) // this should never happen!
            m_sceneGraph->release();

        // start with root node of scene graph
        m_sceneGraph = asg::newObject();
        auto obj = asg::newObject();
        m_sceneGraph->addChild(obj);

        // the shape of one square
        asg::TriangleGeometry cube = grabber::makeCube(s_squareWidth,
                                                       s_squareHeight,
                                                       s_squareWidth);
        // colors
        asg::Material greenColor = asg::newMaterial("matte");
        greenColor->setParam(asg::Param("kd",0.f,1.f,0.f));

        asg::Material redColor = asg::newMaterial("matte");
        redColor->setParam(asg::Param("kd",1.f,0.f,0.f));

        // a piece
        static float cylV[] = { 0.f,-s_pieceHeight/2.f,0.f,0.f,s_pieceHeight/2.f,0.f };
        static float cylR[] = { s_pieceRadius };
        static uint8_t cylC[] = { 1 };
        asg::CylinderGeometry cylinder = grabber::makeCylinder(s_pieceHeight,
                                                               s_pieceRadius);
        asg::Material copper = asg::newMaterial("matte");
        copper->setParam(asg::Param("kd",.8f,.6f,.4f));
        copper->setParam(asg::Param("ks",1.f,.6f,.6f));
        asg::Surface piece = asg::newSurface(cylinder,copper);

        // build the scene graph
        int index = 0;
        for (int i=0; i<7; ++i) {
            for (int j=0; j<7; ++j) {
                if (m_squares[index] != InvalidField) {
                    static float I[] = {1.f,0.f,0.f,
                                        0.f,1.f,0.f,
                                        0.f,0.f,1.f,
                                        0.f,0.f,0.f};
                    asg::Transform square = asg::newTransform(I);
                    if (index % 2)
                        square->addChild(asg::newSurface(cube,greenColor));
                    else
                        square->addChild(asg::newSurface(cube,redColor));
                    square->translate(getPositionOfPiece(index).data());
                    m_sceneGraph->addChild(square);
                    asg::Transform trans = asg::newTransform(I);
                    trans->translate(getPositionOfPiece(index).data());
                    float tr[] = {0.f,(s_squareHeight+s_pieceHeight)/2.f,0.f};
                    trans->translate(tr);
                    if (m_squares[index] == OccupiedField) {
                        trans->addChild(piece);
                        m_sceneGraph->addChild(trans);
                    }
                }
                index++;
            }
        }
    }

    visionaray::vec3f getPositionOfPiece(int index)
    {
        int i = index / 7;
        int j = index % 7;

        visionaray::vec3f position((i+3)*s_squareWidth, 0.0, (j+3)*s_squareWidth);


        // compute the vector position of the upper face of the piece
        return position;
    }

    // the edge length of one gameboard square
    constexpr static float s_squareWidth = 1.f;
    // the height of one gameboard square 
    constexpr static float s_squareHeight = .1f;
    // the radius of one piece
    constexpr static float s_pieceRadius = .35f;
    // the height of one piece
    constexpr static float s_pieceHeight = .6f;

    enum SquareState { InvalidField = -1, EmptyField, OccupiedField };
    SquareState m_squares[7*7];

    asg::Object m_sceneGraph = nullptr;
};

struct GrabberGame : Scene
{
    GrabberGame(ANARIDevice dev, ANARIWorld wrld)
        : Scene(dev,wrld)
    {
        asg::Object scene = asg::newObject();

        scene->addChild(gr.m_sceneGraph);
        scene->addChild(gb.m_sceneGraph);

        root = (ASGObject)*scene;

        // Build up ANARI world
        ASG_SAFE_CALL(asgBuildANARIWorld(root,device,world,
                                         ASG_BUILD_WORLD_FLAG_FULL_REBUILD,0));

        // Compute bounding box
        ASG_SAFE_CALL(asgComputeBounds(root,&bbox.min.x,&bbox.min.y,&bbox.min.z,
                                       &bbox.max.x,&bbox.max.y,&bbox.max.z,0));
    }

    virtual visionaray::aabb getBounds()
    {
        return bbox;
    }

    Grabber gr;
    Gameboard gb;

    visionaray::aabb bbox;
};


