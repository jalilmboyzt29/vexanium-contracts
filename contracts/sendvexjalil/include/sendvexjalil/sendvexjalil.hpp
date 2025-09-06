#pragma once

#include <vexanium/vexanium.hpp>
#include <string>

using namespace vexanium;
using std::string;

CONTRACT sendvexjalil : public contract {
   public:
      using contract::contract;

      // Struktur untuk menyimpan statistik dan detail token
      struct [[vexanium::table]] currency_stats {
         asset    supply;
         asset    max_supply;
         name     issuer;
         string   token_name;      // Nama lengkap token
         string   logo_url;        // URL logo token
         
         uint64_t primary_key()const { return supply.symbol.code().raw(); }
      };

      // Struktur untuk menyimpan balance akun
      struct [[vexanium::table]] account {
         asset    balance;

         uint64_t primary_key()const { return balance.symbol.code().raw(); }
      };

      typedef multi_index< "stat"_n, currency_stats > stats;
      typedef multi_index< "accounts"_n, account > accounts;

      // Actions
      [[vexanium::action]]
      void addtoken( const name& issuer, const string& token_name, const string& symbol_str, 
                     uint8_t precision, uint64_t max_supply, const string& logo_url );

      [[vexanium::action]]
      void create( const name& issuer, const asset& maximum_supply );

      [[vexanium::action]]
      void updateinfo( const string& symbol_str, const string& token_name, const string& logo_url );

      [[vexanium::action]]
      void issue( const name& to, const asset& quantity, const string& memo );

      [[vexanium::action]]
      void retire( const asset& quantity, const string& memo );

      [[vexanium::action]]
      void transfer( const name& from, const name& to, const asset& quantity, const string& memo );

      [[vexanium::action]]
      void open( const name& owner, const symbol& symbol, const name& ram_payer );

      [[vexanium::action]]
      void close( const name& owner, const symbol& symbol );

   private:
      void sub_balance( const name& owner, const asset& value );
      void add_balance( const name& owner, const asset& value, const name& ram_payer );
};
