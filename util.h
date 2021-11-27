#pragma once

#include <iostream>
#include <ostream>
#include <string>
#include <asg/asg.h>

#define ASG_SAFE_CALL(X) X

static std::string getExt(const std::string &fileName)
{
    int pos = fileName.rfind('.');
    if (pos == fileName.npos)
        return "";
    return fileName.substr(pos);
}

static void printSceneGraph(ASGObject rootObj, bool verbose = false)
{
    struct UserData {
        int level;
        bool verbose;
    };
    UserData data;
    data.level = 0;
    data.verbose = verbose;

    ASGVisitor visitor = asgCreateVisitor([](ASGVisitor self, ASGObject obj,
                                             void* userData) {

        UserData* data = (UserData*)userData;

        ASGType_t t;
        ASG_SAFE_CALL(asgGetType(obj,&t));

        const char* nameC_Str = "";
        ASG_SAFE_CALL(asgObjectGetName(obj,&nameC_Str));
        std::string name(nameC_Str);
        std::string nameOut;
        if (!name.empty()) {
            nameOut = ", name: ";
            nameOut.append(name);
            nameOut.append(" ");
        }

        std::string indent;
        for (int i=0; i<data->level; ++i) {
            indent.append("  ");
        }

        switch (t) {
            case ASG_TYPE_TRANSFORM: {
                std::cout << indent << "Transform" << nameOut;
                if (data->verbose) {
                    float matrix[12];
                    ASG_SAFE_CALL(asgTransformGetMatrix((ASGTransform)obj,matrix));
                    std::cout << ", matrix: [0]:" << matrix[0];
                    for (int i=1; i<12; ++i) {
                        std::cout << ", [" << i << "]: " << matrix[i];
                    }
                }
                std::cout << '\n';
                break;
            }

            case ASG_TYPE_CAMERA: {
                std::cout << indent << "Camera" << nameOut;
                std::cout << '\n';
                break;
            }

            case ASG_TYPE_SURFACE: {
                std::cout << indent << "Surface" << nameOut;
                std::cout << '\n';
                break;
            }

            case ASG_TYPE_SELECT: {
                std::cout << indent << "Select" << nameOut;
                std::cout << '\n';
                break;
            }

            default: {
                std::cout << indent << "Object" << nameOut << '\n';
                break;
            }
        }

        data->level++;
        ASG_SAFE_CALL(asgVisitorApply(self,obj));
        data->level--;

    },&data,ASG_VISITOR_TRAVERSAL_TYPE_CHILDREN);
    ASG_SAFE_CALL(asgObjectAccept(rootObj,visitor));
}

