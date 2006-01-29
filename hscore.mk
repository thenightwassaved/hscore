hscore_mods = hscore_buysell hscore_commands hscore_database \
	hscore_items hscore_money hscore_mysql hscore_rewards\
	hscore_spawner hscore_storeman hscore_freqman \
	hscore_moneystub hscore_itemsstub hscore_teamnames \
	hscore_prizer

hscore_libs = $(MYSQL_LDFLAGS) -lm

$(eval $(call dl_template,hscore))

$(call tobuild, hscore_mysql.o): CFLAGS += $(MYSQL_CFLAGS)
