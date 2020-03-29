
file(GLOB SOURCES 
	${CMAKE_CURRENT_SOURCE_DIR}/*.c*)

add_executable(curl_test
	${SOURCES}
)

find_package(CURL CONFIG REQUIRED)
target_link_libraries(curl_test PRIVATE CURL::libcurl)

find_package(OpenSSL REQUIRED)
target_link_libraries(curl_test PRIVATE OpenSSL::SSL OpenSSL::Crypto)


find_library(PCRE_LIBRARY pcreposix)
target_link_libraries(curl_test PRIVATE ${PCRE_LIBRARY})

find_package(pthread)
target_link_libraries(curl_test PRIVATE ${PTHREAD_LIBRARY})


if(MSVC)
	foreach(ROOTDIR ${CMAKE_PREFIX_PATH})
		file(GLOB RESOURCELIST ${ROOTDIR}/bin/*.dll)

		foreach(RESOURCE ${RESOURCELIST})
			add_custom_command(TARGET curl_test POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_if_different
				${RESOURCE}
				$<TARGET_FILE_DIR:curl_test>)
		endforeach()
	endforeach()

	set(LIBS
		mockhttpd
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

