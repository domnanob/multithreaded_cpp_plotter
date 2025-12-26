#include <iostream>
#include <mutex>
#include <algorithm>
#include <condition_variable>
#include <vector>
#include <ranges>
#include <iterator>
#include "test_scene.h"

using namespace std;

float eval2(float x) {
    return Math::sqrt(x * x * x * x) + Math::sqrt(x * x) + Math::sqrt(x * x) - 10.0;
}
float eval(float x) {
    float d = x;
    for (int i = 0; i < 1000000; ++i) {
        d += sqrt(x * x * x * x * x * x);
    }
    d /= 1000000.0;
    return eval2(x * x) + eval2(x) + d;
}

float calc_min_y(vector<float> &v) {
    if(!v.empty()) {
        float min = numeric_limits<float>::max();
        for (float f : v) {
            if(f < min) {
                min = f;
            }
        }
        return min;
    }
    return 0.0f;
}
float calc_max_y(vector<float> &v) {
    if(!v.empty()) {
        float max = numeric_limits<float>::min();
        for (float f : v) {
            if(max < f) {
                max = f;
            }
        }
        return max;
    }
    return 0.0f;
}

void TestScene::render() {
#pragma region setup
    Renderer::get_singleton()->clear_screen(Color());
    Renderer::get_singleton()->camera_2d_projection_set_to_window();
    Renderer *r = Renderer::get_singleton();
    const Vector2i windowSize = Renderer::get_singleton()->get_window_size();
#pragma endregion
#pragma region draw_x
    r->draw_text_2d(String::num(_min_x_value), _font, Vector2(20, windowSize.y/2 - 20), Color(1, 0, 0));
    r->draw_text_2d(String::num(_max_x_value), _font, Vector2(windowSize.x- _font->get_string_size(String::num(_max_x_value)).length(), windowSize.y/2 - 20), Color(1, 0, 0));
    r->draw_text_2d("X", _font, Vector2(windowSize.x - 20, windowSize.y/2 + 10), Color(1, 0, 0));
    r->draw_line(Vector2(10, windowSize.y/2), Vector2(windowSize.x, windowSize.y/2), Color(0, 0,1),3);
#pragma endregion
#pragma region draw_y
    float min_y = calc_min_y(results);
    float max_y = calc_max_y(results);
    r->draw_text_2d(String::num(min_y), _font, Vector2(20, windowSize.y-20), Color(1, 0, 0));
    r->draw_text_2d(String::num(max_y), _font, Vector2(20, 10), Color(1, 0, 0));
    r->draw_line(Vector2(10, 5), Vector2(10, windowSize.x), Color(0, 1, 0), 3);
#pragma endregion
#pragma region draw_points
    for (size_t i = 0; i < results.size(); ++i) {
        float x = i * windowSize.x / results.size() + 10;
        float y = windowSize.y - (results[i] - min_y) / (max_y - min_y) * windowSize.y;
        r->draw_point(Vector2(x, y), Color(255, 255, 255));
    }
#pragma endregion
#pragma region draw_gui
    GUI::new_frame();
    if (!_is_running) {
        ImGui::Begin("Plotter");
        ImGui::Text("Thread settings:");
        ImGui::SliderInt(" = Thread Count", &_thread_value, 1, 200);
        ImGui::Text("X:");
        ImGui::SliderFloat(" = Min X", &_min_x_value, -300, 300);
        ImGui::SliderFloat(" = Max X", &_max_x_value, -300, 300);
        ImGui::SliderFloat(" = Step X", &_step_value, 0.1, 300);
        if(ImGui::Button("Start")) {
            start_threads();
            _is_running = true;
        }
        ImGui::End();
    } else {
        ImGui::Begin("Working...");
        ImGui::Text("Thread settings:");
        ImGui::Text("Thread Count: %d", _thread_value);
        ImGui::Text("X:");
        ImGui::Text("Min X: %f", _min_x_value);
        ImGui::Text("Max X: %f", _max_x_value);
        ImGui::Text("Step X: %f", _step_value);
        ImGui::End();
    }
    GUI::render();
    if (_is_running) {
        lock_guard<mutex> result_lock(result_mutex);
        if (current_thread_id >= _thread_value) {
            _is_running = false;
        }
    }
#pragma endregion
}


TestScene::TestScene() {
    Renderer::initialize();
    GUI::initialize();
    _min_x_value = -250;
    _max_x_value = 250;
    _step_value = 0.100;
    _thread_value = 5;
    _float_slider_value = 10;
    _int_slider_value = 5;
    _test_label_shown = false;

    _font.instance();
    _font->load_default(13.0);

    _sfw_image.instance();
    _sfw_image->load_from_file("icon.png");

    _sfw_texture.instance();
    _sfw_texture->create_from_image(_sfw_image);
}
TestScene::~TestScene() {
    Renderer::destroy();
    GUI::destroy();
    for (int i = 0; i < _threads.size(); ++i) {
        ThreadContext *tc = _threads[i];
        tc->stop();
        memdelete(tc);
    }
    _threads.clear();
}

void TestScene::calculate_values(float min_x, float max_x, float step, int thread_id, int total_threads) {
    unique_lock<mutex> lock(thread_mutex);
    thread_cv.wait(lock, [thread_id, this]() { return thread_id == current_thread_id; });
    float range = (max_x - min_x) / total_threads;
    float start_x = min_x + thread_id * range;
    float end_x = (thread_id == total_threads - 1) ? max_x : start_x + range;
    for (float x = start_x; x <= end_x; x += step) {
        float y = eval(x);
        lock_guard<mutex> result_lock(result_mutex);
        results.push_back(y);
    }
    current_thread_id++;
    thread_cv.notify_all();
}

void TestScene::_thread_func(void *p_user_data) {
    const auto *tc = static_cast<ThreadContext *>(p_user_data);
    tc->scene->calculate_values(tc->scene->_min_x_value, tc->scene->_max_x_value, tc->scene->_step_value, tc->id, tc->scene->_thread_value);
}

void TestScene::start_threads() {
    rerun_handler();
    //11 == 5
    int for_end = (_thread_value % 2 == 0)?  _thread_value/2 - 1 :  _thread_value/2;
    for(int i = 0; i <= for_end; ++i) {
        create_thread(i); // 0, 1, 2, 3, 4, 5
        create_thread(_thread_value-i); // 11, 10, 9, 8, 7, 6
    }
    if(_thread_value%2 == 0) {
        create_thread(for_end+1);
    }
}
void TestScene::create_thread(int id) {
    ThreadContext *tc = memnew(ThreadContext);
    tc->init();
    tc->id = id;
    tc->scene = this;
    _threads.push_back(tc);
    tc->thread->start(_thread_func, tc);
}

void TestScene::rerun_handler() {
    lock_guard<mutex> lck(result_mutex);
    results.clear();
    stop_all_threads();
    current_thread_id = 0;
    _threads.clear();
}
void TestScene::stop_all_threads() {
    for (int i = 0; i < _threads.size(); ++i) {
        ThreadContext *tc = _threads[i];
        if (tc->running && !tc->thread->is_main_thread()) {
            tc->stop();
            memdelete(tc);
        }
    }
}