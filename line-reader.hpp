#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace jaw {

/*
 *  reader
 *
 *  initialize with a file
 *  call to return a null terminated line
 *  line separators are lf, crlf, cr
 *  we'll read a lf, or cr followed by lf, or
 *  standalone cr
 */

// lazy line reader
struct reader {
  std::string _filename;
  std::vector<char> _buffer;
  std::vector<char>::iterator _pos;
  std::vector<char>::iterator _end;
  std::vector<char>::iterator _line;
  bool _badchar;

  // theres another line to return
  bool hasline() { return _pos != _end; }

  reader() = delete;
  reader(std::string filename) : _filename(filename) {
    // get file size
    std::fstream file(_filename,  std::fstream::in |
                                  std::fstream::ate |
                                  std::fstream::binary);
    auto filesize = long(file.tellg());
    auto buffersize = filesize + 1;
    // create buffer
    _buffer = std::vector<char>(buffersize);
    // seek to begin and read file
    file.seekg(std::fstream::beg);
    file.read(&_buffer[0], filesize);
    // setup iterators
    _pos = _line = _buffer.begin();
    _end = _buffer.end();
  };

  // find next line in zero terminated buffer
  std::vector<char>::iterator findnextline() {
    while (1) {
      // read chars from pos until \0 lf crlf cr error
      while (*_pos > 0xD) {
        // leave pos on \0 \a \d or bad character
        ++_pos;
      }
      // if we read a cr, skip over single following lf
      switch (*_pos) {
      case 0x0: // end of buffer
      case 0xA: // terminate at lf or lf part of crlf
        *_pos++ = '\0';
        return _pos;
      case 0xD: // terminate at cr or crlf
        *_pos++ = '\0';
        if (*_pos != 0xA) {
          // not a lf, return position of next line
          return _pos;
        }
        // otherwise, terminate at lf of crlf and bump _pos
        *_pos++ = '\0';
        return _pos;
      default: // bad character, just skip over it
        _badchar |= true;
        ++_pos;
      }
    }
    return _pos;
  }

  // getline
  // _pos always points at nextline
  // when called, we need to terminate the line, and be ready
  // with the next line.
  std::string_view getline() {
    // set current line to pos
    _line = _pos;
    // at end of buffer? no more lines, return empty string_view
    if (_line == _end)
      return std::string_view();
    // move _pos to start of next line
    _pos = findnextline();
    // return current line
    return std::string_view(&*_line);
  }
};

} // namespace jaw
