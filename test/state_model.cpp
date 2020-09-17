//
// Created by kier on 2020/9/15.
//

#include "ohm/state_model.h"
#include "ohm/thread_state_model.h"
#include "ohm/print.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

int main() {
    using namespace ohm;

    std::string video = "circle.mp4";

    StateModel model;

    cv::VideoCapture capture;

    // state: 1-待机， 2-播放，3-暂停
    // action: 1-开机，2-播放/暂停，3-停止
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
    };

    cv::Mat cover;
    cv::Mat snap;

    model.transit(STATE_START, EVENT_OPEN, [&]() -> StateCode {
        capture.open(video);
        if (!capture.isOpened()) return STATE_END;
        return STATE_STAY;
    });

    model.enter(STATE_STAY, [&]() {
        capture.set(cv::CAP_PROP_POS_FRAMES, 0);
        capture.grab();
        capture.retrieve(cover);
    });
    model.transit(STATE_STAY, EVENT_PLAY, STATE_PLAY);
    model.transit(STATE_STAY, EVENT_CLOSE, STATE_END);

    model.transit(STATE_PLAY, EVENT_PLAY, STATE_PAUSE, [&]() {
        capture.retrieve(snap);
    });
    model.transit(STATE_PLAY, EVENT_STOP, STATE_STAY);
    model.transit(STATE_PLAY, EVENT_CLOSE, STATE_END);

    // model.init(STATE_PAUSE, [&]() { capture.retrieve(snap); }); // done in play's transit

    model.transit(STATE_PAUSE, EVENT_PLAY, STATE_PLAY);
    model.transit(STATE_PAUSE, EVENT_STOP, STATE_STAY);
    model.transit(STATE_PAUSE, EVENT_CLOSE, STATE_END);

    model.event(EVENT_OPEN);

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

        cv::imshow("Video", frame);
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

