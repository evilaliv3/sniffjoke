ADD_LIBRARY(shift_ack SHARED shift_ack.cc)
ADD_LIBRARY(fake_seq SHARED fake_seq.cc)
ADD_LIBRARY(fake_window SHARED fake_window.cc)

INSTALL(TARGETS shift_ack
        DESTINATION ${PATH_INSTALL_PLUGINS}
        PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)

INSTALL(TARGETS fake_seq
        DESTINATION ${PATH_INSTALL_PLUGINS}
        PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)

INSTALL(TARGETS fake_window
        DESTINATION ${PATH_INSTALL_PLUGINS}
        PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
