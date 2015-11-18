INST_LIB_PATH:=usr/lib
INST_BIN_PATH:=bin
PKG_NAME:=kappaio-zll
PKG_RELEASE:=1.0.0
# This specifies the directory where we're going to build the program.  
# The root build directory, $(BUILD_DIR), is by default the build_mipsel 
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)/src
include $(INCLUDE_DIR)/package.mk
define Package/$(PKG_NAME)/description
	core
endef

define Package/$(PKG_NAME)
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=$(PKG_NAME) -- Zigbee HomeAutomation Plugin for KR001
	DEPENDS:=+jansson +rsserial +libstdcpp
	Maintainer:=Yuming Liang
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/rsserial
	$(INSTALL_DIR) $(1)/usr/lib/rsserial/endpoints
	$(INSTALL_DIR) $(1)/usr/lib/rsserial/philips_hue
	$(CP) $(PKG_BUILD_DIR)/$(PKG_NAME)*so* $(1)/usr/lib/
	ln -s /usr/lib/$(PKG_NAME).so $(1)/usr/lib/rsserial/endpoints/$(PKG_NAME).so
	$(CP) ./files/hueclusters.json $(1)/usr/lib/rsserial/philips_hue
endef

define Build/InstallDev
endef

define Build/Compile
	$(call Build/Compile/Default,processor_family=$(_processor_family_))
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh
# check if we are on real system
$(info $(Profile))
if [ -z "$${IPKG_INSTROOT}" ]; then
	echo "Restarting application..."
	/etc/init.d/rsserial-watch restart
fi
exit 0
endef

define Package/$(PKG_NAME)/UploadAndInstall
ifeq ($(OPENWRT_BUILD),1)
compile: $(STAGING_DIR_ROOT)/stamp/.$(PKG_NAME)_installed
	$(SCP) $$(PACKAGE_DIR)/$$(PKG_NAME)_$$(VERSION)_$$(ARCH_PACKAGES).ipk $(1):/tmp
	$(SSH) $(1) opkg install --force-overwrite /tmp/$(PKG_NAME)_$$(VERSION)_$$(ARCH_PACKAGES).ipk
	$(SSH) $(1) rm /tmp/$$(PKG_NAME)_$$(VERSION)_$$(ARCH_PACKAGES).ipk
	$(SSH) $(1) rm /tmp/widget_registry.json
endif
ifeq ($(RASPBERRYPI_BUILD),1)
compile:$(STAMP_INSTALLED)
	@echo "---------------------------------------------------"
	@echo "**************** RASPBERRYPI_BUILD ****************"
	@echo "---------------------------------------------------"
	$(SCP) $$(PACKAGE_DIR)/$$(PACKAGE_BIN_DPKG) $(1):/tmp
	$(SSH) $(1) dpkg -i /tmp/$$(PACKAGE_BIN_DPKG)
endif
endef
UPLOAD_PATH:=$(or $(PKG_DST), $($(PKG_NAME)_DST))
$(if $(UPLOAD_PATH), $(eval $(call Package/$(PKG_NAME)/UploadAndInstall, $(UPLOAD_PATH))))

# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.
$(eval $(call BuildPackage,$(PKG_NAME)))
