ALL_STUFF += hscore.$(SO)

hscore.$(SO): modules/hscore/trunk/hscore_buysell.o \
	modules/hscore/trunk/hscore_commands.o \
	modules/hscore/trunk/hscore_database.o \
	modules/hscore/trunk/hscore_items.o \
	modules/hscore/trunk/hscore_money.o \
	modules/hscore/trunk/hscore_mysql.o \
	modules/hscore/trunk/hscore_rewards.o \
	modules/hscore/trunk/hscore_spawner.o \
	modules/hscore/trunk/hscore_storeman.o \
	modules/hscore/trunk/hscore_freqman.o \
	modules/hscore/trunk/hscore_moneystub.o \
	modules/hscore/trunk/hscore_itemsstub.o
hscore.$(SO): LIBS=$(MYSQL_LDFLAGS)

modules/hscore/trunk/hscore_mysql.o: CFLAGS += $(MYSQL_CFLAGS)

