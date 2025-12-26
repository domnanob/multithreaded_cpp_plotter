#ifndef TEST_APPLICATION_H
#define TEST_APPLICATION_H

#include "sfw.h"

#include "test_scene.h"

class TestApplication : public Application {
    SFW_OBJECT(TestApplication, Application);

public:
    TestApplication() {
        scene = Ref<Scene>(memnew(TestScene()));
    }

    ~TestApplication() {
        scene.unref();
    }
};

#endif // TEST_APPLICATION_H
