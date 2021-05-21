include_directories(${CMAKE_SOURCE_DIR}/src)

aux_source_directory(. DNS_RELAY_MODEL_SRC)
add_library(
    dnsRelayModel 
    STATIC 
    ${DNS_RELAY_MODEL_SRC} 
)