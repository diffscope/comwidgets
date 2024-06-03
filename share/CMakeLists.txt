if(IDECORE_BUILD_TRANSLATIONS)
    qm_import(Translate)
    qm_find_qt(LinguistTools)

    add_custom_target(idecore_lupdate_task)
    add_custom_target(idecore_lrelease_task)

    # idecore
    get_property(_targets GLOBAL PROPERTY IDECORE_TRANSLATE_TARGETS)
    set(_qm_dir ${CMAKE_BUILD_SHARE_DIR}/${IDECORE_INSTALL_NAME}/translations)
    qm_add_translation(idecore_translations
        LOCALES ja_JP zh_CN zh_TW
        PREFIX idecore
        TARGETS ${_targets}
        TS_DIR translations
        QM_DIR ${_qm_dir}
        TS_DEPENDS idecore_lupdate_task
        QM_DEPENDS idecore_lrelease_task
        CREATE_ONCE
    )

    if(IDECORE_INSTALL)
        install(CODE "
            # execute_process(COMMAND \"${CMAKE_COMMAND}\" --build \"${CMAKE_BINARY_DIR}\" --target idecore_lupdate_task)
            execute_process(COMMAND \"${CMAKE_COMMAND}\" --build \"${CMAKE_BINARY_DIR}\" --target idecore_lrelease_task)
        ")
        install(DIRECTORY ${_qm_dir}/
            DESTINATION ${CMAKE_INSTALL_DATADIR}/${IDECORE_INSTALL_NAME}/translations
            FILES_MATCHING PATTERN "*.qm"
        )
    endif()
endif()