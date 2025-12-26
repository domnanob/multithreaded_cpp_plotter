
#ifndef TEST_SCENE_H
#define TEST_SCENE_H

#include "sfw.h"
#include <mutex>

class TestScene : public Scene {
    SFW_OBJECT(TestScene, Scene);

public:
    void render() override;
    virtual void start_threads();

    void create_thread(int id);

    virtual void calculate_values(float min_x, float max_x, float step, int thread_id, int total_threads);

    virtual void rerun_handler();

    void stop_all_threads();

    TestScene();
    ~TestScene() override;

protected:
    static void _thread_func(void *p_user_data);

    float _float_slider_value;
    int _int_slider_value;
    bool _test_label_shown;
    float _min_x_value;
    float _max_x_value;
    float _step_value;
    int _thread_value;
    bool _is_running = false;
    int current_thread_id = 0;

    std::mutex thread_mutex;
    std::mutex result_mutex;
    std::condition_variable thread_cv;

    std::vector<float> results;
    Ref<Font> _font;
    Ref<Image> _sfw_image;
    Ref<Texture> _sfw_texture;

    struct ThreadContext {
        int id;
        Thread *thread;
        bool running;
        TestScene *scene;

        ThreadContext() {
            id = 0;
            thread = NULL;
            running = false;
            scene = NULL;
        }

        void init() {
            thread = memnew(Thread);
            running = true;
        }

        void stop() {
            running = false;
            thread->wait_to_finish();
            memdelete(thread);
            thread = NULL;
        }
    };

    Vector<ThreadContext *> _threads;
    SafeNumeric<int> _counter;
};


#endif // TEST_SCENE_H

