#ifndef __XBUFFER__HPP__
#define __XBUFFER__HPP__
#include <string>
namespace Novichu
{
    class XBuffer
    {
    public:
        XBuffer(size_t len) : _total_len(len), _cur_pos(0) {}
        XBuffer() : _total_len(0), _cur_pos(0) {}
        XBuffer(std::string &&in) : _buf(std::move(in)), _cur_pos(0) {}
        XBuffer(const std::string &in) : _buf(in), _cur_pos(0) {}
        XBuffer(const XBuffer &other) : _buf(other._buf), _total_len(other._total_len), _cur_pos(other._cur_pos) {}
        XBuffer(XBuffer &&other) : _buf(std::move(other._buf)), _total_len(other._total_len), _cur_pos(other._cur_pos) { other._cur_pos = 0; }
        XBuffer &operator=(const XBuffer &other)
        {
            _buf = other._buf;
            _cur_pos = other._cur_pos;
            _total_len = other._total_len;
            return *this;
        }
        XBuffer &operator=(XBuffer &&other)
        {
            _buf = std::move(other._buf);
            _cur_pos = other._cur_pos;
            _total_len = other._total_len;
            other._cur_pos = 0;
            return *this;
        }
        void set_max(size_t len) { _total_len = len; }
        size_t avaliable() { return _total_len - _buf.size(); }
        bool empty() { return _buf.empty(); }
        bool full() { return _buf.size() > _total_len; }
        std::string &append(const std::string &log_line) { return _buf.append(log_line); }
        std::string &append(std::string &&log_line) { return _buf.append(std::move(log_line)); }
        void clear() { _buf.clear(), _cur_pos = 0; }
        void reset() { _cur_pos = 0; }
        void offset(size_t k) { _cur_pos += k; }
        size_t size() { return _buf.size(); }
        bool is_end() { return _cur_pos >= size(); }
        void input(const std::string &in) { append(in); }
        void input(std::string &&in) { append(std::move(in)); }
        void insert(size_t pos, const std::string &in) { _buf.insert(pos, in); }
        size_t findc(char c) { return _buf.find(c); }
        std::string &data() { return _buf; }
        std::string current() { return _buf.substr(_cur_pos); }
        std::string current(size_t len) { return _buf.substr(_cur_pos, len); }

    private:
        size_t _total_len;
        std::string _buf;
        size_t _cur_pos;
    };
}
#endif