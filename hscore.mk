hscore_mods = hscore_buysell hscore_commands hscore_database \
	hscore_items hscore_money hscore_mysql hscore_rewards\
	hscore_spawner hscore_storeman hscore_freqman \
	hscore_teamnames hscore_wepevents hscore_buysell_points \
	hscore_moneycjf

hscore_libs = $(MYSQL_LDFLAGS) -lm

$(eval $(call dl_template,hscore))

$(call tobuild, hscore_mysql.o): CFLAGS += $(MYSQL_CFLAGS)

EXTRA_INCLUDE_DIRS := $(EXTRA_INCLUDE_DIRS) -Ihscore
