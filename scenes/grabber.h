#pragma once

#include <asg/asg.hpp>
#include "../scenes.h"
#include <visionaray/math/vector.h>
struct Gameboard;
struct Grabber;
namespace grabber
{
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

    asg::SphereGeometry makeSphere(float radius)
    {
        float* sphVertex = new float[3] { 0,0,0};
        float* sphRadius = new float[1] { radius };

        asg::SphereGeometry sph = asg::newSphereGeometry(
                sphVertex,sphRadius,nullptr,1,nullptr,0);

        return sph;
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
        copper->setParam(asg::Param("kd", .8f, .6f, .4f));
        copper->setParam(asg::Param("ks", 1.f, .6f, .6f));
        asg::Surface piece = asg::newSurface(cylinder, copper);
        asg::Select piece_select = asg::newSelect(ASG_TRUE);
        m_sceneGraph->addChild(piece_select);
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
                        piece_select->addChild(trans);

                        piece_ids[index] = piece_select->getChildren() - 1;
                    }
                    else
                    {
                        piece_ids[index] = -1;
                    }

                    if (index == 2)
                    {
                        asg::CylinderGeometry selector_cylinder = grabber::makeCylinder(s_pieceHeight,
                                                                                        s_pieceRadius + 0.1f);
                        asg::Material selector = asg::newMaterial("matte");
                        selector->setParam(asg::Param("kd", 1.0f, 0.0f, 0.0f));
                        m_selector_surface = asg::newSurface(selector_cylinder, selector);

                        asg::Transform selector_trans = asg::newTransform(I);
                        selector_trans->translate(getPositionOfPiece(index).data());
                        float trr[] = {0.f, (s_squareHeight + s_pieceHeight) / 2.f, 0.f};
                        selector_trans->translate(trr);
                        selector_trans->addChild(m_selector_surface);

                        m_sceneGraph->addChild(selector_trans);
                        m_selector = selector_trans;
                        m_selector_index = 2;
                    }
                }
                else
                {
                    piece_ids[index] = -1;
                }
                index++;
            }
        }
    }
    asg::Object getPieceAtPosition(visionaray::vec2i pos)
    {

        for (int i = 0; i < sizeof(m_squares) / sizeof(Gameboard::SquareState); i++)
        {
            if (m_squares[i] == OccupiedField)
            {
                std::cout << getPositionOfPiece(i).data();
            }
        }

        // if(pos.x >= piece_pos.x && pos.x <= piece_pos.x + s_pieceRadius
        //   && pos.y >= piece_pos.y && pos.y <= piece_pos.y + s_pieceHeight)

        return m_sceneGraph->getChild(0);
        // else return nullptr;
    }
    asg::Object getPiece()
    {
        return removePiece();
    }
    asg::Object removePiece()
    {

        // 1. check if the square is occupied
        // 2. remove the piece from the scenegraph
        // 3. mark the square as empty

        if (m_squares[m_selector_index] == OccupiedField)
        {
            asg::Select child = std::static_pointer_cast<asg::detail::Select>(m_sceneGraph->getChild(1));
            child->setChildVisible(piece_ids[m_selector_index], ASG_FALSE);
            asg::Object piece = child->getChild(piece_ids[m_selector_index]);
            // this piece possibly gets handed over to the grabber, so keep
            // a reference to it
            // piece->ref();

            m_squares[m_selector_index] = Gameboard::SquareState::EmptyField;

            return piece;
        }

        return NULL;
    }
    visionaray::vec3f getPositionOfPiece(int index)
    {
        int i = index / 7;
        int j = index % 7;

        visionaray::vec3f position((i+3)*s_squareWidth, 0.0, (j+3)*s_squareWidth);


        // compute the vector position of the upper face of the piece
        return position;
    }
    void attachGrabber(Grabber *grabber)
    {
        m_grabber = grabber;
    }

    // the edge length of one gameboard square
    constexpr static float s_squareWidth = 1.f;
    // the height of one gameboard square 
    constexpr static float s_squareHeight = .1f;
    // the radius of one piece
    constexpr static float s_pieceRadius = .35f;
    // the height of one piece
    constexpr static float s_pieceHeight = .6f;
    int m_selector_index = 0;

    enum SquareState
    {
        InvalidField = -1,
        EmptyField,
        OccupiedField
    };
    SquareState m_squares[7 * 7];

    enum GameboardState
    {
        NO_PIECE_PICKED,
        PIECE_PICKED
    };
    GameboardState m_state;
    //!  NO_PIECE_PICKED = no piece has been picked up, so we should pick one
    //!  PIECE_PICKED = a piece was previously picked up and should now be dropped down

    asg::Transform m_selector = nullptr;
    asg::Surface m_selector_surface = nullptr;
    asg::Object m_sceneGraph = nullptr;
    Grabber *m_grabber;
    int piece_ids[7 * 7];
};

// ::grabber
struct Grabber
{
    Grabber()
    {
        initSceneGraph();
    }

    void initSceneGraph()
    {
        double hand_length =
            s_shoulderHeight / 2 + s_upperArmLength + s_elbowRadius - s_wristRadius - s_fingerLength - Gameboard::s_pieceHeight - Gameboard::s_squareHeight / 2;
        asg::Object grabber = asg::newObject();

        asg::Transform trans = nullptr;

        // the part of the grabber starting with its shoulder to its finger
        asg::Object fromShoulder = asg::newObject();

        asg::Object shoulder = asg::newObject();
        static float vals[12][3];
        for (int i = 0; i < 6; i++)
        {
            vals[i][0] = vals[i + 6][0] = sin(-2. * M_PI / 6 * i) * s_shoulderWidth * .5;
            vals[i][2] = vals[i + 6][2] = cos(-2. * M_PI / 6 * i) * s_shoulderWidth * .5;
            vals[i][1] = -s_shoulderHeight;
            vals[i + 6][1] = s_shoulderHeight;
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
        float I[] = {1.f, 0.f, 0.f, 0.f,
                     0.f, 1.f, 0.f, 0.f,
                     0.f, 0.f, 1.f, 0.f,
                     0.f, 0.f, 0.f, 1.f};
        asg::TriangleGeometry faceset = asg::newTriangleGeometry(
            (float *)vals, NULL, NULL, 12, ind, sizeof(ind) / sizeof(ind[0]) / 3, NULL);
        asg::Material dfltColor = asg::newMaterial("matte");
        dfltColor->setParam(asg::Param("kd", .80f, .8f, .8f));
        shoulder->addChild(asg::newSurface(faceset, dfltColor));
        if (m_shoulderRot != nullptr)
            m_shoulderRot->release();
        m_shoulderRot = grabber::makeRotation(1, m_startShoulderRot);
        m_shoulderRot_angle = m_startShoulderRot;
        m_shoulderRot_axis = 1;
        fromShoulder->addChild(shoulder);
        grabber->addChild(fromShoulder);

        // the part of the grabber starting with its upper arm to its finger
        asg::Object from_upper_arm = asg::newObject();
        trans = grabber::makeTransform(0, 0.f, 0.f,
                                       (s_shoulderHeight + s_upperArmLength) / 2, 0.f);

        asg::TriangleGeometry upper_arm = grabber::makeCube(s_upperArmWidth,
                                                            s_upperArmLength,
                                                            s_upperArmWidth);
        trans->addChild(asg::newSurface(upper_arm, dfltColor));
        from_upper_arm->addChild(trans);
        fromShoulder->getChild(0)->addChild(from_upper_arm);

        // the part of the grabber starting with its elbow to its finger
        asg::Object from_elbow = asg::newObject();
        asg::Transform rot = grabber::makeRotation(2, M_PI / 2.f);
        trans = grabber::makeTransform(2, M_PI / 2.f, 0, s_upperArmLength / 2.f + s_elbowRadius, 0.f);
        asg::CylinderGeometry elbow = grabber::makeCylinder(s_elbowWidth, s_elbowRadius);
        trans->addChild(asg::newSurface(elbow, dfltColor));
        // m_azimuth = grabber::makeRotation(1,m_startAzimuth);
        // trans->addChild(m_azimuth);
        from_elbow->addChild(trans);

        m_azimuth_axis = 1;
        m_azimuth_angle = m_startAzimuth;
        // m_azimuth->axis = SoRotationXYZ::Y;
        // m_azimuth->angle = m_startAzimuth;
        // from_elbow->addChild(m_azimuth);
        from_upper_arm->getChild(0)->addChild(from_elbow);

        // the part of the grabber starting with its under arm to its finger
        asg::Object from_under_arm = asg::newObject();
        trans = grabber::makeTransform(0, 0, 0.f, s_elbowRadius + s_underArmLength / 2.f + (m_startRadialDist - s_underArmLength + s_elbowRadius + s_wristRadius) / 2, 0.f);
        asg::CylinderGeometry under_arm_piece = grabber::makeCylinder(s_underArmLength, s_underArmRadius1);
        trans->addChild(asg::newSurface(under_arm_piece, dfltColor));
        from_under_arm->addChild(trans);

        from_elbow->getChild(0)->addChild(from_under_arm);

        m_from_under_arm = from_under_arm;
    }

    void attachGameboard(Gameboard *gameboard)
    {
        m_gameboard = gameboard;
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
    constexpr static float s_epsilon = 1e-4;
    constexpr static float s_rampStep = 0.01;
    float m_startAzimuth = M_PI / 32.f;
    float m_startShoulderRot = 0.f;
    float m_startRadialDist = 0.f;
    float m_rampStep = 0.01f;
    float m_ramp = 0.0f;
    float m_endAzimuth;
    float m_endRadialDist;
    float m_endShoulderRot;
    float m_radCalc_a = s_underArmLength + s_elbowRadius + s_wristRadius;
    float m_radCalc_b = m_startRadialDist;
    int m_azimuth_axis;
    float m_azimuth_angle;
    float m_shoulderRot_angle;
    int m_shoulderRot_axis;
    asg::Object m_sceneGraph = nullptr;
    asg::Object m_finger = nullptr;
    asg::Transform m_shoulderRot = nullptr;
    asg::Object m_from_under_arm = nullptr;
    asg::Transform m_azimuth = nullptr;

    enum Mode
    {
        INACTIVE,
        GET_PIECE,
        SET_PIECE
    };
    Mode m_mode = INACTIVE;

    enum AnimationStep
    {
        NO_ANIMATION,
        MOVE_GRABBER,
        DOWN_GRABBER,
        UP_GRABBER
    };
    AnimationStep m_animationStep = NO_ANIMATION;
    Gameboard *m_gameboard;
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
        ASG_SAFE_CALL(asgBuildANARIWorld(root, device, world,
                                         ASG_BUILD_WORLD_FLAG_FULL_REBUILD, 0));

        // Compute bounding box
        ASG_SAFE_CALL(asgComputeBounds(root, &bbox.min.x, &bbox.min.y, &bbox.min.z,
                                       &bbox.max.x, &bbox.max.y, &bbox.max.z, 0));
    }
    virtual bool handleKeyPress(visionaray::key_event const &event)
    {
        if (event.key() == visionaray::keyboard::ArrowLeft)
        {
            do
            {
                gb.m_selector_index--;
            } while (gb.m_squares[gb.m_selector_index] != Gameboard::SquareState::OccupiedField);
            if (gb.m_selector_index < 2)
                gb.m_selector_index = 2;
            // move selected piece
            static float I[] = {1.f, 0.f, 0.f,
                                0.f, 1.f, 0.f,
                                0.f, 0.f, 1.f,
                                0.f, 0.f, 0.f};

            asg::Transform trans = asg::newTransform(I);

            trans->translate(gb.getPositionOfPiece(gb.m_selector_index).data());
            float tr[] = {0.f, (gb.s_squareHeight + gb.s_pieceHeight) / 2.f, 0.f};
            trans->translate(tr);
            trans->addChild(gb.m_selector_surface);
            gb.m_sceneGraph->removeChild(gb.m_selector);
            gb.m_selector->release();

            gb.m_sceneGraph->addChild(trans);
            gb.m_selector = trans;

            rebuildANARIWorld = true;
        }
        if (event.key() == visionaray::keyboard::ArrowRight)
        {
            do
            {
                gb.m_selector_index++;
            } while (gb.m_squares[gb.m_selector_index] != Gameboard::SquareState::OccupiedField);
            if (gb.m_selector_index > 30)
                gb.m_selector_index = 30;
            // move selector to next piece

            static float I[] = {1.f, 0.f, 0.f,
                                0.f, 1.f, 0.f,
                                0.f, 0.f, 1.f,
                                0.f, 0.f, 0.f};

            asg::Transform trans = asg::newTransform(I);

            trans->translate(gb.getPositionOfPiece(gb.m_selector_index).data());
            float tr[] = {0.f, (gb.s_squareHeight + gb.s_pieceHeight) / 2.f, 0.f};
            trans->translate(tr);
            trans->addChild(gb.m_selector_surface);
            gb.m_sceneGraph->removeChild(gb.m_selector);
            gb.m_selector->release();

            gb.m_sceneGraph->addChild(trans);
            gb.m_selector = trans;

            rebuildANARIWorld = true;
        }
        if (event.key() == visionaray::keyboard::Enter)
        {
            // remove selected piece
            asg::Select child = std::static_pointer_cast<asg::detail::Select>(gb.m_sceneGraph->getChild(1));
            child->setChildVisible(gb.piece_ids[gb.m_selector_index], ASG_FALSE);
            gb.m_squares[gb.m_selector_index] = Gameboard::SquareState::EmptyField;
            do
            {
                gb.m_selector_index++;
            } while (gb.m_squares[gb.m_selector_index] != Gameboard::SquareState::OccupiedField);

            if (gb.m_selector_index > 30)
                gb.m_selector_index = 30;

            static float I[] = {1.f, 0.f, 0.f,
                                0.f, 1.f, 0.f,
                                0.f, 0.f, 1.f,
                                0.f, 0.f, 0.f};

            asg::Transform trans = asg::newTransform(I);

            trans->translate(gb.getPositionOfPiece(gb.m_selector_index).data());
            float tr[] = {0.f, (gb.s_squareHeight + gb.s_pieceHeight) / 2.f, 0.f};
            trans->translate(tr);
            trans->addChild(gb.m_selector_surface);
            gb.m_sceneGraph->removeChild(gb.m_selector);
            gb.m_selector->release();
            // tell grabber to start animation for grabbing the piece, not implemented yet
            // gr.getPiece(gb.getPositionOfPiece(gb.m_selector_index));

            gb.m_sceneGraph->addChild(trans);
            gb.m_selector = trans;

            rebuildANARIWorld = true;
        }
    }
    virtual void beforeRenderFrame()
    {
        if (rebuildANARIWorld)
        {
            ASG_SAFE_CALL(asgBuildANARIWorld(root, device, world,
                                             ASG_BUILD_WORLD_FLAG_FULL_REBUILD & ~ASG_BUILD_WORLD_FLAG_LIGHTS, 0));

            anariCommit(device, world);
        }

        rebuildANARIWorld = false;
    }
    bool needFrameReset()
    {
        return rebuildANARIWorld;
    }

    virtual visionaray::aabb getBounds()
    {
        return bbox;
    }

    Grabber gr;
    Gameboard gb;

    visionaray::aabb bbox;
    bool rebuildANARIWorld = false;
};


