#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

bool Writer::is_closed() const
{
  // Your code here.
  return is_closed_;
}

void Writer::push( string data )
{
  // Your code here.
  if (is_closed())
    return;
  uint64_t available_capacity = this -> available_capacity();
  uint64_t push_size = min(data.size(), available_capacity);
  bytes_pushed_ += push_size;
  buffer_ += data.substr(0, push_size);
  return;
}

void Writer::close()
{
  // Your code here.
  is_closed_ = true;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_ - buffer_.size();
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return bytes_pushed_;
}

bool Reader::is_finished() const
{
  // Your code here.
  return (is_closed_ && this -> bytes_buffered() == 0);
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return bytes_popped_;
}

string_view Reader::peek() const
{
  // Your code here.
  string_view sv( buffer_.data(), min(static_cast<uint64_t>(1000), this->bytes_buffered()));
  return sv;
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  uint64_t pop_size = min(len, buffer_.size());
  bytes_popped_ += pop_size;
  buffer_.erase(buffer_.begin(), buffer_.begin() +len);
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return buffer_.size();
}
