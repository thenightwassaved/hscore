
# dist: public

ALL_STUFF += contrib/turf.$(SO)

contrib/turf.$(SO): contrib/turf_reward.o contrib/points_turf_reward.o \
	contrib/turf_stats.o

