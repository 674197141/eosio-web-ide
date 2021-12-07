#include <eosio/eosio.hpp>

// 管理表
struct [[eosio::table("manager_user"), eosio::contract("PiaoPiaoLe")]] manager_user {
    uint64_t    id          = {};   // Non-0
    eosio::name user     = {};
    uint64_t    login_time = {}; // Non-0 if this is a reply

    uint64_t primary_key() const { return id; }
    uint64_t get_login_time() const { return -login_time; }
};

using manager_user_table = eosio::multi_index<
    "manager_user"_n, manager_user, eosio::indexed_by<"by.login.time"_n, eosio::const_mem_fun<manager_user, uint64_t, &manager_user::get_login_time>>>;

// 类型表
struct [[eosio::table("happy_type"), eosio::contract("PiaoPiaoLe")]] happy_type {
    uint64_t    id          = {};   // Non-0
    eosio::name user        = {};
    std::string to_teacher  = {};
    uint64_t    is_open     = {};   // Non-0
    uint64_t    create_time = {};   // Non-0 if this is a reply
    uint64_t    end_time = {};      // Non-0 if this is a reply
    uint64_t    need_num = {};      // 需要金额
    uint64_t    have_num = {};      // 筹集金额

    uint64_t primary_key() const { return id; }
    uint64_t get_create_time() const { return -create_time; }
};

// 数据表-一个管理同一时间只能开一个
struct [[eosio::table("happy_data"), eosio::contract("PiaoPiaoLe")]] happy_data {
    uint64_t    id             = {}; // Non-0
    uint64_t    type_id        = {}; // Non-0
    eosio::name people         = {}; // form-user
    uint64_t    people_pay_num = {}; // Non-0 if this is a reply
    uint64_t    pay_time       = {}; // Non-0 if this is a reply

    uint64_t primary_key() const { return id; }
    uint64_t get_create_time() const { return -pay_time; }
};

using user_table = eosio::multi_index<
    "manager_user"_n, table_user, eosio::indexed_by<"by.login.time"_n, eosio::const_mem_fun<table_user, uint64_t, &table_user::get_login_time>>>;


// The contract
class PiaoPiaoLe : eosio::contract {
  public:
    // Use contract's constructor
    using contract::contract;

    [[eosio::action]]
    void Login(eosio::name user){
        table_user table{get_self(), 0};
        // 当前时间戳
        require_auth(user);
        auto table_user_itr = table.find(user.value);

        if (table_user_itr != table.end()) {
            _refer_amount.modify(refer_amount_itr, same_payer, [&](auto &ra) {
                vector<asset> &assets = ra.assets;
                bool should_add_new_token = true;
                for (int i = 0; i < assets.size(); i++) {
                    if (assets[i].symbol == quantity.symbol) {
                        should_add_new_token = false;
                        break;
                    }
                }

                eosio_assert(should_add_new_token,"you already support this token");
                quantity=asset(0,quantity.symbol);
                ra.assets.push_back(quantity);

            });
        }
        uint64_t time_now = current_time();
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
