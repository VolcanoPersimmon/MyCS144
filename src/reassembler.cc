#include "reassembler.hh"
#include <iostream>
#include <string_view>
#include <typeinfo>
using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  const uint64_t end_index = next_byte_index_ + output_.writer().available_capacity();
  if ( output_.writer().is_closed() || output_.writer().available_capacity() <= 0 || first_index >= end_index ) {
    return;
  }
  if ( first_index + data.size() > end_index ) {
    data.resize( end_index - first_index );
    is_last_substring = false;
  }

  if ( first_index <= next_byte_index_ ) {
    push_to_stream( first_index, data, is_last_substring );
  } else {
    cache_to_buffer( first_index, data, is_last_substring );
  }
  flush();
}

uint64_t Reassembler::bytes_pending() const
{
  return total_bytes_pending_;
}

void Reassembler::flush()
{

  auto it = buffer_.begin();

  while ( it != buffer_.end() && !output_.writer().is_closed() && output_.writer().available_capacity() > 0 ) {
    if ( it->first > next_byte_index_ ) {
      break;
    }
    next_byte_index_ += it->second.data.size();
    total_bytes_pending_ -= it->second.data.size();
    output_.writer().push( it->second.data );
    if ( it->second.is_last_substring ) {
      total_bytes_pending_ = 0;
      output_.writer().close();
    }
    auto temp = it;
    it++;
    buffer_.erase( temp );
  }
}

void Reassembler::push_to_stream( uint64_t first_index, string data, bool is_last_substring )
{
  if ( first_index < next_byte_index_ ) {
    data.erase( 0, next_byte_index_ - first_index );
    first_index = next_byte_index_;
  }
  if ( !buffer_.empty() ) {
    auto it = buffer_.begin();
    auto ite = buffer_.lower_bound( first_index + data.size() );
    int flag = 1;
    while ( flag && it != buffer_.end() ) {
      flag = ( it != ite );
      if ( first_index + data.size() > it->first + it->second.data.size() ) {
        total_bytes_pending_ -= it->second.data.size();
        auto temp = it;
        it++;
        buffer_.erase( temp );
        continue;
      } else if ( first_index + data.size() > it->first ) {
        data.resize( it->first - first_index );
      } else {
        break;
      }
      it++;
    }
  }
  next_byte_index_ += data.size();
  output_.writer().push( std::move( data ) );
  if ( is_last_substring ) {
    output_.writer().close();
  }
}

void Reassembler::cache_to_buffer( uint64_t first_index, string data, bool is_last_substring )
{
  auto it3 = buffer_.lower_bound( first_index + data.size() );
  auto it2 = buffer_.lower_bound( first_index );
  auto it1 = it2;

  if ( it2 != buffer_.begin() ) {
    it1--;

    if ( it1->first + it1->second.data.size() > first_index ) {
      if ( it1->first + it1->second.data.size() < first_index + data.size() ) {
        total_bytes_pending_ -= it1->second.data.size();
        total_bytes_pending_ += first_index - it1->first;
        it1->second.data.resize( first_index - it1->first );
      } else {
        return;
      }
    }
  }
  if ( it2 == buffer_.end() ) {
    total_bytes_pending_ += data.size();
    BString tmp( std::move( data ), is_last_substring );
    buffer_[first_index] = std::move( tmp );
    return;
  }
  auto it = it2;
  int flag = 1;
  while ( flag && it != buffer_.end() ) {
    flag = ( it != it3 );
    if ( first_index + data.size() > it->first + it->second.data.size() ) {
      total_bytes_pending_ -= it->second.data.size();
      auto tmp = it;
      it++;
      buffer_.erase( tmp );
      continue;
    } else if ( first_index + data.size() > it->first ) {
      data.resize( it->first - first_index );
      is_last_substring = false;
    } else {
      break;
    }
    it++;
  }
  if ( data.size() == 0 ) {
    return;
  }
  total_bytes_pending_ += data.size();
  BString tmp( std::move( data ), is_last_substring );
  buffer_[first_index] = std::move( tmp );
}
