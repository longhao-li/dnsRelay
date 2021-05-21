include_directories(${CMAKE_SOURCE_DIR}/src)

aux_source_directory(. DNS_RELAY_CORE_SRC)

add_library(
    dnsRelayCore 
    STATIC 
    ${DNS_RELAY_CORE_SRC}
)