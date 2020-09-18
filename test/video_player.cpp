//
// Created by kier on 2020/9/15.
//

#include "ohm/state_model.h"
#include "ohm/thread_state_model.h"
#include "ohm/print.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

class MouseCallback {
public:
    using self = MouseCallback;

    using F = std::function<void(int event, int x, int y, int flag)>;

    explicit MouseCallback(F f)
        : m_f(std::move(f)) {}

    static void C(int event, int x, int y, int flag, void *data) {
        auto obj = reinterpret_cast<self*>(data);
        obj->m_f(event, x, y, flag);
    }

    void setup(const std::string &name) {
        cv::setMouseCallback(name, &C, this);
    }

private:
    mutable F m_f;
};

int main() {
    using namespace ohm;

    std::string title = "Video";
    std::string video = "circle.mp4";

    StateModel model;

    cv::VideoCapture capture;

    enum VIDEO_STATE {
        STATE_STAY = 1,
        STATE_PLAY = 2,
        STATE_PAUSE = 3,
    };
    enum VIDEO_EVENT {
        EVENT_OPEN = 1,
        EVENT_PLAY = 2,
        EVENT_STOP = 3,
        EVENT_CLOSE = 4,
        EVENT_JUMP = 5,
    };

    Event<int> jump_to(EVENT_JUMP);

    cv::Mat cover;
    cv::Mat snap;
    int anchor = 0;
    int total = 0;

    model.transit(STATE_START, EVENT_OPEN, [&]() -> StateCode {
        capture.open(video);
        if (!capture.isOpened()) return STATE_END;
        total = int(capture.get(cv::CAP_PROP_FRAME_COUNT));
        return STATE_STAY;
    });

    model.enter(STATE_STAY, [&]() {
        capture.set(cv::CAP_PROP_POS_FRAMES, 0);
        capture.grab();
        capture.retrieve(cover);
        anchor = 0;
    });
    model.transit(STATE_STAY, EVENT_PLAY, STATE_PLAY);
    model.transit(STATE_STAY, EVENT_CLOSE, STATE_END);
    model.transit(STATE_STAY, jump_to, [&](int frame) -> StateCode {
        capture.set(cv::CAP_PROP_POS_FRAMES, frame);
        capture.grab();
        capture.retrieve(cover);
        anchor = frame;
        return STATE_STILL;
    });

    model.transit(STATE_PLAY, EVENT_PLAY, STATE_PAUSE, [&]() {
        capture.retrieve(snap);
    });
    model.transit(STATE_PLAY, EVENT_STOP, STATE_STAY);
    model.transit(STATE_PLAY, EVENT_CLOSE, STATE_END);
    model.transit(STATE_PLAY, jump_to, [&](int frame) -> StateCode {
        capture.set(cv::CAP_PROP_POS_FRAMES, frame);
        anchor = frame;
        return STATE_STILL;
    });

    // model.init(STATE_PAUSE, [&]() { capture.retrieve(snap); }); // done in play's transit

    model.transit(STATE_PAUSE, EVENT_PLAY, STATE_PLAY);
    model.transit(STATE_PAUSE, EVENT_STOP, STATE_STAY);
    model.transit(STATE_PAUSE, EVENT_CLOSE, STATE_END);
    model.transit(STATE_PAUSE, jump_to, [&](int frame) -> StateCode {
        capture.set(cv::CAP_PROP_POS_FRAMES, frame);
        capture.grab();
        capture.retrieve(snap);
        anchor = frame;
        return STATE_STILL;
    });

    model.event(EVENT_OPEN);

    // show bar
    int bar_height = 5;
    int bar_top = 20;
    int bar_left = 10;
    int bar_right = 10;
    int bar_width = int(capture.get(cv::CAP_PROP_FRAME_WIDTH)) - bar_left - bar_right;

    MouseCallback mouse([&](int event, int x, int y, int flag) {
        if (event == cv::EVENT_LBUTTONDOWN) {
            if (x < bar_left || x > bar_width + bar_left) return;
            if (y < bar_top || y > bar_top + bar_height) return;
            auto progress = float(x - bar_left) / bar_width;
            auto want = int(progress * total);
            if (want < 0) want = 0;
            if (want >= total) want = total - 1;
            model.event(jump_to, want);
        }
    });
    cv::namedWindow(title);
    mouse.setup(title);

    cv::Mat frame;
    bool end = false;

    while (true) {
        switch (model.state()) {
            case STATE_STAY:
            {
                frame = cover;
                break;
            }
            case STATE_PLAY:
            {
                if (!capture.grab()) {
                    model.event(EVENT_STOP);
                    continue;
                }
                capture.retrieve(frame);
                ++anchor;
                break;
            }
            case STATE_PAUSE:
            {
                frame = snap;
                break;
            }
            case STATE_END:
            {
                end = true;
                break;
            }
        }
        if (end) break;

        cv::Point top_left(bar_left, bar_top);
        cv::Point bottom_right(frame.cols - bar_right, bar_top + bar_height);
        cv::rectangle(frame, top_left, bottom_right,
                      CV_RGB(255, 255, 255), 1);

        float progress = float(anchor) / total;
        cv::Point progress_top_left(bar_left, bar_top);
        cv::Point progress_bottom_right(bar_left + (bar_width * progress), bar_top + bar_height);
        cv::rectangle(frame, progress_top_left, progress_bottom_right,
                      CV_RGB(255, 255, 255), -1);

        cv::imshow(title, frame);
        auto key = cv::waitKey(30);
        switch (key) {
            default:
                break;
            case 27:
                model.event(EVENT_CLOSE);
                break;
            case ' ':
                model.event(EVENT_PLAY);
                break;
            case 's':
                model.event(EVENT_STOP);
                break;
        }
    }
};

