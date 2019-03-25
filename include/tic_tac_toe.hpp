#include <eosio/eosio.hpp>

using namespace eosio;

CONTRACT tic_tac_toe : public contract {
   public:
    tic_tac_toe(name self, name first_receiver, datastream<const char*> ds) : contract(self, first_receiver, ds) {}

    ACTION create(name challenger, name host);
    ACTION restart(name challenger, name host, name by);
    ACTION close(name challenger, name host);
    ACTION move(name challenger, name host, name by, const uint16_t row, const uint16_t column);

    TABLE game {
        static const uint16_t board_width = 3;
        static const uint16_t board_height = board_width;
        game() { initialize_board(); }

        name challenger;
        name host;
        name turn;
        name winner = "none"_n;
        std::vector<uint8_t> board;

        void initialize_board() { board = std::vector<uint8_t>(board_width * board_height, 0); }

        void reset_game() {
            initialize_board();
            turn = host;
            winner = "none"_n;
        }

        uint64_t primary_key() const { return challenger.value; }

        EOSLIB_SERIALIZE(game, (challenger)(host)(turn)(winner)(board))
    };

    typedef eosio::multi_index<"games"_n, game> games;

   private:
    using create_action = action_wrapper<"create"_n, &tic_tac_toe::create>;
    using restart_action = action_wrapper<"restart"_n, &tic_tac_toe::restart>;
    using close_action = action_wrapper<"close"_n, &tic_tac_toe::close>;
    using move_action = action_wrapper<"move"_n, &tic_tac_toe::move>;
};
