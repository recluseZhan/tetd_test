##################################################
###########
#
## zjx_cca
#
###################################################
###########
ifeq ($(BR2_PACKAGE_ZJX_CCA), y)
	ZJX_CCA_VERSION:=1.0.0
	ZJX_CCA_SITE:=$(TOPDIR)/../zjx_cca-src
	ZJX_CCA_SITE_METHOD:=local
	ZJX_CCA_INSTALL_TARGET:=YES
define ZJX_CCA_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC=$(TARGET_CC) CXX=$(TARGET_CXX) -C $(@D)
endef
define ZJX_CCA_CLEAN_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) clean
endef
define ZJX_CCA_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) install
endef
define ZJX_CCA_UNINSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) uninstall
endef
$(eval $(generic-package))
endif
