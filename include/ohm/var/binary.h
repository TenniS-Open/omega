//
// Created by Lby on 2017/10/31.
//

#ifndef OMEGA_VAR_BINARY_H
#define OMEGA_VAR_BINARY_H

#include <cstddef>
#include <memory>
#include <string>

#include "notation.h"

namespace ohm {
    namespace notation {
        class Binary {
        public:
            using self = Binary;

            enum class pos {
                now,
                beg,
                end,
            };

            using size_t = ::size_t;


            Binary() {}

            Binary(size_t _size) {
                resize(_size);
            }

            Binary(const void *_buffer, size_t _size) {
                write(_buffer, _size);
            }

            size_t size() const { return m_size; }

            size_t capacity() const { return m_capacity; }

            size_t read(void *_buffer, size_t _size) const {
                size_t memory_left = m_size - m_index;
                size_t can_read = std::min<size_t>(memory_left, _size);
                std::memcpy(_buffer, now_data(), can_read);
                m_index += can_read;
                return can_read;
            }

            size_t write(const void *_buffer, size_t _size) {
                size_t memory_right = m_index + _size;
                reverse(memory_right);
                std::memcpy(now_data(), _buffer, _size);
                if (memory_right > m_size) m_size = memory_right;
                m_index += _size;
                return _size;
            }

            size_t get_pos() const {
                return m_index;
            }

            size_t set_pos(pos _pos, int _shift) {
                m_index = correct_index(_pos, _shift);
                return m_index;
            }

            void shift(int _size) {
                set_pos(pos::now, _size);
            }

            const void *data() const {
                return m_data.get();
            }

            void *data() {
                return m_data.get();
            }

            template<typename T>
            const T *data() const { return reinterpret_cast<const T *>(data()); }

            template<typename T>
            T *data() { return reinterpret_cast<T *>(data()); }

            Binary clone() const {
                Binary doly;
                doly.write(self::data(), self::size());
                return std::move(doly);
            }

            void memset(char ch) {
                std::memset(self::data(), ch, self::capacity());
            }

            void memset(pos _pos, int _begin, int _end, char ch) {
                size_t c_begin = correct_index(_pos, _begin);
                size_t c_end = correct_index(_pos, _end);
                size_t c_size = c_end - c_begin;
                std::memset(self::data<char>() + c_begin, ch, c_size);
            }

            void reverse(size_t _size) {
                if (_size > m_capacity) {
                    auto *new_data = std::malloc(_size);
                    std::memcpy(new_data, self::data(), self::size());
                    m_data.reset(new_data, std::free);
                    m_capacity = _size;
                }
            }

            void resize(size_t _size) {
                reverse(_size);
                m_size = _size;
            }

            void clear() {
                m_index = 0;
                m_size = 0;
            }

            void dispose() {
                m_index = 0;
                m_size = 0;
                m_capacity = 0;
                m_data.reset();
            }

            bool empty() const {
                return m_size == 0;
            }

        private:
            std::shared_ptr<void> m_data;
            size_t m_capacity = 0;
            size_t m_size = 0;
            mutable size_t m_index = 0;

            size_t correct_index(int _index) {
                int c_index = std::max<int>(0, std::min<int>(static_cast<int>(m_size), _index));
                return static_cast<size_t>(c_index);
            }

            size_t correct_index(pos _pos, int _shift) {
                size_t _base = m_index;
                switch (_pos) {
                    case pos::beg:
                        _base = 0;
                        break;
                    case pos::now:
                        _base = m_index;
                        break;
                    case pos::end:
                        _base = m_size;
                        break;
                }
                int b_index = static_cast<int>(_base) + _shift;
                return correct_index(b_index);
            }

            void *now_data() { return self::data<char>() + m_index; }

            const void *now_data() const { return const_cast<self *>(this)->now_data(); }


        };

        inline std::string to_string(const Binary &str) {
            return std::string(str.data<char>(), str.size());
        }

        inline Binary to_binary(const std::string &bin) {
            return Binary(bin.data(), bin.size());
        }

        inline bool operator==(const Binary &lhs, const Binary &rhs) {
            if (lhs.size() != rhs.size()) return false;
            using long_step_type = size_t;
            size_t size = lhs.size();
            size_t long_step = sizeof(long_step_type);
            size_t long_step_size = size / long_step;
            for (size_t i = 0; i < long_step_size; ++i) {
                if (lhs.data<long_step_type>()[i] != rhs.data<long_step_type>()[i]) return false;
            }
            for (size_t i = long_step_size * long_step; i < size; ++i) {
                if (lhs.data<unsigned char>()[i] != rhs.data<unsigned char>()[i]) return false;
            }
            return true;
        }

        inline bool operator!=(const Binary &lhs, const Binary &rhs) {
            return !operator==(lhs, rhs);
        }

        using ElementBinary = TrustElement<type::Binary, Binary>;

        template<>
        struct type_code<Binary> {
            static const DataType code = type::Binary;
        };

        template<>
        struct code_type<type::Binary> {
            using type = ElementBinary;
        };
    }
}


#endif //OMEGA_VAR_BINARY_H
