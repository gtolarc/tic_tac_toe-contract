project(tic_tac_toe)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( tic_tac_toe tic_tac_toe tic_tac_toe.cpp )
target_include_directories( tic_tac_toe PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( tic_tac_toe ${CMAKE_SOURCE_DIR}/../ricardian )