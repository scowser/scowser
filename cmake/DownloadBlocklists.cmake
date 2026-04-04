set(BLOCKLIST_DIR "${CMAKE_SOURCE_DIR}/resources/blocklists")

set(EASYLIST_URL "https://easylist.to/easylist/easylist.txt")
set(EASYPRIVACY_URL "https://easylist.to/easylist/easyprivacy.txt")

function(download_if_missing url filename)
    set(filepath "${BLOCKLIST_DIR}/${filename}")
    if(NOT EXISTS "${filepath}")
        message(STATUS "Downloading ${filename}...")
        file(DOWNLOAD "${url}" "${filepath}"
            STATUS download_status
            TIMEOUT 30
        )
        list(GET download_status 0 status_code)
        if(NOT status_code EQUAL 0)
            message(WARNING "Failed to download ${filename}: ${download_status}")
            file(REMOVE "${filepath}")
        else()
            message(STATUS "Downloaded ${filename}")
        endif()
    else()
        message(STATUS "Found existing ${filename}")
    endif()
endfunction()

download_if_missing("${EASYLIST_URL}" "easylist.txt")
download_if_missing("${EASYPRIVACY_URL}" "easyprivacy.txt")
