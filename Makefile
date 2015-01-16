#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gearman-worker
PKG_VERSION:=1.0.0
PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

PKG_LICENSE:=GPLv2 GPLv2+
PKG_LICENSE_FILES:=

include $(INCLUDE_DIR)/package.mk

define Package/gearman-worker
  SECTION:=gearman-worker
  CATEGORY:=HomeHub
  TITLE:=Router gearman worker
  DEPENDS:=+libuuid
endef

define Package/gearman-worker/description
  router gearman worker 
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

#define Build/Compile
#	$(MAKE) -C $(PKG_BUILD_DIR) 
#endef

define Package/gearman-worker/install
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gearman-worker $(1)/
	$(CP) $(PKG_BUILD_DIR)/usr/* $(1)/usr/
endef

$(eval $(call BuildPackage,gearman-worker))
