# Translation targets for lgeneral
# Builds .mo files from the legacy gettext catalogs in this directory.

if(NOT GETTEXT_FOUND OR NOT GETTEXT_MSGFMT_EXECUTABLE)
	message(WARNING "Skipping translations: gettext tools are unavailable")
	return()
endif()

foreach(_domain IN ITEMS lgeneral pg)
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_domain}")
		add_subdirectory(${_domain})
	endif()
endforeach()
