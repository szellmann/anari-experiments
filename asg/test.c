#include <stdio.h>
#define ASG_INCLUDE_IMPL
#include "asg.h"

void printObjectType(ASGObject n, void* userData) {
    ASGType_t t;
    asgGetType(n,&t);
    switch (t)
    {
    case ASG_TYPE_OBJECT: printf("%s\n","ASG_TYPE_OBJECT"); break;
    case ASG_TYPE_SURFACE: printf("%s\n","ASG_TYPE_SURFACE"); break;
    case ASG_TYPE_STRUCTURED_VOLUME: printf("%s\n","ASG_TYPE_STRUCTURED_VOLUME"); break;
    default: break;
    }
}

int main() {
    ASGObject root = asgNewObject();
    float volData[63*63*63];
    ASGStructuredVolume c0 = asgNewStructuredVolume(volData,63,63,63,ASG_DATA_TYPE_FLOAT32,NULL);
    asgObjectAddChild(root,c0);

    asgMakeMarschnerLobb(c0);

    ASGVisitor visitor = asgNewVisitor(printObjectType,NULL);
    asgApplyVisitor(root,visitor,ASG_VISITOR_TRAVERSAL_TYPE_CHILDREN);
}


