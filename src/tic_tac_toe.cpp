#include <tic_tac_toe.hpp>

ACTION tic_tac_toe::create(name challenger, name host) {
    require_auth(host);
    eosio_assert(challenger != host, "create: challenger shouldn't be the same as host");

    games existing_host_games(_self, host.value);
    auto itr = existing_host_games.find(challenger.value);
    eosio_assert(itr == existing_host_games.end(), "create: game already exists");

    existing_host_games.emplace(host, [&](auto& g) {
        g.challenger = challenger;
        g.host = host;
        g.turn = host;
    });
}

ACTION tic_tac_toe::restart(name challenger, name host, name by) {
    require_auth(by);

    games existing_host_games(_self, host.value);
    auto itr = existing_host_games.find(challenger.value);
    eosio_assert(itr != existing_host_games.end(), "restart: game doesn't exists");

    eosio_assert(by == itr->host || by == itr->challenger, "restart: this is not your game!");

    existing_host_games.modify(itr, itr->host, [](auto& g) { g.reset_game(); });
}

ACTION tic_tac_toe::close(name challenger, name host) {
    require_auth(host);

    games existing_host_games(_self, host.value);
    auto itr = existing_host_games.find(challenger.value);
    eosio_assert(itr != existing_host_games.end(), "close: game doesn't exists");

    existing_host_games.erase(itr);
}

bool is_empty_cell(const uint8_t& cell) {
    return cell == 0;
}

bool is_valid_movement(const uint16_t& row, const uint16_t& column, const vector<uint8_t>& board) {
    uint16_t board_width = tic_tac_toe::game::board_width;
    uint16_t board_height = tic_tac_toe::game::board_height;
    uint32_t movement_location = row * board_width + column;
    bool is_valid = column < board_width && row < board_height && is_empty_cell(board[movement_location]);
    return is_valid;
}

name get_winner(const tic_tac_toe::game& current_game) {
    auto& board = current_game.board;
    bool is_board_full = true;

    // Use bitwise AND operator to determine the consecutive values of each column, row and diagonal
    // Since 3 == 0b11, 2 == 0b10, 1 = 0b01, 0 = 0b00
    vector<uint32_t> consecutive_column(tic_tac_toe::game::board_width, 3);
    vector<uint32_t> consecutive_row(tic_tac_toe::game::board_height, 3);
    uint32_t consecutive_diagonal_backslash = 3;
    uint32_t consecutive_diagonal_slash = 3;
    for (uint32_t i = 0; i < board.size(); i++) {
        is_board_full &= !is_empty_cell(board[i]);
        uint16_t row = uint16_t(i / tic_tac_toe::game::board_width);
        uint16_t column = uint16_t(i % tic_tac_toe::game::board_width);

        // Calculate consecutive row and column value
        consecutive_row[column] = consecutive_row[column] & board[i];
        consecutive_column[row] = consecutive_column[row] & board[i];
        // Calculate consecutive diagonal \ value
        if (row == column) {
            consecutive_diagonal_backslash = consecutive_diagonal_backslash & board[i];
        }
        // Calculate consecutive diagonal / value
        if (row + column == tic_tac_toe::game::board_width - 1) {
            consecutive_diagonal_slash = consecutive_diagonal_slash & board[i];
        }
    }

    // Inspect the value of all consecutive row, column, and diagonal and determine winner
    vector<uint32_t> aggregate = {consecutive_diagonal_backslash, consecutive_diagonal_slash};
    aggregate.insert(aggregate.end(), consecutive_column.begin(), consecutive_column.end());
    aggregate.insert(aggregate.end(), consecutive_row.begin(), consecutive_row.end());
    for (auto value : aggregate) {
        if (value == 1) {
            return current_game.host;
        } else if (value == 2) {
            return current_game.challenger;
        }
    }

    // Draw if the board is full, otherwise the winner is not determined yet
    return is_board_full ? "draw"_n : "none"_n;
}

ACTION tic_tac_toe::move(name challenger, name host, name by, const uint16_t row, const uint16_t column) {
    require_auth(by);

    games existing_host_games(_self, host.value);
    auto itr = existing_host_games.find(challenger.value);
    eosio_assert(itr != existing_host_games.end(), "move: game doesn't exists");
    eosio_assert(itr->winner == "none"_n, "move: the game has ended!");
    eosio_assert(by == itr->host || by == itr->challenger, "move: this is not your game!");
    eosio_assert(by == itr->turn, "move: it's not your turn yet!");
    eosio_assert(is_valid_movement(row, column, itr->board), "move: not a valid movement!");

    const uint8_t cell_value = itr->turn == itr->host ? 1 : 2;
    const auto turn = itr->turn == itr->host ? itr->challenger : itr->host;
    existing_host_games.modify(itr, itr->host, [&](auto& g) {
        g.board[row * tic_tac_toe::game::board_width + column] = cell_value;
        g.turn = turn;
        g.winner = get_winner(g);
    });
}

EOSIO_DISPATCH(tic_tac_toe, (create)(restart)(close)(move))
