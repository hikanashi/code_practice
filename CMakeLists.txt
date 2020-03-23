
file(GLOB SOURCES 
	${CMAKE_CURRENT_SOURCE_DIR}/*.c*)

add_executable(curl_test
	${SOURCES}
)

message("CMAKE_PREFIX_PATH:${CMAKE_PREFIX_PATH}")

find_package(CURL CONFIG REQUIRED)
target_link_libraries(curl_test PRIVATE CURL::libcurl)

find_package(OpenSSL REQUIRED)
target_link_libraries(curl_test PRIVATE OpenSSL::SSL OpenSSL::Crypto)


find_library(PCRE_LIBRARY pcreposix)
target_link_libraries(curl_test PRIVATE ${PCRE_LIBRARY})

find_package(pthread)
target_link_libraries(curl_test PRIVATE ${PTHREAD_LIBRARY})
message("PTHREAD_LIBRARY:${PTHREAD_LIBRARY}")

if(MSVC)
#	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LIBPATH:\"c:\\opt\\local\\lib\"")

	set(LIBS
		mockhttpd
#		libcurl
#		event
#		event_openssl
#		libssl
#		libcrypto
#		zlib
#		nghttp2
#		ws2_32
#		pthreadVC2
#		CRYPT32
#		Wldap32
#		Normaliz
	)

else()

	set(LIBS
		mockhttpd
		rt
		curl
		crypto
		ssl
		z
		pthread
		event
		event_openssl
		event_pthreads
	)
        #add_custom_command(TARGET curl_test POST_BUILD COMMAND sudo setcap cap_net_bind_service=ep $<TARGET_FILE:curl_test>)
endif()

target_link_libraries(curl_test PRIVATE
	${LIBS}
) 

