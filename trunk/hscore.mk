hscore_mods = 	hscore_buysell hscore_commands hscore_database \
		hscore_items hscore_money hscore_mysql hscore_rewards\
		hscore_spawner hscore_storeman hscore_freqman \
		hscore_moneystub hscore_itemstub hscore_teamnames

hscore_libs = 	$(MYSQL_LDFLAGS)	

$(eval $(call dl_template,hscore))
