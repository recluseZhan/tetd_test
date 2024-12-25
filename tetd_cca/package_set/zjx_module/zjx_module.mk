##################################################
###########
#
## zjx_module
#
###################################################
###########
ifeq ($(BR2_PACKAGE_ZJX_MODULE), y)
	ZJX_LINUX_DIR := $(TOPDIR)/../linux/lib/modules/6.7.0-rc4-g19ff5f60db62-dirty/build
	ZJX_MODULE_VERSION := 1.0.0
	ZJX_MODULE_SITE := $(TOPDIR)/../zjx_module-src
	ZJX_MODULE_SITE_METHOD := local
	ZJX_MODULE_INSTALL_TARGET := YES

define ZJX_MODULE_BUILD_CMDS
	$(MAKE) -C $(ZJX_LINUX_DIR) M=$(ZJX_MODULE_SITE) ARCH=arm64 CROSS_COMPILE=$(TOPDIR)/output/host/bin/aarch64-linux- modules
endef

# 内核模块的清理命令
define ZJX_MODULE_CLEAN_CMDS
	$(MAKE) -C $(ZJX_LINUX_DIR) M=$(ZJX_MODULE_SITE) ARCH=arm64 CROSS_COMPILE=$(TOPDIR)/output/host/bin/aarch64-linux- clean
endef

# 内核模块的安装命令
define ZJX_MODULE_INSTALL_TARGET_CMDS
	$(INSTALL) -d $(TARGET_DIR)/lib/modules/$(LINUX_VERSION)
	$(INSTALL) -m 644 $(ZJX_MODULE_SITE)/*.ko $(TARGET_DIR)/lib/modules/$(LINUX_VERSION)/
endef

# 内核模块的卸载命令
define ZJX_MODULE_UNINSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(ZJX_LINUX_DIR) M=$(ZJX_MODULE_SITE) unload
endef

# 执行通用的构建规则
$(eval $(generic-package))
endif
