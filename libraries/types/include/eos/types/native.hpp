#pragma once
#include <vector>
#include <array>
#include <string>
#include <functional>

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <fc/variant.hpp>
#include <fc/crypto/base64.hpp>
#include <fc/crypto/hex.hpp>
#include <fc/crypto/elliptic.hpp>
#include <fc/crypto/sha256.hpp>
#include <fc/io/datastream.hpp>
#include <fc/time.hpp>
#include <fc/fixed_string.hpp>

#include <fc/reflect/reflect.hpp>

namespace eos { namespace types {
   using namespace boost::multiprecision;

   template<typename... T>
   using Vector       = std::vector<T...>;

   template<typename... T>
   using Array        = std::array<T...>;

   using String        = std::string;
   using Time          = fc::time_point_sec;
   using Signature     = fc::ecc::compact_signature;
   using Checksum      = fc::sha256;
   using FieldName     = fc::fixed_string<>;
   using FixedString32 = fc::fixed_string<fc::array<uint64_t,4>>;// std::tuple<uint64_t,uint64_t,uint64_t,uint64_t>>; 
   using FixedString16 = fc::fixed_string<>; 
   using TypeName      = FixedString32;
   using Bytes         = Vector<char>;

   template<size_t Size>
   using UInt = number<cpp_int_backend<Size, Size, unsigned_magnitude, unchecked, void> >;
   template<size_t Size>
   using Int = number<cpp_int_backend<Size, Size, signed_magnitude, unchecked, void> >;

   using UInt8     = UInt<8>; 
   using UInt16    = UInt<16>; 
   using UInt32    = UInt<32>;
   using UInt64    = UInt<64>;
   using UInt128   = boost::multiprecision::uint128_t;
   using UInt256   = boost::multiprecision::uint256_t;
   using Int8      = int8_t;//Int<8>;  these types are different sizes than native...
   using Int16     = int16_t; //Int<16>;
   using Int32     = int32_t; //Int<32>;
   using Int64     = int64_t; //Int<64>; 
   using Int128    = boost::multiprecision::int128_t;
   using Int256    = boost::multiprecision::int256_t;

   struct Name {
      uint64_t value = 0;
      Name( const String& str ) {
         FC_ASSERT( str.size() <= 12 );
         for( uint32_t i = 0; i < str.size(); ++i ) {
            value <<= 5;
            value |= char_to_symbol( str[ str.size() -1 - i ] );
         }
      }
      char char_to_symbol( char c ) const {
         if( c >= 'a' && c <= 'z' )
            return (c - 'a') + 1;
         if( c >= '1' && c <= '5' )
            return (c - '1') + 26;
         FC_ASSERT( c == '.', "invalid character '${c}' (${i}) in Name string", ("c", String(&c,1))("i",int(c)) );
         return 0;
      }

      Name( uint64_t v = 0 ):value(v){
         FC_ASSERT( !(v>>(5*12)), "invalid name id" );
      };

      operator String()const {
         static const char* charmap = ".abcdefghijklmnopqrstuvwxyz12345";
         String str;
         uint64_t tmp = value;
         for( uint32_t i = 0; i < 12; ++i ) {
            char c = charmap[tmp & 0x1f ];
            str += c;
            tmp >>= 5;
         }
         boost::algorithm::trim_right_if( str, []( char c ){ return c == '.'; } );
         return str;
      }

      Name& operator=( uint64_t v ) {
         FC_ASSERT( !(v>>(5*12)), "invalid name id" );
         value = v;
         return *this;
      }

      Name& operator=( const String& n ) {
         value = Name(n).value;
         return *this;
      }

      operator uint64_t()const        { return value; }
   };

   struct Field {
      FieldName name;
      TypeName  type;

      bool operator==(const Field& other) const;
   };

   struct Struct {
      TypeName        name;
      TypeName        base;
      Vector<Field>   fields;

      bool operator==(const Struct& other) const;
   };

   using Fields = Vector<Field>;

   template<typename T>
   struct GetStruct{};

    template<> struct GetStruct<Field> { 
        static const Struct& type() { 
           static Struct result = { "Field ", "", {
                {"name", "FieldName"},
                {"type", "TypeName"}
              }
           };
           return result;
         }
    };
    template<> struct GetStruct<Struct> { 
        static const Struct& type() { 
           static Struct result = { "Struct ", "", {
                {"name", "TypeName"},
                {"base", "TypeName"},
                {"fields", "Field[]"}
              }
           };
           return result;
         }
    };


   /// TODO: make sure this works with FC raw
   template<typename Stream, typename Number>
   void fromBinary(Stream& st, boost::multiprecision::number<Number>& value) {
      unsigned char data[(std::numeric_limits<decltype(value)>::digits+1)/8];
      st.read((char*)data, sizeof(data));
      boost::multiprecision::import_bits(value, data, data + sizeof(data), 1);
   }
   template<typename Stream, typename Number>
   void toBinary(Stream& st, const boost::multiprecision::number<Number>& value) {
      unsigned char data[(std::numeric_limits<decltype(value)>::digits+1)/8];
      boost::multiprecision::export_bits(value, data, 1);
      st.write((const char*)data, sizeof(data));
   }
   
}} // namespace eos::types

namespace fc {
  void to_variant(const eos::types::Name& c, fc::variant& v);
  void from_variant(const fc::variant& v, eos::types::Name& check);
  void to_variant(const std::vector<eos::types::Field>& c, fc::variant& v);
  void from_variant(const fc::variant& v, std::vector<eos::types::Field>& check);
  void to_variant(const std::map<std::string,eos::types::Struct>& c, fc::variant& v);
  void from_variant(const fc::variant& v, std::map<std::string,eos::types::Struct>& check);
}

FC_REFLECT(eos::types::Name, (value))
FC_REFLECT(eos::types::Field, (name)(type))
FC_REFLECT(eos::types::Struct, (name)(base)(fields))
