#include <sendvexjalil/sendvexjalil.hpp>

// Action addtoken - membuat token dengan informasi lengkap
void sendvexjalil::addtoken( const name& issuer, const string& token_name, const string& symbol_str, 
                            uint8_t precision, uint64_t max_supply, const string& logo_url ) {
    require_auth( issuer );
    
    // Validasi input
    check( token_name.size() > 0 && token_name.size() <= 64, "Token name must be 1-64 characters" );
    check( logo_url.size() <= 256, "Logo URL must be max 256 characters" );
    
    // Buat symbol dari string dan precision
    symbol sym = symbol(symbol_str, precision);
    check( sym.is_valid(), "invalid symbol name" );
    check( max_supply > 0, "max-supply must be positive");

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing == statstable.end(), "token with symbol already exists" );

    // Simpan token dengan informasi lengkap
    statstable.emplace( issuer, [&]( auto& s ) {
       s.supply = asset(0, sym);
       s.max_supply = asset(max_supply, sym);
       s.issuer = issuer;
       s.token_name = token_name;
       s.logo_url = logo_url;
    });
}

// Action updateinfo - update nama token dan logo URL
void sendvexjalil::updateinfo( const string& symbol_str, const string& token_name, const string& logo_url ) {
    symbol sym = symbol(symbol_str, 4); // Asumsikan precision 4
    
    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist" );
    
    const auto& st = *existing;
    require_auth( st.issuer ); // Hanya issuer yang bisa update info
    
    // Validasi input
    check( token_name.size() > 0 && token_name.size() <= 64, "Token name must be 1-64 characters" );
    check( logo_url.size() <= 256, "Logo URL must be max 256 characters" );
    
    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.token_name = token_name;
       s.logo_url = logo_url;
    });
}

// Action create - standar seperti eosio.token
void sendvexjalil::create( const name& issuer, const asset& maximum_supply ) {
    require_auth( get_self() );

    auto sym = maximum_supply.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( maximum_supply.is_valid(), "invalid supply");
    check( maximum_supply.amount > 0, "max-supply must be positive");

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing == statstable.end(), "token with symbol already exists" );

    statstable.emplace( get_self(), [&]( auto& s ) {
       s.supply.symbol = maximum_supply.symbol;
       s.max_supply = maximum_supply;
       s.issuer = issuer;
       s.token_name = ""; // Kosong jika pakai create
       s.logo_url = "";   // Kosong jika pakai create
    });
}

// Action issue - mengeluarkan token
void sendvexjalil::issue( const name& to, const asset& quantity, const string& memo ) {
    auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
    const auto& st = *existing;

    require_auth( st.issuer );
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must issue positive quantity" );

    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    check( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.supply += quantity;
    });

    add_balance( to, quantity, st.issuer );
}

void sendvexjalil::retire( const asset& quantity, const string& memo ) {
    auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist" );
    const auto& st = *existing;

    require_auth( st.issuer );
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must retire positive quantity" );

    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );

    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.supply -= quantity;
    });

    sub_balance( st.issuer, quantity );
}

void sendvexjalil::transfer( const name& from, const name& to, const asset& quantity, const string& memo ) {
    check( from != to, "cannot transfer to self" );
    require_auth( from );
    check( is_account( to ), "to account does not exist");
    
    auto sym = quantity.symbol.code();
    stats statstable( get_self(), sym.raw() );
    const auto& st = statstable.get( sym.raw() );

    require_recipient( from );
    require_recipient( to );

    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must transfer positive quantity" );
    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    auto payer = has_auth( to ) ? to : from;

    sub_balance( from, quantity );
    add_balance( to, quantity, payer );
}

void sendvexjalil::open( const name& owner, const symbol& symbol, const name& ram_payer ) {
   require_auth( ram_payer );

   check( is_account( owner ), "owner account does not exist" );

   auto sym_code_raw = symbol.code().raw();
   stats statstable( get_self(), sym_code_raw );
   const auto& st = statstable.get( sym_code_raw, "symbol does not exist" );
   check( st.supply.symbol == symbol, "symbol precision mismatch" );

   accounts acnts( get_self(), owner.value );
   auto it = acnts.find( sym_code_raw );
   if( it == acnts.end() ) {
      acnts.emplace( ram_payer, [&]( auto& a ){
        a.balance = asset{0, symbol};
      });
   }
}

void sendvexjalil::close( const name& owner, const symbol& symbol ) {
   require_auth( owner );
   accounts acnts( get_self(), owner.value );
   auto it = acnts.find( symbol.code().raw() );
   check( it != acnts.end(), "Balance row already deleted or never existed. Action won't have any effect." );
   check( it->balance.amount == 0, "Cannot close because the balance is not zero." );
   acnts.erase( it );
}

void sendvexjalil::sub_balance( const name& owner, const asset& value ) {
   accounts from_acnts( get_self(), owner.value );

   const auto& from = from_acnts.get( value.symbol.code().raw(), "no balance object found" );
   check( from.balance.amount >= value.amount, "overdrawn balance" );

   from_acnts.modify( from, owner, [&]( auto& a ) {
         a.balance -= value;
      });
}

void sendvexjalil::add_balance( const name& owner, const asset& value, const name& ram_payer ) {
   accounts to_acnts( get_self(), owner.value );
   auto to = to_acnts.find( value.symbol.code().raw() );
   if( to == to_acnts.end() ) {
      to_acnts.emplace( ram_payer, [&]( auto& a ){
        a.balance = value;
      });
   } else {
      to_acnts.modify( to, same_payer, [&]( auto& a ) {
        a.balance += value;
      });
   }
}
