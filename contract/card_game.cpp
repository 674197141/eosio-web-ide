#include <eosio/eosio.hpp>

// user table
struct [[eosio::table("table_user"), eosio::contract("CardGame")]] table_user {
    eosio::name user     = {};
    uint64_t    login_time = {}; // Non-0 if this is a reply
    std::string data = {};
    uint64_t primary_key() const { return user.value; }
    uint64_t get_login_time() const { return -login_time; }
};

using user_table = eosio::multi_index<
    "table_user"_n, table_user, eosio::indexed_by<"by.login.time"_n, eosio::const_mem_fun<table_user, uint64_t, &table_user::get_login_time>>>;

// The contract
class CardGame : eosio::contract {
  public:
    // Use contract's constructor
    using contract::contract;

    [[eosio::action]]
    void Login(eosio::name user){
        table_user table{user, 0};
        // 当前时间戳
        require_auth(user);
        auto table_user_itr = table.find(user.value);
        uint64_t time_now = current_time();
        if(table_user_itr == addresses.end())
        {
            addresses.emplace(user, [&]( auto& row ) {
            row.user = user;
            row.user = first_name;
            row.last_name = last_name;
            row.street = street;
            row.city = city;
            row.state = state;
            });
        }
        else {
            addresses.modify(table_user_itr, user, [&]( auto& row ) {
                row.key = user;
                row.first_name = first_name;
                row.last_name = last_name;
                row.street = street;
                row.city = city;
                row.state = state;
            });
        }
        table.get(user);
        five_minutes_later = time_now + five_minutes;
        five_minutes_later > time_now;
    }

    // Post a message
    [[eosio::action]]
    void post(uint64_t id, uint64_t reply_to, eosio::name user, const std::string& content) {
        table_user table{get_self(), 0};

        // Check user
        require_auth(user);

        // Check reply_to exists
        if (reply_to)
            table.get(reply_to);

        // Create an ID if user didn't specify one
        eosio::check(id < 1'000'000'000ull, "user-specified id is too big");
        if (!id)
            id = std::max(table.available_primary_key(), 1'000'000'000ull);

        // Record the message
        table.emplace(get_self(), [&](auto& message) {
            message.id       = id;
            message.reply_to = reply_to;
            message.user     = user;
            message.content  = content;
        });
    }
};
