# gettext catalogs for the lgeneral domain

file(GLOB _po_files CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.po")
if(NOT _po_files)
	return()
endif()

set(_po_langs)
foreach(_po_file IN LISTS _po_files)
	get_filename_component(_lang "${_po_file}" NAME_WE)
	list(APPEND _po_langs "${_lang}")
endforeach()
list(REMOVE_DUPLICATES _po_langs)
list(SORT _po_langs)

gettext_process_po_files(lgeneral ALL
	PO_FILES ${_po_files})

set(_gmo_files)
set(_copy_cmds)
foreach(_lang IN LISTS _po_langs)
	set(_gmo_file "${CMAKE_CURRENT_BINARY_DIR}/${_lang}.gmo")
	list(APPEND _gmo_files ${_gmo_file})

	# Install tree (system prefix)
	install(FILES ${_gmo_file}
		DESTINATION ${CMAKE_INSTALL_LOCALEDIR}/${_lang}/LC_MESSAGES
		RENAME lgeneral.mo)

	# Build tree (developer run): copy into locale layout alongside build outputs
	set(_locale_dir "${CMAKE_BINARY_DIR}/po/locale/${_lang}/LC_MESSAGES")
	list(APPEND _copy_cmds
		COMMAND ${CMAKE_COMMAND} -E make_directory "${_locale_dir}"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_gmo_file}" "${_locale_dir}/lgeneral.mo")
endforeach()

if(_gmo_files)
	add_custom_target(lgeneral_locale ALL
		DEPENDS ${_gmo_files}
		${_copy_cmds})
endif()
