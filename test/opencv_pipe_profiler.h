//
// Created by kier on 2020/10/16.
//

#ifndef OMEGA_OPENCV_PIPE_PROFILER_H
#define OMEGA_OPENCV_PIPE_PROFILER_H

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "ohm/pipe/pipe_profiler.h"

namespace ohm {
    namespace opencv {
        namespace canvas {
            struct Color {
                uint8_t r = 0;
                uint8_t g = 0;
                uint8_t b = 0;

                Color() = default;

                Color(uint8_t r, uint8_t g, uint8_t b)
                        : r(r), g(g), b(b) {}
            };

            namespace {
                Color red = Color(255, 0, 0);
                Color green = Color(0, 255, 0);
                Color blue = Color(0, 0, 255);
            }

            class Cell {
            public:
                virtual int width() const = 0;

                /**
                 *
                 * @param canvas
                 * @param x start x point (left border)
                 * @param y start y point (right border)
                 */
                virtual void paint(cv::Mat &canvas, int x, int y,
                                   Color color = green) = 0;
            };

            class Alignment {
            public:
                void including(const Cell &cell) {
                    if (cell.width() > m_max) {
                        m_max = cell.width();
                    }
                }

                int width() const {
                    return m_max;
                }

            private:
                int m_max = 0;
            };

            class StringCell : public Cell {
            public:
                StringCell(const std::string &text, float font_scale = 0.5f)
                        : m_text(text), m_font_scale(font_scale) {
                }

                int width() const override {
                    const auto LW = int(20 * m_font_scale);
                    return int(m_text.size() * LW);
                }

                void paint(cv::Mat &canvas, int x, int y,
                           Color color) override {
                    const auto LH = int(40 * m_font_scale);
                    cv::putText(canvas, m_text, cv::Point(x, y + int(LH)),
                                0, m_font_scale, CV_RGB(color.r, color.g, color.b));
                }

            private:
                std::string m_text;
                float m_font_scale;
            };
        }
    }
    using namespace opencv::canvas;

    class OpenCanvas {
    public:
        using self = OpenCanvas;

        struct TableCell {
            std::shared_ptr<Cell> cell;
            std::shared_ptr<Alignment> alignment;

            template<typename CELL, typename = Required<std::is_base_of<Cell, CELL>>>
            TableCell(CELL cell, std::shared_ptr<Alignment> alignment = nullptr)
                : cell(std::make_shared<CELL>(std::move(cell)))
                , alignment(std::move(alignment)) {
                    if (this->alignment) {
                        this->alignment->including(*this->cell);
                    }
            }

            TableCell(std::shared_ptr<Cell> cell, std::shared_ptr<Alignment> alignment = nullptr)
                    : cell(std::move(cell))
                    , alignment(std::move(alignment)) {
                if (this->alignment) {
                    this->alignment->including(*this->cell);
                }
            }

            int width() {
                if (alignment) {
                    return alignment->width();
                } else {
                    return cell->width();
                }
            }

            void align() {
                alignment = std::make_shared<Alignment>();
                alignment->including(*cell);
            }
        };

        class Line {
        public:
            std::vector<TableCell> cells;
            int alignment_start = -1; /// -1 means no alignment, other means alignment start line
        };

        OpenCanvas() {
            clear();
        }

        void bind(cv::Mat &canvas) {
            m_canvas = &canvas;
        }

        void set_font_scale(float scale = 0.5f) {
            m_font_scale = scale;
        }

        void clear() {
            m_cursor.x = 0;
            m_cursor.y = 0;
            m_lines.clear();
            m_lines.push_back(Line());
        }

        void putCell(std::shared_ptr<Cell> cell) {
            // check if there is alignment
            std::shared_ptr<Alignment> alignment;
            if (m_lines.size() > 1) {
                auto line = m_lines.back().alignment_start;
                if (line >= 0 && line < m_cursor.y) {
                    alignment = m_lines[line].cells[m_cursor.x].alignment;
                }
            }
            m_lines.back().cells.emplace_back(std::move(cell), alignment);
            m_cursor.x++;
        }

        self &putText(const std::string &text) {
            putCell(std::make_shared<StringCell>(text, m_font_scale));
            return *this;
        }

        self &align() {
            if (m_lines.back().cells.empty()) return *this;
            if (m_lines.back().alignment_start < 0) {
                m_lines.back().alignment_start = m_cursor.y;
            }
            auto &cell = m_lines.back().cells.back();
            if (!cell.alignment) {
                cell.align();
            }
            return *this;
        }

        void enter() {
            auto alignment_start = m_lines.back().alignment_start;
            m_lines.emplace_back(Line());
            m_cursor.x = 0;
            m_cursor.y++;
            m_lines.back().alignment_start = alignment_start;
        };

        void nonalign() {
            m_lines.back().alignment_start = -1;
        }

        void progress() {

        }

        void set_border(int top, int bottom, int left, int right) {
            m_border.top = top;
            m_border.bottom = bottom;
            m_border.left = left;
            m_border.right = right;
        }

        void update() {
            if (!m_canvas) return;
            auto &canvas = *m_canvas;
            // get width
            auto LH = int(40 * m_font_scale);
            auto LW = int(20 * m_font_scale);
            int canvas_height = int(m_border.top + m_border.bottom + LH * m_lines.size());
            int canvas_width = 0;
            for (auto &line : m_lines) {
                int line_width = m_border.left + m_border.right;
                for (auto &cell : line.cells) {
                    line_width += cell.width();
                }
                if (line_width > canvas_width) canvas_width = line_width;
            }
            // paint
            canvas = cv::Mat(canvas_height, canvas_width, CV_8UC3,
                             CV_RGB(m_background.r, m_background.g, m_background.b));
            int y = m_border.top;
            for (auto &line : m_lines) {
                int x = m_border.left;
                for (auto &cell : line.cells) {
                    cell.cell->paint(canvas, x, y);
                    x += cell.width();
                }
                y += LH;
            }
        }

    private:
        cv::Mat *m_canvas = nullptr;
        float m_font_scale = 0.5f;
        struct {
            int x = 0;
            int y = 0;
        } m_cursor;
        struct {
            int top = 10;
            int bottom = 10;
            int left = 10;
            int right = 10;
        } m_border;
        std::vector<Line> m_lines;
        Color m_background;
    };

    class PipeStatistics {
    public:
        using self = PipeStatistics;

        /**
         * This API will be called in thread-1
         * @param report
         */
        void report(const PipeProfiler::Report &report) {
            m_report = report;
        }

        /**
         * This API can be called in thread-2
         * @param value
         * @param max
         * @param spent
         * @param left
         */
        void input(int64_t value, int64_t max,
                   time::ms spent, time::ms left) {
            m_input.value = value;
            m_input.max = max;
            m_input.spent_ms = time::count<time::ms>(spent);
            m_input.left_ms = time::count<time::ms>(left);
        }

        /**
         * This API can be called in thread-2
         * @param dps
         */
        void input_dps(float dps) {
            m_input.dps = dps;
        }

        /**
         * This API only can be called in thread-1
         * @param name
         */
        void imshow(const std::string &name) const {
            if (m_canvas.empty()) return;
            cv::imshow(name, m_canvas);
        }

        /**
         * THie API only can be called in thread-1
         */
        void update() {
            // draw input status and report
            int LW = 20 * m_font_scale;
            int LH = 20 * m_font_scale;

            OpenCanvas canvas;
            canvas.set_font_scale(m_font_scale);
            canvas.bind(m_canvas);

            // make header
            {
                auto value = m_input.value.load();
                auto max = m_input.max.load();
                auto spent = m_input.spent_ms.load();
                auto left = m_input.left_ms.load();
                auto dps = m_input.dps.load();
                static const std::string input = "input:";
                std::string percent = ohm::concat(value, "/", max);
                static const std::string spent_title = "   spent: ";
                static const std::string left_title = "   left: ";
                std::string spent_value = ohm::to_string(time::us(spent * 1000), 8);
                std::string left_value = ohm::to_string(time::us(left * 1000), 8);

                canvas.putText(input);
                canvas.putText(percent);
                canvas.putText(spent_title);
                canvas.putText(spent_value);
                canvas.putText(left_title);
                canvas.putText(left_value);
                canvas.putText(ohm::sprint("   dps: ", int(dps)));
                canvas.enter();
            }
            // make body
            {
                for (auto &name : m_report.lines) {
                    auto &line = m_report.report[name];
                    canvas.putText(name).align()
                        .putText(ohm::concat("I: ", int(line.queue.in.dps), " dps, ")).align()
                        .putText(ohm::concat("[", line.queue.count, "/", line.capacity, "]")).align()
                        .putText(ohm::concat("O: ", int(line.queue.out.dps), " dps, ")).align()
                        .putText(ohm::sprint("Thread: ", line.threads, ", ")).align()
                        .putText(ohm::sprint("Time cost: ", line.average_time)).align();
                    canvas.enter();
                }
            }

            canvas.update();
        }

        void set_font_scale(float scale) {
            m_font_scale = scale;
        }

    private:
        cv::Mat m_canvas;

        struct Input {
            std::atomic<int64_t> value;
            std::atomic<int64_t> max;
            std::atomic<int64_t> spent_ms;
            std::atomic<int64_t> left_ms;
            std::atomic<float> dps;

            Input()
                    : value(0), max(0), spent_ms(0), left_ms(0) {}
        } m_input;

        PipeProfiler::Report m_report;

        float m_font_scale = 0.5f;
    };
}

#endif //OMEGA_OPENCV_PIPE_PROFILER_H
